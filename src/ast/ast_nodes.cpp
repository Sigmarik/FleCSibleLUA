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
    {Type::FloorDiv, "FloorDiv"},
    {Type::And, "And"},
    {Type::Or, "Or"},
    {Type::Xor, "Xor"},
    {Type::ShiftLeft, "ShiftLeft"},
    {Type::ShiftRight, "ShiftRight"},
    {Type::Concatenate, "Concatenate"},
    {Type::CmpEq, "CmpEq"},
    {Type::CmpLt, "CmpLt"},
    {Type::CmpLe, "CmpLe"},
    {Type::CmpGt, "CmpGt"},
    {Type::CmpGe, "CmpGe"},
    {Type::Index, "Index"},
};

Loop::Loop(NodePtr&& condition) : condition(std::move(condition)) {}

Branch::Branch(NodePtr&& condition) : condition(std::move(condition)) {}

UnaryOperator::UnaryOperator(Type type, NodePtr&& node) : type(type), node(std::move(node)) {}

BinaryOperator::BinaryOperator(Type type, NodePtr&& left, NodePtr&& right)
        : type(type), left(std::move(left)), right(std::move(right))
{}

FieldRequest::FieldRequest(NodePtr&& body, std::string field)
        : body(std::move(body)), field(std::move(field))
{}

IndexRequest::IndexRequest(NodePtr&& body, NodePtr&& index)
        : body(std::move(body)), index(std::move(index))
{}

Assignment::Assignment(NodePtr&& subject, NodePtr&& data) : subject(std::move(subject)), data(std::move(data)) {}
}
