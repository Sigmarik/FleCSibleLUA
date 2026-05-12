#include "interpreter.h"

#include <complex>
#include <iostream>
#include <memory>
#include <ranges>

#include <flecs.h>

#include "component_map/comp_map.h"
#include "ecs/guarded_iterator.h"
#include "meta/string.h"
#include "meta/variant_helper.h"
#include "meta/remap.h"

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
            if (!queryPair.query) break;
            ecs_query_fini(queryPair.query);
        }
    }
    Visitor::~Visitor();
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
    std::vector<mem_utils::CopyMovePtr<data::GenericValue>> capturedValues;
    for (const data::Address& address : node.valuesToCapture)
    {
        capturedValues.push_back(resolveAddress(address));
    }
    m_returnedValue = data::Function{
        data::LuaFunction{
            .body = &node,
            .capturedValues = capturedValues,
        }
    };
}

void Interpreter::visit(ast::System& node)
{
    if (node.entities.empty())
        throw LuaRuntimeError(node, "Attempt to create a system with no entities");

    const ast::EcsEntityFilter& entity = node.entities.front();
    ecs_query_desc_t query = makeEcsQueryDesc(entity, node);

    std::vector<AddressQueryPair>& queries = m_nodeQueries[&node] =
                                          std::vector<AddressQueryPair>(node.entities.size() - 1, {});
    for (unsigned entityId = 1; entityId < node.entities.size(); ++entityId)
    {
        queries[entityId - 1].address = node.iteratorAddresses[entityId];
        queries[entityId - 1].query = makeEcsQuery(node.entities[entityId], node);
    }

    ecs_system_desc_t sysDesc = {};
    static const ecs_id_t addons[] = {
        (ECS_PAIR | (static_cast<uint64_t>(EcsDependsOn) << 32) + static_cast<uint32_t>(
             EcsOnUpdate)),
        0
    };
    ecs_entity_desc_t systemEntityDesc{
        .add = addons,
    };
    sysDesc.entity = ecs_entity_init(m_world->c_ptr(), &systemEntityDesc);
    sysDesc.callback = system_runner;
    sysDesc.query = query;

    ecs_entity_t sys = ecs_system_init(m_world->c_ptr(), &sysDesc);
    if (!ecs_is_valid(m_world->c_ptr(), sys) || !ecs_is_alive(m_world->c_ptr(), sys))
        throw LuaRuntimeError(node, "Failed to create the system");

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

        m_continuing = false;
        if (m_returning || m_breaking) break;
    }

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::ForLoopNumeric& node)
{
    Visitor::visit(node.base);
    data::GenericValue& counter =
            *(resolveAddress(node.iteratorAddress) = mem_utils::CopyMovePtr<data::GenericValue>(m_returnedValue.spit()));
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

        m_continuing = false;
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
        throw LuaRuntimeError(node, "Attempt to call a non-functional value of type " + data::get_type_name(functor));

    while (true)
    {
        std::deque<ast::NodePtr> args;
        executeFunction(node, std::get<data::Function>(functor), args);
        if (m_returnedValue.sequence.empty() || std::holds_alternative<data::Nil>(
                m_returnedValue.sequence.front().value))
            break;
        for (unsigned id = 0; id < node.names.size(); ++id)
        {
            const mem_utils::PointerMappedString& name = node.names[id];
            data::Address address = node.iteratorAddresses[id];
            if (id < m_returnedValue.sequence.size())
                resolveAddress(address) = mem_utils::CopyMovePtr<data::GenericValue>(
                    std::move(m_returnedValue.sequence[id].value));
            else
                resolveAddress(address) = mem_utils::CopyMovePtr<data::GenericValue>(data::Nil());
        }

        visitTransparentBlock(node.body);

        m_continuing = false;
        if (m_returning || m_breaking) break;
    }

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::Query& node)
{
    if (m_nodeQueries.contains(&node))
    {
        runBodyWithinQueries(m_nodeQueries.at(&node), node.body, 0);
        return;
    }
    if (node.filters.empty())
        throw LuaRuntimeError(node, "Attempt to create a query with no entities");

    std::vector<AddressQueryPair>& queries = m_nodeQueries[&node] =
                                          std::vector<AddressQueryPair>(node.filters.size(), {});
    for (unsigned entityId = 0; entityId < node.filters.size(); ++entityId)
    {
        queries[entityId].address = node.iteratorAddresses[entityId];
        queries[entityId].query = makeEcsQuery(node.filters[entityId], node);
    }

    runBodyWithinQueries(queries, node.body, 0);

    m_breaking = false;
    if (!m_returning) m_returnedValue.clear();
}

