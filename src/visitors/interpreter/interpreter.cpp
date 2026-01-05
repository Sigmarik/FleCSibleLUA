#include "interpreter.h"

#include <iostream>
#include <memory>
#include <ranges>

#include <flecs.h>

#include "component_map/comp_map.h"
#include "meta/string.h"
#include "meta/variant_helper.h"

namespace flua::vst
{
using namespace flua;

std::map<flecs::entity_t, Interpreter::RegisteredSystemInfo> Interpreter::s_interpreterSystems{};

Interpreter::~Interpreter()
{
    for (ecs_entity_t entity : m_ownedSystems)
    {
        s_interpreterSystems.erase(entity);
    }
    for (auto& [key, queries] : m_nodeQueries)
    {
        for (auto& queryPair : queries)
        {
            ecs_query_fini(queryPair.query);
        }
    }
    Visitor::~Visitor();
}

void Interpreter::overrideGlobal(const std::string& name, const std::function<void(FluaState*)>& function)
{
    m_stack.front().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(data::Function(function));
}

void Interpreter::overrideGlobal(const std::string& name, double value)
{
    m_stack.front().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(value);
}

void Interpreter::overrideGlobal(const std::string& name, const std::string& value)
{
    m_stack.front().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(value);
}

void Interpreter::visit(ast::Program& node)
{
    // NOTE: Internal errors are thrown as exceptions and isolated here.
    //       Please do not take this as an allowance to use exceptions elsewhere
    //       in the project without a VERY good reason.
    try
    {
        for (ast::NodePtr& component : node.components)
        {
            Visitor::visit(component);
        }
        m_returnedValue.clear();
    }
    catch (const LuaRuntimeError& err)
    {
        m_fallen = true;
        printError(err);
    }
}

void Interpreter::visit(ast::Function& node)
{
    using VariableMap = std::unordered_map<std::string, mem_utils::CopyMovePtr<data::GenericValue>>;
    VariableMap bakedFrame;
    if (m_stack.size() > 1)
    {
        for (auto frameIt = m_stack.rbegin(); frameIt != m_stack.rend(); ++frameIt)
        {
            auto& frame = *frameIt;
            for (auto& [name, value] : frame.varNameMap)
            {
                if (bakedFrame.contains(name)) continue;
                bakedFrame[name] = value;
            }
            if (!frame.transparent) break;
        }
    }
    m_returnedValue = data::Function{
        data::LuaFunction{
            .body = &node,
            .frame = std::make_shared<VariableMap>(bakedFrame),
        }
    };
}

void Interpreter::visit(ast::System& node)
{
    if (node.entities.empty())
        throw LuaRuntimeError(node, "Attempt to create a system with no entities");

    auto entity = node.entities.front();
    ecs_query_desc_t query = makeEcsQueryDesc(entity, node);

    std::vector<NameQueryPair>& queries = m_nodeQueries[&node] =
        std::vector<NameQueryPair>(node.entities.size() - 1, {});
    for (unsigned entityId = 1; entityId < node.entities.size(); ++entityId)
    {
        queries[entityId - 1].entityName = node.entities[entityId].entityName.string;
        queries[entityId - 1].query = makeEcsQuery(node.entities[entityId], node);
    }

    ecs_system_desc_t sysDesc = {};
    static const ecs_id_t addons[] = {
        (ECS_PAIR | (static_cast<uint64_t>(EcsDependsOn) << 32) + static_cast<uint32_t>(
             EcsOnUpdate)),
        0
    };
    ecs_entity_desc_t systemEntityDesc{
        .name = "Move",
        .add = addons,
    };
    sysDesc.entity = ecs_entity_init(m_world->c_ptr(), &systemEntityDesc);
    sysDesc.callback = system_runner;
    sysDesc.query = query;

    ecs_entity_t sys = ecs_system_init(m_world->c_ptr(), &sysDesc);
    if (!ecs_is_valid(m_world->c_ptr(), sys) || !ecs_is_alive(m_world->c_ptr(), sys))
        throw LuaRuntimeError(node, "Failed to create system");

    s_interpreterSystems[sys] = {.interpreter = this, .luaSystem = &node};
    m_ownedSystems.emplace(sys);
}

void Interpreter::visit(ast::WhileLoop& node)
{
    while (true)
    {
        m_continuing = false;

        Visitor::visit(node.condition);
        if (!data::to_bool(m_returnedValue.spit())) break;

        visitTransparentBlock(node.body);

        if (m_continuing) m_continuing = false;
        if (m_returning || m_breaking) break;
    }

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::ForLoopNumeric& node)
{
    NamespaceHolder forNamespace(m_stack);

    Frame& frame = m_stack.back();
    frame.transparent = true;
    Visitor::visit(node.base);
    data::GenericValue& counter =
            *(frame.varNameMap[node.name.string] = mem_utils::CopyMovePtr<data::GenericValue>(m_returnedValue.spit()));
    Visitor::visit(node.limit);
    data::GenericValue limit = m_returnedValue.spit();

    Visitor::visit(node.step);
    data::GenericValue step = m_returnedValue.spit();

    double* counterPtr = std::get_if<double>(&counter);
    double* limitPtr = std::get_if<double>(&limit);
    double* stepPtr = std::get_if<double>(&step);

    assert(counterPtr && limitPtr && stepPtr && "Not all arguments of the for loop returned numeric values");

    while ((*limitPtr - *counterPtr) * *stepPtr >= 0)
    {
        visitTransparentBlock(node.body);

        if (m_continuing) m_continuing = false;
        if (m_returning || m_breaking) break;

        *counterPtr += *stepPtr;
    }

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::ForLoopGeneric& node)
{
    Visitor::visit(node.iterator);
    data::GenericValue functor = m_returnedValue.spit();

    if (!std::holds_alternative<data::Function>(functor))
        throw LuaRuntimeError(node, "Attempt to call a non-functional value");

    while (true)
    {
        NamespaceHolder forNamespace(m_stack);
        std::deque<ast::NodePtr> args;
        executeFunction(node, std::get<data::Function>(functor), args);
        if (m_returnedValue.sequence.empty() || std::holds_alternative<data::Nil>(
                m_returnedValue.sequence.front().value))
            break;
        for (unsigned id = 0; id < node.names.size(); ++id)
        {
            const std::string& name = node.names[id].string;
            if (id < m_returnedValue.sequence.size())
                m_stack.back().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(
                    std::move(m_returnedValue.sequence[id].value));
            else
                m_stack.back().varNameMap[name] = mem_utils::CopyMovePtr<data::GenericValue>(data::Nil());
        }

        visitTransparentBlock(node.body);

        if (m_continuing) m_continuing = false;
        if (m_returning || m_breaking) break;
    }

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::Query& node)
{
    NamespaceHolder queryNamespace(m_stack);

    if (m_nodeQueries.contains(&node))
    {
        runBodyWithinQueries(m_nodeQueries.at(&node), node.body, 0);
        return;
    }
    if (node.filters.empty())
        throw LuaRuntimeError(node, "Attempt to create a query with no entities");

    std::vector<NameQueryPair>& queries = m_nodeQueries[&node] =
        std::vector<NameQueryPair>(node.filters.size(), {});
    for (unsigned entityId = 0; entityId < node.filters.size(); ++entityId)
    {
        queries[entityId].entityName = node.filters[entityId].entityName.string;
        queries[entityId].query = makeEcsQuery(node.filters[entityId], node);
    }

    runBodyWithinQueries(queries, node.body, 0);
}

void Interpreter::visit(ast::RepeatUntil& node)
{
    while (true)
    {
        m_continuing = false;

        visitTransparentBlock(node.body);

        if (m_continuing) m_continuing = false;
        if (m_returning || m_breaking) break;

        Visitor::visit(node.condition);
        if (!data::to_bool(m_returnedValue.spit())) break;
    }

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::Branch& node)
{
    bool executeElseBranch = true;
    for (ast::Branch::Case& branchCase : node.cases)
    {
        Visitor::visit(branchCase.condition);
        if (data::to_bool(m_returnedValue.spit()))
        {
            executeElseBranch = false;
            visitTransparentBlock(branchCase.block);
            if (m_returning || m_breaking || m_continuing) return;
        }

        if (!m_returning) m_returnedValue.clear();
    }

    if (executeElseBranch)
    {
        visitTransparentBlock(node.ifFalse);
    }

    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::FunctionCall& node)
{
    Visitor::visit(node.function);

    data::GenericValue maybeFunction = m_returnedValue.spit();

    if (!std::holds_alternative<data::Function>(maybeFunction))
        throw LuaRuntimeError(node, "Cannot execute a non-functional object");

    data::Function func = std::get<data::Function>(maybeFunction);

    executeFunction(node, func, node.args);
}

void Interpreter::visit(ast::UnaryOperator& node)
{
    Visitor::visit(node.node);
    data::GenericValue operand = m_returnedValue.spit();
    switch (node.type)
    {
        case ast::UnaryOperator::Type::Not:
        {
            m_returnedValue = !data::to_bool(operand);
        }
        break;
        case ast::UnaryOperator::Type::Negate:
        {
            if (std::holds_alternative<double>(operand))
            {
                m_returnedValue = -std::get<double>(operand);
            }
            else if (std::holds_alternative<bool>(operand))
            {
                m_returnedValue = !data::to_bool(operand);
            }
            else
            {
                throw LuaRuntimeError(node, "Cannot negate a non-numerical and non-logical type");
            }
        }
        break;
        case ast::UnaryOperator::Type::Length:
        {
            m_returnedValue = static_cast<double>(data::to_string(operand).length());
        }
        break;
    }
}

void Interpreter::visit(ast::BinaryOperator& node)
{
    Visitor::visit(node.left);
    data::GenericValue leftOperand = m_returnedValue.spit();

    if (node.type == ast::BinaryOperator::Type::And && !data::to_bool(leftOperand))
    {
        m_returnedValue = false;
        return;
    }
    if (node.type == ast::BinaryOperator::Type::Or && data::to_bool(leftOperand))
    {
        m_returnedValue = true;
        return;
    }

    Visitor::visit(node.right);
    data::GenericValue rightOperand = m_returnedValue.spit();

    if (node.type == ast::BinaryOperator::Type::Concatenate)
    {
        m_returnedValue = data::to_string(leftOperand) + data::to_string(rightOperand);
        return;
    }

    if (node.type == ast::BinaryOperator::Type::CmpEq)
    {
        m_returnedValue = data::to_string(leftOperand) == data::to_string(rightOperand);
        return;
    }

    if (node.type == ast::BinaryOperator::Type::CmpNeq)
    {
        m_returnedValue = data::to_string(leftOperand) != data::to_string(rightOperand);
        return;
    }

    switch (node.type)
    {
        case ast::BinaryOperator::Type::And:
            m_returnedValue = data::to_bool(leftOperand) && data::to_bool(rightOperand);
            return;
        case ast::BinaryOperator::Type::Or:
            m_returnedValue = data::to_bool(leftOperand) || data::to_bool(rightOperand);
            return;
        case ast::BinaryOperator::Type::Xor:
            m_returnedValue = data::to_bool(leftOperand) != data::to_bool(rightOperand);
            return;
        default: ;
    }

    if (!std::holds_alternative<double>(leftOperand) || !std::holds_alternative<double>(rightOperand))
    {
        throw LuaRuntimeError(node, "Attempt to perform arithmetics on a non-numeric value");
    }

    double alpha = std::get<double>(leftOperand);
    double beta = std::get<double>(rightOperand);

    switch (node.type)
    {
        case ast::BinaryOperator::Type::Add:
            m_returnedValue = alpha + beta;
            break;
        case ast::BinaryOperator::Type::Subtract:
            m_returnedValue = alpha - beta;
            break;
        case ast::BinaryOperator::Type::Multiply:
            m_returnedValue = alpha * beta;
            break;
        case ast::BinaryOperator::Type::Divide:
            m_returnedValue = alpha / beta;
            break;
        case ast::BinaryOperator::Type::Mod:
            m_returnedValue = alpha - std::floor(alpha / beta) * beta;
            break;
        case ast::BinaryOperator::Type::Pow:
            m_returnedValue = std::pow(alpha, beta);
            break;
        case ast::BinaryOperator::Type::CmpGe:
            m_returnedValue = alpha >= beta;
            break;
        case ast::BinaryOperator::Type::CmpGt:
            m_returnedValue = alpha > beta;
            break;
        case ast::BinaryOperator::Type::CmpLe:
            m_returnedValue = alpha <= beta;
            break;
        case ast::BinaryOperator::Type::CmpLt:
            m_returnedValue = alpha < beta;
            break;
        default: ;
    }
}

void Interpreter::visit(ast::FieldRequest& node)
{
    assert(false && "Not implemented");
}

void Interpreter::visit(ast::IndexRequest& node)
{
    Visitor::visit(node.body);
    data::GenericValue dict = m_returnedValue.spit();
    Visitor::visit(node.index);
    std::string index = data::to_string(m_returnedValue.spit());

    if (std::holds_alternative<data::Entity>(dict))
    {
        indexEntity(node, std::get<data::Entity>(dict), index);
        return;
    }

    if (std::holds_alternative<data::EntityComponent>(dict))
    {
        indexEntityComponent(node, std::get<data::EntityComponent>(dict), index);
        return;
    }

    if (!std::holds_alternative<data::Table>(dict))
    {
        throw LuaRuntimeError(node, "Attempt to index a non-dictionary value");
    }

    auto& dct = std::get<data::Table>(dict);
    if (!dct->contains(index))
    {
        dct->emplace(index, std::make_unique<data::GenericValue>(data::Nil()));
    }
    m_returnedValue.clear();
    m_returnedValue.addReferenced(*dct->at(index));
}

void Interpreter::visit(ast::Constant& node)
{
    m_returnedValue = node.value;
}

void Interpreter::visit(ast::MakeTable& node)
{
    data::Table table;
    size_t elementIndex = 1;
    for (auto& element : node.values)
    {
        data::GenericValue index = static_cast<double>(elementIndex);
        if (element.index.has_value())
        {
            Visitor::visit(*element.index.value());
            index = m_returnedValue.spit();
        }
        else
        {
            ++elementIndex;
        }

        Visitor::visit(*element.value);
        table->emplace(data::to_string(index), new data::GenericValue(m_returnedValue.spit()));
    }

    m_returnedValue = std::move(table);
}

void Interpreter::visit(ast::Variable& node)
{
    bool localAssignment = m_inLocalAssignment;
    m_inLocalAssignment = false;

    data::GenericValue* foundValue = nullptr;
    for (auto& frameIt : std::ranges::reverse_view(m_stack))
    {
        auto found = frameIt.varNameMap.find(node.name.string);
        if (found != frameIt.varNameMap.end())
        {
            foundValue = found->second.get();
            break;
        }
        if (!frameIt.transparent)
        {
            break;
        }
    }

    if (foundValue == nullptr && !localAssignment)
    {
        auto foundGlobal = m_stack.front().varNameMap.find(node.name.string);
        if (foundGlobal != m_stack.front().varNameMap.end()) foundValue = foundGlobal->second.get();
    }

    if (foundValue == nullptr)
    {
        std::unordered_map<std::string, mem_utils::CopyMovePtr<data::GenericValue> >& frameToAddTo =
                localAssignment ? m_stack.back().varNameMap : m_stack.front().varNameMap;
        foundValue = (frameToAddTo[node.name.string] =
                      mem_utils::CopyMovePtr<data::GenericValue>(data::Nil())).get();
    }

    m_returnedValue.clear();
    m_returnedValue.addReferenced(*foundValue);
}

void Interpreter::visit(ast::Assignment& node)
{
    std::vector<data::MaybeFixedValuePtr> subjects;
    std::vector<data::GenericValue> values;
    subjects.reserve(node.subjects.size());
    for (ast::NodePtr& subject : node.subjects)
    {
        Visitor::visit(subject);
        if (m_returnedValue.sequence.empty() || m_returnedValue.sequence.front().reference ==
            data::MaybeFixedValuePtr(nullptr))
        {
            throw LuaRuntimeError(node, "Attempt to assign a value to a non-assignable variable");
        }
        subjects.emplace_back(m_returnedValue.sequence.front().reference);
    }

    for (ast::NodePtr& value : node.data)
    {
        Visitor::visit(value);

        for (data::ValueSequence::ValueBackrefPair& pair : m_returnedValue.sequence)
        {
            values.emplace_back(std::move(pair.value));
        }
    }

    for (size_t idx = 0; idx < subjects.size(); ++idx)
    {
        data::MaybeFixedValuePtr& subject = subjects[idx];
        if (auto genericSubject = std::get_if<data::GenericValue*>(&subject))
        {
            **genericSubject = idx < values.size() ? std::move(values[idx]) : data::Nil();
        }
        else
        {
            if (idx >= values.size())
                throw LuaRuntimeError(node, "Not enough arguments to satisfy entity component field assignment");
            data::GenericValue& value = values[idx];
            performFixedTypeAssignment(node, std::get<cmp_info::GenericComponentPtr>(subject), value);
        }
    }

    m_returnedValue.clear();
}

void Interpreter::visit(ast::LocalAssignment& node)
{
    std::vector<data::GenericValue*> subjects;
    std::vector<data::GenericValue> values;
    subjects.reserve(node.names.size());
    for (const ids::ResolvableName& subject : node.names)
    {
        Frame& localFrame = m_stack.back();
        auto emplacementResult = localFrame.varNameMap.try_emplace(subject.string);
        if (!emplacementResult.second)
        {
            throw LuaRuntimeError(
                node, "Attempt to create a local variable \"" + subject.string + "\" that already exists");
        }
        subjects.emplace_back(emplacementResult.first->second.get());
    }

    for (ast::NodePtr& value : node.values)
    {
        Visitor::visit(value);

        for (data::ValueSequence::ValueBackrefPair& pair : m_returnedValue.sequence)
        {
            values.emplace_back(std::move(pair.value));
        }
    }

    for (size_t idx = 0; idx < subjects.size(); ++idx)
    {
        auto& subject = subjects[idx];
        *subject = idx < values.size() ? std::move(values[idx]) : data::Nil();
    }
}

void Interpreter::visit(ast::Return& node)
{
    std::vector<data::ValueSequence::ValueBackrefPair> values;
    for (ast::NodePtr& value : node.values)
    {
        Visitor::visit(value);

        for (data::ValueSequence::ValueBackrefPair& pair : m_returnedValue.sequence)
        {
            values.emplace_back(std::move(pair));
        }

        m_returnedValue.clear();
    }

    m_returnedValue.sequence = std::move(values);
    m_returning = true;
}

void Interpreter::visit(ast::Break& node)
{
    m_breaking = true;
    m_returnedValue.clear();
}

void Interpreter::visit(ast::Continue& node)
{
    m_continuing = true;
    m_returnedValue.clear();
}

void Interpreter::performFixedTypeAssignment(ast::Assignment& node, cmp_info::GenericComponentPtr pointer,
                                             data::GenericValue& value)
{
    auto reinterpretations = meta::Overloads{
        [&](int* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a non-numeric value to numeric component field");
            *ptr = static_cast<int>(std::get<double>(value));
        },
        [&](unsigned* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a non-numeric value to numeric component field");
            *ptr = static_cast<unsigned>(std::get<double>(value));
        },
        [&](float* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a non-numeric value to numeric component field");
            *ptr = static_cast<float>(std::get<double>(value));
        },
        [&](double* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a non-numeric value to numeric component field");
            *ptr = std::get<double>(value);
        },
        [&](auto* ptr)
        {
            using UnderlyingType = std::remove_reference_t<decltype(*ptr)>;
            if (!std::holds_alternative<UnderlyingType>(value))
                throw LuaRuntimeError(node, "Cannot assign a value of another type to a component field");
            *ptr = std::get<UnderlyingType>(value);
        },
    };
    std::visit(reinterpretations, pointer);
}

void Interpreter::indexEntity(ast::IndexRequest& node, data::Entity& entity, const std::string& index)
{
    if (!cmp_info::ENTITY_COMPONENT_CHECKERS.contains(index))
        throw LuaRuntimeError(node, "Component " + index + " is not recognized by the system");
    m_returnedValue = data::EntityComponent(entity, index);
}

void Interpreter::indexEntityComponent(ast::IndexRequest& node, data::EntityComponent& component,
                                       const std::string& index)
{
    auto foundMapper = cmp_info::ENTITY_MEMBER_MAP.find(component.name + " " + index);
    if (foundMapper == cmp_info::ENTITY_MEMBER_MAP.end())
        throw LuaRuntimeError(node, "Component field " + index + " is not recognized by the system");

    if (!cmp_info::ENTITY_COMPONENT_CHECKERS.at(component.name)(component.entity))
    {
        m_returnedValue = data::Nil();
        return;
    }

    auto genericReference = foundMapper->second(component.entity);
    data::GenericValue interpretedValue = data::Nil();
    auto interpretation = meta::Overloads{
        [&](std::string* ptr) { interpretedValue = data::GenericValue(*ptr); },
        [&](bool* ptr) { interpretedValue = data::GenericValue(*ptr); },
        [&](flecs::entity* ptr) { interpretedValue = data::GenericValue(*ptr); },
        [&](auto ptr) { interpretedValue = data::GenericValue(static_cast<double>(*ptr)); }
    };
    std::visit(interpretation, genericReference);

    m_returnedValue.clear();
    m_returnedValue.sequence.emplace_back(data::ValueSequence::ValueBackrefPair{
        .value = interpretedValue,
        .reference = std::move(genericReference),
    });
}

void Interpreter::visitTransparentBlock(std::deque<ast::NodePtr>& nodes)
{
    NamespaceHolder blockNamespace(m_stack, true);

    for (ast::NodePtr& node : nodes)
    {
        Visitor::visit(node);
        if (m_breaking || m_continuing || m_returning) break;
        m_returnedValue.clear();
    }
}

void Interpreter::executeFunction(const ast::INode& node, data::Function& func, std::deque<ast::NodePtr>& args)
{
    std::vector<data::GenericValue> arguments;
    for (ast::NodePtr& argument : args)
    {
        Visitor::visit(argument);
        arguments.emplace_back(m_returnedValue.spit());
    }

    if (data::LuaFunction* luaFunction = std::get_if<data::LuaFunction>(&func))
    {
        runLuaFunction(*luaFunction, arguments);
    }
    else
    {
        auto& libFunction = std::get<data::LibraryFunction>(func);
        m_externalFunctionInputs = std::move(arguments);
        FluaState state = generatePublicState();

        try
        {
            libFunction(&state);
        }
        catch (Error& userError)
        {
            throw LuaRuntimeError(node, userError.message);
        }

        m_externalFunctionInputs.clear();
        m_returnedValue.clear();
        for (auto& returned : m_externalFunctionOutputs)
        {
            m_returnedValue.sequence.emplace_back(std::move(returned));
        }
        m_externalFunctionOutputs.clear();
    }
}

void Interpreter::executeFunction(ast::Function& function)
{
    visitTransparentBlock(function.body);
    if (!m_returning) m_returnedValue.clear();
    m_returning = m_breaking = m_continuing = false;
}

void Interpreter::runLuaFunction(data::LuaFunction& luaFunction, std::vector<data::GenericValue>& args)
{
    NamespaceHolder functionEnvNamespace(m_stack, false);

    m_stack.back().varNameMap = std::move(*luaFunction.frame);

    {
        NamespaceHolder functionNamespace(m_stack);

        for (size_t idx = 0; idx < luaFunction.body->parameters.size() && idx < args.size(); ++idx)
        {
            ids::ResolvableName& name = luaFunction.body->parameters[idx];
            m_stack.back().varNameMap[name.string] = mem_utils::CopyMovePtr<data::GenericValue>(std::move(args[idx]));
        }

        executeFunction(*luaFunction.body);
    }
    *luaFunction.frame = std::move(m_stack.back().varNameMap);
}

FluaState Interpreter::generatePublicState()
{
    return {this, m_world};
}

void Interpreter::printError(const LuaRuntimeError& err)
{
    m_errStream << "FLua runtime ERROR at line " << err.where.line << " column " << err.where.column << ":\n\t" <<
            err.what << std::endl;
}

ecs_query_desc_t Interpreter::makeEcsQueryDesc(const ast::EcsEntityFilter& filter, ast::INode& node)
{
    ecs_query_desc_t desc{};

    if (filter.components.size() > FLECS_TERM_COUNT_MAX)
        throw LuaRuntimeError(node, "Entity filter " + filter.entityName.string +
                                    " cannot have more than " + std::to_string(FLECS_TERM_COUNT_MAX) + " components");

    unsigned termIdx = 0;
    for (const std::string& component : filter.components)
    {
        auto found = m_componentIds.find(component);
        if (found == m_componentIds.end())
            throw LuaRuntimeError(node, "Component " + component + " is not recognized by the system");
        desc.terms[termIdx] = {found->second};
    }
    return desc;
}

ecs_query_t* Interpreter::makeEcsQuery(const ast::EcsEntityFilter& filter, ast::INode& node)
{
    ecs_query_desc_t desc = makeEcsQueryDesc(filter, node);
    return ecs_query_init(m_world->c_ptr(), &desc);
}

struct GuardedEcsIterator
{
    explicit GuardedEcsIterator(const ecs_iter_t& iterator) : m_iterator(iterator) {}
    ~GuardedEcsIterator() { if (!m_finished) ecs_iter_fini(&m_iterator); }

