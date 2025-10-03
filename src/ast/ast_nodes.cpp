#include "ast_nodes.h"

namespace flua::ast
{

const std::map<BinaryOperator::Type, std::string> BinaryOperator::kTypeNames = {
    {Type::Add, "Add"},
    {Type::Subtract, "Subtract"},
    {Type::Multiply, "Multiply"},
    {Type::Divide, "Divide"},
    {Type::Mod, "Mod"},
    {Type::Pow, "Pow"},
    {Type::And, "And"},
    {Type::Or, "Or"},
    {Type::Xor, "Xor"},
    {Type::Concatenate, "Concatenate"},
    {Type::CmpEq, "CmpEq"},
    {Type::CmpLt, "CmpLt"},
    {Type::CmpLe, "CmpLe"},
    {Type::CmpGt, "CmpGt"},
    {Type::CmpGe, "CmpGe"},
    {Type::CmpNeq, "CmpNeq"},
};

}