void Interpreter::visit(ast::RepeatUntil& node)
{
    while (true)
    {
        m_continuing = false;

        visitTransparentBlock(node.body);

        m_continuing = false;
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

    if (m_returnedValue.sequence.empty())
    {
        throw LuaRuntimeError(
            node, "Cannot execute a non-functional object of type " + data::get_type_name(data::Nil()));
    }

    data::GenericValue maybeFunctionCopy;
    data::GenericValue* maybeFunction = &maybeFunctionCopy;
    if (std::holds_alternative<data::GenericValue*>(m_returnedValue.sequence.front().reference))
    {
        maybeFunction = std::get<data::GenericValue*>(m_returnedValue.sequence.front().reference);
    }
    else
    {
        maybeFunctionCopy = m_returnedValue.spit();
    }
    m_returnedValue.clear();

    if (!std::holds_alternative<data::Function>(*maybeFunction))
        throw LuaRuntimeError(
            node, "Cannot execute a non-functional object of type " + data::get_type_name(*maybeFunction));

    auto& func = std::get<data::Function>(*maybeFunction);

    executeFunction(node, func, node.args);
}

void Interpreter::visit(ast::UnaryOperator& node)
{
    Visitor::visit(node.node);
    data::GenericValue operand = m_returnedValue.spit();
    std::optional<data::GenericValue> maybeResult = data::perform_unary_operation(node.type, operand);
    if (!maybeResult)
        throw LuaRuntimeError(node, "Cannot apply operator " + data::UNARY_OP_TYPE_NAMES.at(node.type) +
                                    " to a value of type " + data::get_type_name(operand));
    m_returnedValue = *maybeResult;
}

void Interpreter::visit(ast::BinaryOperator& node)
{
    Visitor::visit(node.left);
    data::GenericValue leftOperand = m_returnedValue.spit();

    if (node.type == data::BinaryOpType::And && !data::to_bool(leftOperand))
    {
        m_returnedValue = false;
        return;
    }
    if (node.type == data::BinaryOpType::Or && data::to_bool(leftOperand))
    {
        m_returnedValue = true;
        return;
    }

    Visitor::visit(node.right);
    data::GenericValue rightOperand = m_returnedValue.spit();

    std::optional<data::GenericValue> maybeResult =
            data::perform_binary_operation(node.type, leftOperand, rightOperand);
    if (!maybeResult)
        throw LuaRuntimeError(node, "Cannot apply operator " + data::BINARY_OP_TYPE_NAMES.at(node.type) +
                                    " to values of types " + data::get_type_name(leftOperand) + " and " +
                                    data::get_type_name(rightOperand));
    m_returnedValue = *maybeResult;
}

void Interpreter::visit(ast::FieldRequest& node)
{
    assert(false && "Not implemented");
}

template <class VecN>
static std::optional<double> extract_component(const VecN&, char) { return {}; }

template <>
std::optional<double> extract_component(const Vec2& vec, char ch)
{
    if (ch == 'x' || ch == 'X') return vec.x;
    if (ch == 'y' || ch == 'Y') return vec.y;
    return {};
}

template <>
std::optional<double> extract_component(const Vec3& vec, char ch)
{
    if (ch == 'x' || ch == 'X') return vec.x;
    if (ch == 'y' || ch == 'Y') return vec.y;
    if (ch == 'z' || ch == 'Z') return vec.z;
    return {};
}

template <>
std::optional<double> extract_component(const Vec4& vec, char ch)
{
    if (ch == 'x' || ch == 'X') return vec.x;
    if (ch == 'y' || ch == 'Y') return vec.y;
    if (ch == 'z' || ch == 'Z') return vec.z;
    if (ch == 'w' || ch == 'W') return vec.w;
    return {};
}

template <class VecN>
std::vector<double> Interpreter::extract_vector_components(const VecN& vec, const std::string& index, ast::INode& node)
{
    std::vector<double> components;
    for (char ch : index)
    {
        std::optional<double> comp = extract_component(vec, ch);
        if (comp) components.push_back(*comp);
        else throw LuaRuntimeError(node,
                "Unexpected index " + std::string(1, ch) + " for type " + data::get_type_name(vec));
    }
    return components;
}

void Interpreter::visit(ast::IndexRequest& node)
{
    Visitor::visit(node.body);
    data::GenericValue subject = m_returnedValue.spit();
    Visitor::visit(node.index);
    mem_utils::PointerMappedString index(data::to_string(m_returnedValue.spit()));

    if (std::holds_alternative<Vec2>(subject) ||
        std::holds_alternative<Vec3>(subject) ||
        std::holds_alternative<Vec4>(subject))
    {
        std::vector<double> components;
        std::visit([&](const auto& vec)
        {
            components = extract_vector_components(vec, *index, node);
        }, subject);

        if (components.size() == 1)
            m_returnedValue = components.front();
        else if (components.size() == 2)
            m_returnedValue = Vec2{components[0], components[1]};
        else if (components.size() == 3)
            m_returnedValue = Vec3{components[0], components[1], components[2]};
        else if (components.size() == 4)
            m_returnedValue = Vec4{components[0], components[1], components[2], components[3]};
        else
            throw LuaRuntimeError(node,
                "Cannot index-create a vector with " + std::to_string(components.size()) + " components");
        return;
    }

    if (std::holds_alternative<data::Entity>(subject))
    {
        indexEntity(node, std::get<data::Entity>(subject), index);
        return;
    }

    if (std::holds_alternative<data::EntityComponent>(subject))
    {
        indexEntityComponent(node, std::get<data::EntityComponent>(subject), index);
        return;
    }

    if (!std::holds_alternative<data::Table>(subject))
    {
        throw LuaRuntimeError(node, "Attempt to index value of type " + data::get_type_name(subject));
    }

    auto& dct = std::get<data::Table>(subject);
    if (!dct->contains(index))
    {
        dct->emplace(index, mem_utils::CopyMovePtr<data::GenericValue>(data::Nil()));
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
        table->emplace(data::to_string(index), data::GenericValue(m_returnedValue.spit()));
    }

    m_returnedValue = std::move(table);
}

void Interpreter::visit(ast::Variable& node)
{
    if (node.resolvedAddress)
    {
        m_returnedValue.clear();
        m_returnedValue.addReferenced(*resolveAddress(*node.resolvedAddress));
        return;
    }

    bool localAssignment = m_inLocalAssignment;
    m_inLocalAssignment = false;

    data::GenericValue* foundValue = nullptr;

    if (!localAssignment)
    {
        auto foundGlobal = m_globalVariables.find(node.name);
        if (foundGlobal != m_globalVariables.end()) foundValue = foundGlobal->second.get();
    }

    if (foundValue == nullptr)
    {
        foundValue = (m_globalVariables[node.name] =
                      mem_utils::CopyMovePtr<data::GenericValue>(data::Nil())).get();
    }

    m_returnedValue.clear();
    m_returnedValue.addReferenced(*foundValue);
}

void Interpreter::visit(ast::Assignment& node)
{
    std::vector<data::GenericValue> subjectValues;
    std::vector<data::MaybeFixedValuePtr> subjects;
    std::vector<data::GenericValue> values;
    subjects.reserve(node.subjects.size());
    for (ast::NodePtr& subject : node.subjects)
    {
        Visitor::visit(subject);
        if (m_returnedValue.sequence.empty() ||
            (!std::holds_alternative<data::EntityComponent>(m_returnedValue.sequence.front().value) &&
                m_returnedValue.sequence.front().reference == data::MaybeFixedValuePtr(nullptr)))
        {
            throw LuaRuntimeError(node, "Attempt to assign a value to a non-assignable variable");
        }
        subjects.emplace_back(m_returnedValue.sequence.front().reference);
        subjectValues.emplace_back(m_returnedValue.sequence.front().value);
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
        data::GenericValue value = idx < values.size() ? std::move(values[idx]) : data::Nil();
        if (node.op)
        {
            std::optional<data::GenericValue> maybeValue =
                    data::perform_binary_operation(*node.op, subjectValues[idx], value);
            if (!maybeValue)
                throw LuaRuntimeError(node, "Attempt to apply operator " +
                                            data::BINARY_OP_TYPE_NAMES.at(*node.op) + " to values of types " +
                                            data::get_type_name(subjectValues[idx]) + " and " + data::get_type_name(
                                                value) +
                                            " during assignment");
            value = *maybeValue;
        }

        bool assignedToComponent = false;
        if (idx < subjectValues.size() && std::holds_alternative<data::EntityComponent>(subjectValues[idx]))
        {
            const data::EntityComponent& comp = std::get<data::EntityComponent>(subjectValues[idx]);
            assignedToComponent = data::try_implicitly_write_to_component(comp, value);
            if (!assignedToComponent && subject == data::MaybeFixedValuePtr(nullptr))
                throw LuaRuntimeError(node, "Failed to implicitly assign value of type " +
                    data::get_type_name(value) + " to entity component " + *comp.name);
        }

        if (!assignedToComponent)
        {
            if (auto genericSubject = std::get_if<data::GenericValue*>(&subject))
            {
                **genericSubject = value;
            }
            else if (auto fixedSubject = std::get_if<cmp_info::GenericComponentPtr>(&subject))
            {
                if (idx >= values.size())
                    throw LuaRuntimeError(node, "Not enough arguments to satisfy entity component field assignment");
                performFixedTypeAssignment(node, *fixedSubject, value);
            }
        }
    }

    m_returnedValue.clear();
}

void Interpreter::visit(ast::LocalAssignment& node)
{
    std::vector<data::GenericValue> values;

    for (ast::NodePtr& value : node.values)
    {
        Visitor::visit(value);

        for (data::ValueSequence::ValueBackrefPair& pair : m_returnedValue.sequence)
        {
            values.emplace_back(std::move(pair.value));
        }
    }

    for (size_t idx = 0; idx < node.addresses.size(); ++idx)
    {
        auto& subjectAddr = node.addresses[idx];
        auto& subject = resolveAddress(subjectAddr);
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
                throw LuaRuntimeError(node, "Cannot assign a value of type " + data::get_type_name(value) +
                                            " to numeric component field");
            *ptr = static_cast<int>(std::get<double>(value));
        },
        [&](unsigned* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a value of type " + data::get_type_name(value) +
                                            " to numeric component field");
            *ptr = static_cast<unsigned>(std::get<double>(value));
        },
        [&](float* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a value of type " + data::get_type_name(value) +
                                            " to numeric component field");
            *ptr = static_cast<float>(std::get<double>(value));
        },
        [&](double* ptr)
        {
            if (!std::holds_alternative<double>(value))
                throw LuaRuntimeError(node, "Cannot assign a value of type " + data::get_type_name(value) +
                                            " to numeric component field");
            *ptr = std::get<double>(value);
        },
        [&](std::string* ptr)
        {
            *ptr = data::to_string(value);
        },
        [&](auto* ptr)
        {
            using UnderlyingType = std::remove_reference_t<decltype(*ptr)>;
            if (!std::holds_alternative<UnderlyingType>(value))
                throw LuaRuntimeError(node, "Cannot assign a value of type " + data::get_type_name(value) +
                                            " to the component field");
            *ptr = std::get<UnderlyingType>(value);
        },
    };
    std::visit(reinterpretations, pointer);
}

