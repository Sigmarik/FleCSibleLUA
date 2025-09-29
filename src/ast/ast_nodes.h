#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <deque>

#include "data_types.h"
#include "identification.h"
#include "mem_utils/copyable_ptr.h"

namespace flua::ast
{
using namespace flua;

struct Program;
struct Function;
struct System;

struct Loop;
struct Query;
struct Branch;

struct FunctionCall;
struct UnaryOperator;
struct BinaryOperator;

struct FieldRequest;
struct IndexRequest;
struct Constant;
struct Variable;

struct Assignment;
struct Return;
struct Break;
struct Continue;

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

struct Loop
{
    explicit Loop(NodePtr&& condition);

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
    explicit Branch(NodePtr&& condition);

    NodePtr condition;
    std::deque<NodePtr> ifTrue{};
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
    Assignment(NodePtr&& subject, NodePtr&& data);

    NodePtr subject;
    NodePtr data;
};

struct Return
{
    std::optional<NodePtr> value{};
};

struct Break {};
struct Continue {};

namespace
{
    using NodeVariant = std::variant<
        Program,
        Function,
        System,
        Loop,
        Query,
        Branch,
        UnaryOperator,
        BinaryOperator,
        FieldRequest,
        IndexRequest,
        Constant,
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
