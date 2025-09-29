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
    m_stream << m_indent << "Function " << node.name.string << "( ";
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
    m_stream << m_indent << "System (" << node.query << ") " << node.name.string << "( ";
    for (ids::ResolvableName& param : node.parameters)
    {
        m_stream << param.string << " ";
    }
    m_stream << ") {\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::Loop& node)
{
    m_stream << m_indent << "Loop (\n";
    increaseIndent();
    Visitor::visit(node.condition);
    decreaseIndent();
    m_stream << ") {\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::Query& node)
{
    m_stream << m_indent << "Query (\n";
    increaseIndent();
    m_stream << m_indent << "query: " << node.query << "\n";
    m_stream << m_indent << "entity: " << node.entityName.string << "\n";
    decreaseIndent();
    m_stream << m_indent << ") {\n";
    visitList(node.body);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::Branch& node)
{
    m_stream << m_indent << "If (\n";
    increaseIndent();
    Visitor::visit(node.condition);
    decreaseIndent();
    m_stream << ") {\n";
    visitList(node.ifTrue);
    m_stream << m_indent << "} else {\n";
    visitList(node.ifFalse);
    m_stream << m_indent << "}\n";
}

void AstDebugger::visit(ast::FunctionCall& node)
{
    m_stream << m_indent << "FunctionCall " << node.name.string << "(\n";
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
    m_stream << "]\n";
}

void AstDebugger::visit(ast::Constant& node)
{
    m_stream << m_indent << "Constant " << data::to_string(node.value) << "\n";
}

void AstDebugger::visit(ast::Variable& node)
{
    m_stream << m_indent << "Variable " << node.name.string << "\n";
}

void AstDebugger::visit(ast::Assignment& node)
{
    m_stream << m_indent << "Assignment to (\n";
    increaseIndent();
    Visitor::visit(node.subject);
    decreaseIndent();
    m_stream << m_indent << ") of (\n";
    increaseIndent();
    Visitor::visit(node.data);
    decreaseIndent();
    m_stream << ")\n";
}

void AstDebugger::visit(ast::Return& node)
{
    m_stream << m_indent << "Return";
    if (node.value)
    {
        m_stream << "(\n";
        increaseIndent();
        Visitor::visit(*node.value);
        decreaseIndent();
        m_stream << m_indent << ")\n";
    } else
    {
        m_stream << "\n";
    }
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