    void finish() { m_finished = true; }
    ecs_iter_t* operator->() { return &m_iterator; }
    const ecs_iter_t* operator->() const { return &m_iterator; }
    ecs_iter_t& operator*() { return m_iterator; }
    const ecs_iter_t& operator*() const { return m_iterator; }

private:
    ecs_iter_t m_iterator;
    bool m_finished = false;
};

void Interpreter::runBodyWithinQueries(std::vector<NameQueryPair>& queries, std::deque<ast::NodePtr>& body,
                                       unsigned iterId)
{
    if (iterId >= queries.size())
    {
        visitTransparentBlock(body);
        return;
    }

    auto& queryPair = queries[iterId];
    ecs_query_t* query = queryPair.query;
    GuardedEcsIterator iter(ecs_query_iter(m_world->c_ptr(), query));
    while (ecs_query_next(&*iter))
    {
        const std::string& entityName = queryPair.entityName;
        for (long long iterEntityIdx = 0; iterEntityIdx < iter->count; ++iterEntityIdx)
        {
            ecs_entity_t ecsEntity = iter->entities[iterEntityIdx];
            m_stack.back().varNameMap[entityName] = mem_utils::CopyMovePtr<data::GenericValue>(
                flecs::entity(iter->world, ecsEntity));
            runBodyWithinQueries(queries, body, iterId + 1);
        }
    }
    iter.finish();
}

void Interpreter::prepareAndRunSystem(ast::System& node, ecs_iter_t* systemIt)
{
    NamespaceHolder systemNamespace(m_stack, false);

    const std::string& entityName = node.entities.front().entityName.string;
    for (long long iterEntityIdx = 0; iterEntityIdx < systemIt->count; ++iterEntityIdx)
    {
        ecs_entity_t ecsEntity = systemIt->entities[iterEntityIdx];
        m_stack.back().varNameMap[entityName] = mem_utils::CopyMovePtr<data::GenericValue>(
            flecs::entity(systemIt->world, ecsEntity));
        runBodyWithinQueries(m_nodeQueries[&node], node.body, 0);
    }
}

void Interpreter::system_runner(ecs_iter_t* it)
{
    auto found = s_interpreterSystems.find(it->system);
    if (found == s_interpreterSystems.end())
    {
        ecs_delete(it->world, it->system);
        return;
    }
    RegisteredSystemInfo& registration = found->second;
    ast::System& node = *registration.luaSystem;
    Interpreter& interpreter = *registration.interpreter;

    try
    {
        interpreter.prepareAndRunSystem(node, it);
    }
    catch (LuaRuntimeError& err)
    {
        interpreter.m_fallen = true;
        interpreter.printError(err);
        ecs_delete(it->world, it->system);
    }
}
}