void Interpreter::indexEntity(ast::IndexRequest& node, data::Entity& entity, const mem_utils::PointerMappedString& index)
{
    if (!cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.contains(index))
        throw LuaRuntimeError(node, "Component " + *index + " is not recognized by the system");
    m_returnedValue = data::EntityComponent(entity, index);
}

void Interpreter::indexEntityComponent(ast::IndexRequest& node, data::EntityComponent& component,
                                       const mem_utils::PointerMappedString& index)
{
    auto foundMapper = cmp_info::ENTITY_MEMBER_MAP.find(*component.name + " " + *index);
    if (foundMapper == cmp_info::ENTITY_MEMBER_MAP.end())
        throw LuaRuntimeError(node, "Component field " + *index + " is not recognized by the system");

    if (!cmp_info::CACHED_ENTITY_COMPONENT_CHECKERS.at(component.name)(component.entity))
    {
        m_returnedValue = data::Nil();
        return;
    }

    auto genericReference = foundMapper->second(component.entity);
    data::GenericValue interpretedValue = data::Nil();
    auto interpretation = meta::Overloads{
        [&](std::string* ptr) { interpretedValue = data::GenericValue(mem_utils::PointerMappedString(*ptr)); },
        [&](bool* ptr) { interpretedValue = data::GenericValue(*ptr); },
        [&](flecs::entity* ptr) { interpretedValue = data::GenericValue(*ptr); },
        [&](auto* ptr) { interpretedValue = data::GenericValue(static_cast<double>(*ptr)); }
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
        if (&argument == &args.back())
        {
            for (data::ValueSequence::ValueBackrefPair& arg : m_returnedValue.sequence)
            {
                arguments.emplace_back(std::move(arg.value));
            }
            m_returnedValue.clear();
        }
        else
        {
            arguments.emplace_back(m_returnedValue.spit());
        }
    }

    m_functionCaller = &node;
    runAnyFunction(func, arguments);
}

