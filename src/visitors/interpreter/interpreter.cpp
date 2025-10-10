#include "interpreter.h"

namespace flua::vst
{
using namespace flua;


void Interpreter::visit(ast::Program& node)
{
    for (ast::NodePtr& component : node.components)
    {
        // Visitor::visit(component);
    }
}
}
