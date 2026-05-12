#include "address_resolver.h"

namespace flua::vst
{

void AddressResolver::visit(ast::Program& node)
{
    visitList(node.components);
}

void AddressResolver::visit(ast::Function& node)
{
    std::map<mem_utils::PointerMappedString, data::Address> capturableValues;
    for (auto it = m_stack.rbegin(); it + 1 != m_stack.rend(); ++it)
    {
        for (const auto& [key, value] : *it)
        {
            capturableValues.emplace(key, value);
        }
    }

    auto oldAddress = m_currentAddress;
    m_currentAddress = 0;
    m_stack.emplace_back();

    for (const auto& [key, value] : capturableValues)
    {
        data::Address localAddr;
        localAddr.shift = m_currentAddress++;
        localAddr.relative = true;
        localAddr.resetBeforeUse = false;
        node.valuesToCapture.emplace_back(value);
        m_stack.back().emplace(key, localAddr);
    }

    visitList(node.body);
    m_stack.pop_back();
    m_currentAddress = oldAddress;
}

void AddressResolver::visit(ast::System& node)
{
    auto oldAddress = m_currentAddress;
    m_currentAddress = 0;
    m_stack.emplace_back();
    for (const ast::EcsEntityFilter& entity : node.entities)
    {
        node.iteratorAddresses.emplace_back(resolveLocal(entity.entityName));
    }
    visitList(node.body);
    m_stack.pop_back();
    m_currentAddress = oldAddress;
}

void AddressResolver::visit(ast::WhileLoop& node)
{
    m_stack.emplace_back();
    Visitor::visit(node.condition);
    visitList(node.body);
    m_stack.pop_back();
}

void AddressResolver::visit(ast::ForLoopNumeric& node)
{
    m_stack.emplace_back();
    node.iteratorAddress = resolveLocal(node.name);
    Visitor::visit(node.base);
    Visitor::visit(node.limit);
    Visitor::visit(node.step);
    visitList(node.body);
    m_stack.pop_back();
}

void AddressResolver::visit(ast::ForLoopGeneric& node)
{
    m_stack.emplace_back();
    for (const mem_utils::PointerMappedString& name : node.names)
    {
        node.iteratorAddresses.emplace_back(resolveLocal(name));
    }
    Visitor::visit(node.iterator);
    visitList(node.body);
    m_stack.pop_back();
}

void AddressResolver::visit(ast::Query& node)
{
    m_stack.emplace_back();
    for (const ast::EcsEntityFilter& entity : node.filters)
    {
        node.iteratorAddresses.emplace_back(resolveLocal(entity.entityName));
    }
    visitList(node.body);
    m_stack.pop_back();
}

void AddressResolver::visit(ast::RepeatUntil& node)
{
    m_stack.emplace_back();
    visitList(node.body);
    Visitor::visit(node.condition);
    m_stack.pop_back();
}

void AddressResolver::visit(ast::Branch& node)
{
    for (auto & cs : node.cases)
    {
        m_stack.emplace_back();
        Visitor::visit(cs.condition);
        visitList(cs.block);
        m_stack.pop_back();
    }
    m_stack.emplace_back();
    visitList(node.ifFalse);
    m_stack.pop_back();
}

void AddressResolver::visit(ast::FunctionCall& node)
{
    visit(node.function);
    visitList(node.args);
}

void AddressResolver::visit(ast::UnaryOperator& node)
{
    Visitor::visit(node.node);
}

void AddressResolver::visit(ast::BinaryOperator& node)
{
    Visitor::visit(node.left);
    Visitor::visit(node.right);
}

void AddressResolver::visit(ast::FieldRequest& node)
{
    Visitor::visit(node.body);
}

void AddressResolver::visit(ast::IndexRequest& node)
{
    Visitor::visit(node.body);
    Visitor::visit(node.index);
}

void AddressResolver::visit(ast::Constant& node)
{
    // Do nothing
}

void AddressResolver::visit(ast::MakeTable& node)
{
    for (ast::MakeTable::KeyValuePair& pair : node.values)
    {
        if (pair.index)
        {
            Visitor::visit(*pair.index);
        }

        Visitor::visit(pair.value);
    }
}

void AddressResolver::visit(ast::Variable& node)
{
    node.resolvedAddress = resolveUnknown(node.name);
}

void AddressResolver::visit(ast::Assignment& node)
{
    visitList(node.subjects);
    visitList(node.data);
}

void AddressResolver::visit(ast::LocalAssignment& node)
{
    for (mem_utils::PointerMappedString& name : node.names)
    {
        resolveLocal(name);
    }
    visitList(node.values);
}

void AddressResolver::visit(ast::Return& node)
{
    visitList(node.values);
}
void AddressResolver::visit(ast::Break& node)
{
    // Do nothing
}

void AddressResolver::visit(ast::Continue& node)
{
    // Do nothing
}

void AddressResolver::visitList(std::deque<ast::NodePtr>& nodes)
{
    for (ast::NodePtr& ptr : nodes)
    {
        Visitor::visit(ptr);
    }
}

data::Address AddressResolver::resolveLocal(const mem_utils::PointerMappedString& name)
{
    data::Address addr;
    addr.resetBeforeUse = false;
    addr.shift = m_currentAddress++;
    addr.relative = m_stack.size() > 1;
    auto emplaced = m_stack.back().emplace(name, addr);
    return emplaced.first->second;
}

std::optional<data::Address> AddressResolver::resolveUnknown(const mem_utils::PointerMappedString& name)
{
    for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it)
    {
        auto found = it->find(name);
        if (found != it->end())
        {
            return found->second;
        }
    }

    return std::nullopt;
}
}