void Interpreter::executeFunction(ast::Function& function)
{
    visitTransparentBlock(function.body);
    if (!m_returning) m_returnedValue.clear();
    m_returning = m_breaking = m_continuing = false;
}

void Interpreter::runAnyFunction(data::Function& func, std::vector<data::GenericValue>& arguments)
{
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
            libFunction(state);
        }
        catch (Error& userError)
        {
            throw LuaRuntimeError(*m_functionCaller, userError.message);
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

void Interpreter::runLuaFunction(data::LuaFunction& luaFunction, std::vector<data::GenericValue>& args)
{
    auto prevStackBase = m_stackBasePtr;
    m_stackBasePtr = m_stack.size();
    for (const auto& value : luaFunction.capturedValues)
    {
        m_stack.push_back(value);
    }

    for (size_t idx = 0; idx < luaFunction.body->parameters.size() && idx < args.size(); ++idx)
    {
        data::Address addr;
        addr.relative = true;
        addr.shift = luaFunction.capturedValues.size() + idx;
        resolveAddress(addr) = mem_utils::CopyMovePtr<data::GenericValue>(std::move(args[idx]));
    }

    executeFunction(*luaFunction.body);

    for (unsigned id = 0; id < luaFunction.capturedValues.size(); ++id)
    {
        *luaFunction.capturedValues[id] = std::move(*m_stack[m_stackBasePtr + id]);
    }

    if (m_stack.size() > m_stackBasePtr * 2) m_stack.resize(m_stackBasePtr);
    m_stackBasePtr = prevStackBase;
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

    if (filter.components.empty())
        throw LuaRuntimeError(node, "Entity filter " + *filter.entityName +
                                    " should have at least on component requirement");
    if (filter.components.size() > FLECS_TERM_COUNT_MAX)
        throw LuaRuntimeError(node, "Entity filter " + *filter.entityName +
                                    " cannot have more than " + std::to_string(FLECS_TERM_COUNT_MAX) + " components");

    unsigned termIdx = 0;
    for (const mem_utils::PointerMappedString& component : filter.components)
    {
        auto found = m_componentIds.find(component);
        if (found == m_componentIds.end())
            throw LuaRuntimeError(node, "Component " + *component + " is not recognized by the system");
        desc.terms[termIdx] = {found->second};
        ++termIdx;
    }
    return desc;
}

ecs_query_t* Interpreter::makeEcsQuery(const ast::EcsEntityFilter& filter, ast::INode& node)
{
    ecs_query_desc_t desc = makeEcsQueryDesc(filter, node);
    return ecs_query_init(m_world->c_ptr(), &desc);
}

std::vector<std::string> split_by_dot(const std::string& s)
{
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (true)
    {
        std::size_t pos = s.find('.', start);
        if (pos == std::string::npos)
        {
            parts.push_back(s.substr(start));
            break;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return parts;
}

data::GenericValue* Interpreter::getGlobalValueByName(const mem_utils::PointerMappedString& name)
{
    std::vector<std::string> parts = split_by_dot(*name);
    std::map<mem_utils::PointerMappedString, mem_utils::CopyMovePtr<data::GenericValue> >* mapPtr = &m_globalVariables;

    for (unsigned partIdx = 0; partIdx + 1 < parts.size(); ++partIdx)
    {
        auto newMap = mapPtr->emplace(parts[partIdx], mem_utils::CopyMovePtr<data::GenericValue>(data::Table()));
        data::GenericValue& value = *newMap.first->second;
        if (!std::holds_alternative<data::Table>(value)) return nullptr;
        mapPtr = std::get<data::Table>(value).get();
    }

    return mapPtr->emplace(parts.back(), mem_utils::CopyMovePtr<data::GenericValue>(data::Nil())).first->second.get();
}

const data::GenericValue* Interpreter::getGlobalValueByName(const mem_utils::PointerMappedString& name) const
{
    std::vector<std::string> parts = split_by_dot(*name);
    const std::map<mem_utils::PointerMappedString, mem_utils::CopyMovePtr<data::GenericValue> >* mapPtr = &m_globalVariables;

    for (unsigned partIdx = 0; partIdx + 1 < parts.size(); ++partIdx)
    {
        auto newMap = mapPtr->find(mem_utils::PointerMappedString(parts.back()));
        if (newMap == mapPtr->end()) return nullptr;
        const data::GenericValue& value = *newMap->second;
        if (!std::holds_alternative<data::Table>(value)) return nullptr;
        mapPtr = std::get<data::Table>(value).get();
    }

    auto found = mapPtr->find(mem_utils::PointerMappedString(parts.back()));
    if (found == mapPtr->end()) return nullptr;
    return found->second.get();
}

void Interpreter::runBodyWithinQueries(std::vector<AddressQueryPair>& queries, std::deque<ast::NodePtr>& body,
                                       unsigned iterId)
{
    if (iterId >= queries.size())
    {
        visitTransparentBlock(body);
        return;
    }

    auto& queryPair = queries[iterId];
    ecs_query_t* query = queryPair.query;
    ecs::GuardedEcsIterator iter(ecs_query_iter(m_world->c_ptr(), query));
    while (ecs_query_next(&*iter))
    {
        const data::Address addr = queryPair.address;
        for (long long iterEntityIdx = 0; iterEntityIdx < iter->count; ++iterEntityIdx)
        {
            ecs_entity_t ecsEntity = iter->entities[iterEntityIdx];
            resolveAddress(addr) = mem_utils::CopyMovePtr<data::GenericValue>(
                flecs::entity(iter->world, ecsEntity));
            runBodyWithinQueries(queries, body, iterId + 1);

            m_continuing = false;
            if (m_returning || m_breaking) break;
        }
    }
    iter.finish();
}

void Interpreter::prepareAndRunSystem(ast::System& node, ecs_iter_t* systemIt)
{
    const data::Address addr = node.iteratorAddresses.front();
    for (long long iterEntityIdx = 0; iterEntityIdx < systemIt->count; ++iterEntityIdx)
    {
        ecs_entity_t ecsEntity = systemIt->entities[iterEntityIdx];
        resolveAddress(addr) = mem_utils::CopyMovePtr<data::GenericValue>(
            flecs::entity(systemIt->world, ecsEntity));
        runBodyWithinQueries(m_nodeQueries[&node], node.body, 0);
        if (m_returning || m_breaking) break;
    }
    m_returning = false;
    m_breaking = false;
    m_returnedValue.clear();
}

mem_utils::CopyMovePtr<data::GenericValue>& Interpreter::resolveAddress(const data::Address& address)
{
    unsigned index = address.shift;
    if (address.relative) index += m_stackBasePtr;
    while (m_stack.size() <= index)
    {
        m_stack.emplace_back(data::Nil());
    }
    return m_stack[index];
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
