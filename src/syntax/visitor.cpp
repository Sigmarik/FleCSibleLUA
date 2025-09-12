#include "visitor.h"

namespace flua::syntax
{

void Visitor::process(Ast& ast)
{
    visit(ast.program);
}

void Visitor::visit(AstNode& node)
{
    std::visit([&](auto& specificNode) { visit(specificNode); }, node);
}

void Visitor::visit(NodePtr& ptr)
{
    visit(*ptr);
}

}
