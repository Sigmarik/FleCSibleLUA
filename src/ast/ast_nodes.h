#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <deque>
#include <vector>

#include "data_types.h"
#include "identification.h"
#include "mem_utils/copyable_ptr.h"
#include "parser/char_pos.h"

namespace flua::ast
{
using namespace flua;

struct INode
{
    explicit INode(const parser::CharacterPos& pos) : m_position(pos) {}

    INode(const INode&) = default;
    INode(INode&&) = default;

    INode& operator=(const INode&) = default;
    INode& operator=(INode&&) = default;

    [[nodiscard]] parser::CharacterPos getPos() const { return m_position; }

private:
    parser::CharacterPos m_position;
};

struct Program;

struct AstNode;

using NodePtr = mem_utils::CopyMovePtr<AstNode>;

struct Program : INode
{
    using INode::INode;

    std::deque<NodePtr> components{};
};

class Ast
{
public:
    Program program;
};

struct Function : INode
{
    using INode::INode;

    ids::ResolvableName name{"!UNNAMED_FUNCTION!"};
    std::deque<ids::ResolvableName> parameters{};
    std::deque<NodePtr> body{};
};

struct WhileLoop : INode
{
    explicit WhileLoop(const parser::CharacterPos& pos, NodePtr&& condition)
        : INode(pos), condition(std::move(condition)) {}

    NodePtr condition;
    std::deque<NodePtr> body{};
};

struct ForLoopNumeric : INode
{
    explicit ForLoopNumeric(const parser::CharacterPos& pos, const std::string& name, NodePtr&& base, NodePtr&& limit,
        NodePtr&& step)
        : INode(pos), name(name), base(std::move(base)), limit(std::move(limit)), step(std::move(step)) {}

    ids::ResolvableName name;
    NodePtr base;
    NodePtr limit;
    NodePtr step;

    std::deque<NodePtr> body{};
};

struct ForLoopGeneric : INode
{
    explicit ForLoopGeneric(const parser::CharacterPos& pos, NodePtr&& iterator)
        : INode(pos), iterator(std::move(iterator)) {}

    std::vector<ids::ResolvableName> names{};
    NodePtr iterator;

    std::deque<NodePtr> body{};
};

struct RepeatUntil : INode
{
    explicit RepeatUntil(const parser::CharacterPos& pos, NodePtr&& condition)
        : INode(pos), condition(std::move(condition)) {}

    NodePtr condition;
    std::deque<NodePtr> body{};
};

struct Branch : INode
{
    using INode::INode;

    struct Case
    {
        Case(NodePtr&& condition, std::deque<NodePtr>&& block)
            : condition(std::move(condition)), block(std::move(block))
        {}

        NodePtr condition;
        std::deque<NodePtr> block{};
    };

    std::vector<Case> cases{};
    std::deque<NodePtr> ifFalse{};
};

struct UnaryOperator : INode
{
    enum class Type
    {
        Negate,
        Not,
        Length,
    };

    UnaryOperator(const parser::CharacterPos& pos, Type type, NodePtr&& node)
        : INode(pos), type(type), node(std::move(node)) {}

    Type type;
    NodePtr node;
};

struct BinaryOperator : INode
{
    enum class Type
    {
        Add,
        Subtract,
        Multiply,
        Divide,

        Mod,
        Pow,

        And,
        Or,
        Xor,

        Concatenate,
        
        CmpEq,
        CmpLt,
        CmpLe,
        CmpGt,
        CmpGe,
        CmpNeq
    };

    static const std::map<Type, std::string> kTypeNames;

    BinaryOperator(const parser::CharacterPos& pos, Type type, NodePtr&& left, NodePtr&& right)
        : INode(pos), type(type), left(std::move(left)), right(std::move(right)) {}

    Type type;
    NodePtr left, right;
};

struct FieldRequest : INode
{
    FieldRequest(const parser::CharacterPos& pos, NodePtr&& body, std::string field)
        : INode(pos), body(std::move(body)), field(std::move(field)) {}

    NodePtr body;
    std::string field;
};

struct IndexRequest : INode
{
    IndexRequest(const parser::CharacterPos& pos, NodePtr&& body, NodePtr&& index)
        : INode(pos), body(std::move(body)), index(std::move(index)) {}

    NodePtr body;
    NodePtr index;
};

struct Constant : INode
{
    explicit Constant(const parser::CharacterPos& pos, data::GenericValue value)
        : INode(pos), value(std::move(value)) {}

    data::GenericValue value;
};

struct MakeTable : INode
{
    using INode::INode;

    struct KeyValuePair
    {
        explicit KeyValuePair(NodePtr&& value) : value(std::move(value)) {}

        std::optional<NodePtr> index{};
        NodePtr value;
    };

    std::deque<KeyValuePair> values{};
};

struct Variable : INode
{
    explicit Variable(const parser::CharacterPos& pos, const std::string& name) : INode(pos), name(name) {}

    ids::ResolvableName name;
};

struct FunctionCall : INode
{
    explicit FunctionCall(const parser::CharacterPos& pos, NodePtr&& what) : INode(pos), function(std::move(what)) {}

    NodePtr function;
    std::deque<NodePtr> args;
};

struct Assignment : INode
{
    using INode::INode;

    std::deque<NodePtr> subjects{};
    std::deque<NodePtr> data{};
};

struct LocalAssignment : INode
{
    using INode::INode;

    std::deque<ids::ResolvableName> names;
    std::deque<NodePtr> values{};
};

struct Return : INode
{
    using INode::INode;
    std::deque<NodePtr> values{};
};

struct Break : INode
{
    using INode::INode;
};

struct Continue : INode
{
    using INode::INode;
};

namespace
{
    using NodeVariant = std::variant<
        Program,
        Function,
        WhileLoop,
        ForLoopNumeric,
        ForLoopGeneric,
        RepeatUntil,
        Branch,
        UnaryOperator,
        BinaryOperator,
        FieldRequest,
        IndexRequest,
        Constant,
        MakeTable,
        Variable,
        FunctionCall,
        Assignment,
        LocalAssignment,
        Return,
        Break,
        Continue
    >;
}

struct AstNode : NodeVariant
{
    using NodeVariant::variant;
};

}
