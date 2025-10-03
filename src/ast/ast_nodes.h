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

namespace flua::ast
{
using namespace flua;

struct Program;

struct AstNode;

using NodePtr = mem_utils::CopyMovePtr<AstNode>;

struct Program
{
    std::deque<NodePtr> components{};
};

class Ast
{
public:
    Program program;
};

struct Function
{
    ids::ResolvableName name{"!UNNAMED_FUNCTION!"};
    std::deque<ids::ResolvableName> parameters{};
    std::deque<NodePtr> body{};
};

struct System
{
    ids::ResolvableName name{"!UNNAMED_SYSTEM!"};
    std::deque<ids::ResolvableName> parameters{};

    std::string query = "!UNRESOLVED_QUERY!";
    std::deque<NodePtr> body{};
};

struct WhileLoop
{
    explicit WhileLoop(NodePtr&& condition);

    NodePtr condition;
    std::deque<NodePtr> body{};
};

struct ForLoopNumeric
{
    explicit ForLoopNumeric(const std::string& name, NodePtr&& base, NodePtr&& limit, NodePtr&& step)
        : name(name), base(std::move(base)), limit(std::move(limit)), step(std::move(step)) {}

    ids::ResolvableName name;
    NodePtr base;
    NodePtr limit;
    NodePtr step;

    std::deque<NodePtr> body{};
};

struct ForLoopGeneric
{
    explicit ForLoopGeneric(NodePtr&& iterator) : iterator(std::move(iterator)) {}

    std::vector<ids::ResolvableName> names{};
    NodePtr iterator;

    std::deque<NodePtr> body{};
};

struct RepeatUntil
{
    explicit RepeatUntil(NodePtr&& condition) : condition(std::move(condition)) {}

    NodePtr condition;
    std::deque<NodePtr> body{};
};

struct Query
{
    std::string query = "!UNRESOLVED_QUERY!";
    ids::ResolvableName entityName{"!UNDEFINED_QUERY_ENTITY_NAME!"};
    std::deque<NodePtr> body{};
};

struct Branch
{
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

struct UnaryOperator
{
    enum class Type
    {
        Negate,
        Not,
        Length,
    };

    UnaryOperator(Type type, NodePtr&& node);

    Type type;
    NodePtr node;
};

struct BinaryOperator
{
    enum class Type
    {
        Add,
        Subtract,
        Multiply,
        Divide,

        Mod,
        Pow,
        FloorDiv,

        And,
        Or,
        Xor,

        ShiftLeft,
        ShiftRight,

        Concatenate,
        
        CmpEq,
        CmpLt,
        CmpLe,
        CmpGt,
        CmpGe,
        CmpNeq,

        Index,
    };

    static const std::map<Type, std::string> kTypeNames;

    BinaryOperator(Type type, NodePtr&& left, NodePtr&& right);

    Type type;
    NodePtr left, right;
};

struct FieldRequest
{
    FieldRequest(NodePtr&& body, std::string field);

    NodePtr body;
    std::string field;
};

struct IndexRequest
{
    IndexRequest(NodePtr&& body, NodePtr&& index);

    NodePtr body;
    NodePtr index;
};

struct Constant
{
    explicit Constant(data::GenericValue value)
        : value(std::move(value))
    {}

    data::GenericValue value;
};

struct MakeTable
{
    struct KeyValuePair
    {
        explicit KeyValuePair(NodePtr&& value) : value(std::move(value)) {}

        std::optional<NodePtr> index{};
        NodePtr value;
    };

    std::deque<KeyValuePair> values{};
};

struct Variable
{
    explicit Variable(const std::string& name) : name(name) {}

    ids::ResolvableName name;
};

struct FunctionCall
{
    explicit FunctionCall(NodePtr&& what) : function(std::move(what)) {}

    NodePtr function;
    std::deque<NodePtr> args;
};

struct Assignment
{
    std::deque<NodePtr> subjects{};
    std::deque<NodePtr> data{};
};

struct Return
{
    std::deque<NodePtr> values{};
};

struct Break {};
struct Continue {};

namespace
{
    using NodeVariant = std::variant<
        Program,
        Function,
        System,
        WhileLoop,
        ForLoopNumeric,
        ForLoopGeneric,
        RepeatUntil,
        Query,
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
