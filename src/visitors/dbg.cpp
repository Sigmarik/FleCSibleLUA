#include "dbg.h"

namespace flua::vst
{

void AstDebugger::visit(ast::Program& node)
{
    m_stream << m_indent << "Program {\n";
    visitList(node.components);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::Function& node)
{
    m_stream << m_indent << "Function ( ";
    for (ids::ResolvableName& param : node.parameters)
    {
        m_stream << param.string << " ";
    }
    m_stream << ") {\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::System& node)
{
    m_stream << m_indent << "System ( ";
    for (const ast::System::SystemEntity& entity : node.entities)
    {
        m_stream << entity.entityName.string << " ( ";
        for (const std::string& component : entity.components)
        {
            m_stream << component << " ";
        }
        m_stream << ") ";
    }
    m_stream << ") {\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::WhileLoop& node)
{
    m_stream << m_indent << "While (\n";
    increaseIndent();
    Visitor::visit(node.condition);
    decreaseIndent();
    m_stream << m_indent << ") {\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::ForLoopNumeric& node)
{
    m_stream << m_indent << "For " << node.name.string << " from\n";
    increaseIndent();
    Visitor::visit(node.base);
    decreaseIndent();
    m_stream << m_indent << "to\n";
    increaseIndent();
    Visitor::visit(node.limit);
    decreaseIndent();
    m_stream << m_indent << "step\n";
    increaseIndent();
    Visitor::visit(node.step);
    decreaseIndent();
    m_stream << m_indent << "{\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::ForLoopGeneric& node)
{
    m_stream << m_indent << "For ";
    for (const ids::ResolvableName& name : node.names)
    {
        m_stream << name.string << " ";
    }
    m_stream << "in\n";
    increaseIndent();
    Visitor::visit(node.iterator);
    decreaseIndent();
    m_stream << m_indent << "{\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::RepeatUntil& node)
{
    m_stream << m_indent << "Repeat {\n";
    visitList(node.body);
    m_stream << m_indent << "} until (\n";
    increaseIndent();
    Visitor::visit(node.condition);
    decreaseIndent();
    m_stream << m_indent << ")\n";
}

void AstDebugger::visit(ast::Branch& node)
{
    for (size_t id = 0; id < node.cases.size(); ++id)
    {
        ast::Branch::Case& cs = node.cases[id];
        m_stream << m_indent << (id == 0 ? "If (\n" : "ElseIf (\n");
        increaseIndent();
        Visitor::visit(cs.condition);
        decreaseIndent();
        m_stream << m_indent << ") {\n";
        visitList(cs.block);
    }
    m_stream << m_indent << "} else {\n";
    visitList(node.ifFalse);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::FunctionCall& node)
{
    m_stream << m_indent << "FunctionCall of\n";
    increaseIndent();
    visit(node.function);
    decreaseIndent();
    visitList(node.args);
    m_stream << m_indent << ")\n";
}

void AstDebugger::visit(ast::UnaryOperator& node)
{
    m_stream << m_indent << "Unary ";
    switch (node.type)
    {
        case ast::UnaryOperator::Type::Length:
            m_stream << "len";
            break;
        case ast::UnaryOperator::Type::Negate:
            m_stream << "negation";
            break;
        case ast::UnaryOperator::Type::Not:
            m_stream << "not";
            break;
    }
    m_stream << "(\n";
    increaseIndent();
    Visitor::visit(node.node);
    decreaseIndent();
    m_stream << ")\n";
}

void AstDebugger::visit(ast::BinaryOperator& node)
{
    m_stream << m_indent << "Binary " << ast::BinaryOperator::kTypeNames.at(node.type) << "(\n";
    increaseIndent();
    Visitor::visit(node.left);
    decreaseIndent();
    m_stream << m_indent << ") and (\n";
    increaseIndent();
    Visitor::visit(node.right);
    decreaseIndent();
    m_stream << m_indent << ")\n";
}

void AstDebugger::visit(ast::FieldRequest& node)
{
    m_stream << m_indent << " FieldRequest (\n";
    increaseIndent();
    Visitor::visit(node.body);
    decreaseIndent();
    m_stream << ")." << node.field << "\n";
}

void AstDebugger::visit(ast::IndexRequest& node)
{
    m_stream << m_indent << "IndexRequest (\n";
    increaseIndent();
    Visitor::visit(node.body);
    decreaseIndent();
    m_stream << m_indent << ") [\n";
    increaseIndent();
    Visitor::visit(node.index);
    decreaseIndent();
    m_stream << m_indent << "]\n";
}

void AstDebugger::visit(ast::Constant& node)
{
    m_stream << m_indent << "Constant " << data::to_string(node.value) << "\n";
}

void AstDebugger::visit(ast::MakeTable& node)
{
    m_stream << m_indent << "Table {\n";
    increaseIndent();
    for (ast::MakeTable::KeyValuePair& pair : node.values)
    {
        if (pair.index)
        {
            m_stream << m_indent << "Key\n";
            increaseIndent();
            Visitor::visit(*pair.index);
            decreaseIndent();
        }

        m_stream << m_indent << "Value\n";
        increaseIndent();
        Visitor::visit(pair.value);
        decreaseIndent();
    }
    decreaseIndent();
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::Variable& node)
{
    m_stream << m_indent << "Variable " << node.name.string << "\n";
}

void AstDebugger::visit(ast::Assignment& node)
{
    m_stream << m_indent << "Assignment to (\n";
    visitList(node.subjects);
    m_stream << m_indent << ") of (\n";
    visitList(node.data);
    m_stream << m_indent << ")\n";
}

void AstDebugger::visit(ast::LocalAssignment& node)
{
    m_stream << m_indent << "LocalAssignment to\n";
    increaseIndent();
    for (ids::ResolvableName& name : node.names)
    {
        m_stream << m_indent << name.string << "\n";
    }
    decreaseIndent();
    m_stream << m_indent << "of (\n";
    visitList(node.values);
    m_stream << m_indent << ")\n";
}

void AstDebugger::visit(ast::Return& node)
{
    m_stream << m_indent << "Return\n";
    visitList(node.values);
}
void AstDebugger::visit(ast::Break& node)
{
    m_stream << m_indent << "Break\n";
}

void AstDebugger::visit(ast::Continue& node)
{
    m_stream << m_indent << "Continue\n";
}


void AstDebugger::increaseIndent()
{
    m_indent += "  ";
}

void AstDebugger::decreaseIndent()
{
    m_indent.pop_back();
    m_indent.pop_back();
}

void AstDebugger::visitList(std::deque<ast::NodePtr>& nodes)
{
    increaseIndent();
    for (ast::NodePtr& ptr : nodes)
    {
        Visitor::visit(ptr);
    }
    decreaseIndent();
}

}
