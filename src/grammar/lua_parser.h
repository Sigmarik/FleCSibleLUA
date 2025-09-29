#pragma once

#include "lua_lexemes.h"
#include "ast/ast_nodes.h"

namespace flua::luagrmr
{
using namespace flua;

struct Program;

using Parser = parser::Parser<Program, lualex::LuaLexer, lualex::LuaIgnored>;

struct Function;
struct ParamNames;
struct Block;
struct Action;
struct Return;

struct Assignment;

struct LValueExpression;
struct ExprComparison;
struct ExprComputation;
struct ExprMultiplication;
struct ExprUnary;
struct ExprSingleton;

struct RValueExpression;

struct Branch;
struct WhileLoop;

struct Program : parser::Grammar
<
    Program, ast::Program,

    parser::Sequence<parser::Lex<parser::Eof>>,
    parser::Sequence<Function, Program>
>
{
    static ast::Program visit(parser::Eof)
    {
        return {};
    }

    static ast::Program visit(ast::Function& function, ast::Program& program)
    {
        program.components.emplace_front(std::move(function));
        return std::move(program);
    }
};

struct Function : parser::Grammar
<
    Function, ast::Function,

    parser::Sequence<
        parser::Lex<lualex::Function>,
        parser::Lex<lualex::Name>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<lualex::Name, parser::Lex<lualex::Name>, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>,
        Block,
        parser::Lex<lualex::End>
    >
>
{
    static ast::Function visit(lualex::Function, const lualex::Name& name, lualex::BracketRoundOp,
        auto& params, lualex::BracketRoundCl, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Function function;
        function.body = std::move(body);
        function.name = name.name;
        for (lualex::Name& paramName : params.parts)
        {
            function.parameters.emplace_back(paramName.name);
        }
        return function;
    }
};

struct ParamNames : parser::Grammar
<
    ParamNames, std::deque<ids::ResolvableName>,

    parser::Sequence<parser::Lex<lualex::Name>, parser::Lex<lualex::Comma>, ParamNames>,
    parser::Sequence<parser::Lex<lualex::Name>>,
    parser::Sequence<>
>
{
    static std::deque<ids::ResolvableName> visit()
    {
        return {};
    }

    static std::deque<ids::ResolvableName> visit(const lualex::Name& name)
    {
        return {ids::ResolvableName(name.name)};
    }

    static std::deque<ids::ResolvableName> visit(const lualex::Name& name, lualex::Comma,
        std::deque<ids::ResolvableName>& params)
    {
        std::deque<ids::ResolvableName> newParams = std::move(params);
        newParams.emplace_front(name.name);
        return newParams;
    }
};

struct Block : parser::Grammar
<
    Block, std::deque<ast::NodePtr>,

    parser::Sequence<Action, Block>,
    parser::Sequence<Return>,
    parser::Sequence<>
>
{
    static std::deque<ast::NodePtr> visit(ast::NodePtr& action, std::deque<ast::NodePtr>& body)
    {
        std::deque<ast::NodePtr> newBody = std::move(body);
        newBody.emplace_front(action);
        return newBody;
    }

    static std::deque<ast::NodePtr> visit(ast::Return& ret)
    {
        return {ast::NodePtr(std::move(ret))};
    }

    static std::deque<ast::NodePtr> visit()
    {
        return {};
    }
};

struct Action : parser::Grammar
<
    Action, ast::NodePtr,

    parser::Sequence<Assignment>,
    parser::Sequence<ExprUnary>,
    parser::Sequence<Branch>,
    parser::Sequence<WhileLoop>
    // TODO: Add assignments, branches, for/while loops and other stuff
>
{
    template <class T>
    static ast::NodePtr visit(T& exactNode)
    {
        return ast::NodePtr(std::move(exactNode));
    }
};

struct Return : parser::Grammar
<
    Return, ast::Return,

    parser::Sequence<parser::Lex<lualex::Return>, LValueExpression>,
    parser::Sequence<parser::Lex<lualex::Return>>
    // TODO: Add return with an argument
>
{
    static ast::Return visit(lualex::Return)
    {
        return {};
    }

    static ast::Return visit(lualex::Return, ast::NodePtr& value)
    {
        return {std::move(value)};
    }
};

struct LValueExpression : parser::Grammar
<
    LValueExpression, ast::NodePtr,

    parser::Sequence<ExprComparison>,
    parser::Sequence<LValueExpression, parser::Lex<lualex::And>, ExprComparison>,
    parser::Sequence<LValueExpression, parser::Lex<lualex::Or>, ExprComparison>
>
{
    static ast::NodePtr visit(ast::NodePtr& node)
    {
        return std::move(node);
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::And, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::And, std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::Or, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::Or, std::move(left), std::move(right))};
    }
};

struct ExprComparison : parser::Grammar
<
    ExprComparison, ast::NodePtr,

    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpEq>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpGe>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpGt>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpLe>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpLt>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpNeq>, ExprComputation>,
    parser::Sequence<ExprComputation>
>
{
    static ast::NodePtr visit(ast::NodePtr& node)
    {
        return std::move(node);
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpEq, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::CmpEq, std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpGe, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::CmpGe, std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpGt, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::CmpGt, std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpLe, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::CmpLe, std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpLt, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::CmpLt, std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpNeq, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::CmpNeq, std::move(left), std::move(right))};
    }
};

struct ExprComputation : parser::Grammar
<
    ExprComputation, ast::NodePtr,

    parser::Sequence<ExprComputation, parser::Lex<lualex::Plus>, ExprMultiplication>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::Minus>, ExprMultiplication>,
    parser::Sequence<ExprMultiplication>
>
{
    static ast::NodePtr visit(ast::NodePtr& value)
    {
        return std::move(value);
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::Plus, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::Add,
            std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::Minus, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::Subtract,
            std::move(left), std::move(right))};
    }
};
struct ExprMultiplication : parser::Grammar
<
    ExprMultiplication, ast::NodePtr,

    parser::Sequence<ExprUnary>,
    parser::Sequence<ExprMultiplication, parser::Lex<lualex::Multiply>, ExprUnary>,
    parser::Sequence<ExprMultiplication, parser::Lex<lualex::Divide>, ExprUnary>,
    parser::Sequence<ExprMultiplication, parser::Lex<lualex::Mod>, ExprUnary>
>
{
    static ast::NodePtr visit(ast::NodePtr& node)
    {
        return std::move(node);
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::Multiply, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::Multiply,
            std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::Divide, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::Divide,
            std::move(left), std::move(right))};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::Mod, ast::NodePtr& right)
    {
        return ast::NodePtr{ast::BinaryOperator(ast::BinaryOperator::Type::Mod,
            std::move(left), std::move(right))};
    }
};

struct ExprUnary : parser::Grammar
<
    ExprUnary, ast::NodePtr,

    parser::Sequence<parser::Lex<lualex::Minus>, ExprUnary>,
    parser::Sequence<parser::Lex<lualex::Length>, ExprUnary>,
    parser::Sequence<parser::Lex<lualex::Not>, ExprUnary>,
    parser::Sequence<ExprSingleton>
>
{
    static ast::NodePtr visit(lualex::Minus, ast::NodePtr& value)
    {
        return ast::NodePtr{ast::UnaryOperator(ast::UnaryOperator::Type::Negate, std::move(value))};
    }

    static ast::NodePtr visit(lualex::Length, ast::NodePtr& value)
    {
        return ast::NodePtr{ast::UnaryOperator(ast::UnaryOperator::Type::Length, std::move(value))};
    }

    static ast::NodePtr visit(lualex::Not, ast::NodePtr& value)
    {
        return ast::NodePtr{ast::UnaryOperator(ast::UnaryOperator::Type::Not, std::move(value))};
    }

    static ast::NodePtr visit(ast::NodePtr& value)
    {
        return std::move(value);
    }
};

struct ExprSingleton : parser::Grammar
<
    ExprSingleton, ast::NodePtr,

    parser::Sequence<parser::Lex<lualex::BracketRoundOp>, LValueExpression, parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<ExprSingleton,
        parser::Lex<lualex::BracketSquareOp>, LValueExpression, parser::Lex<lualex::BracketSquareCl>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::Dot>, parser::Lex<lualex::Name>>,
    parser::Sequence<parser::Lex<lualex::Name>>,
    parser::Sequence<parser::Lex<lualex::Number>>,
    parser::Sequence<parser::Lex<lualex::String>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::BracketRoundOp>,
        parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<ast::NodePtr, LValueExpression, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>>
>
{
    static ast::NodePtr visit(ast::NodePtr& body, lualex::BracketSquareOp, ast::NodePtr& index, lualex::BracketSquareCl)
    {
        return ast::NodePtr{ast::IndexRequest(std::move(body), std::move(index))};
    }

    static ast::NodePtr visit(ast::NodePtr& body, lualex::Dot, const lualex::Name& field)
    {
        ast::NodePtr index = ast::NodePtr{ast::Constant(field.name)};
        return ast::NodePtr{ast::IndexRequest(std::move(body), std::move(index))};
    }

    static ast::NodePtr visit(lualex::BracketRoundOp, ast::NodePtr& value, lualex::BracketRoundCl)
    {
        return std::move(value);
    }

    static ast::NodePtr visit(const lualex::Name& varName)
    {
        return ast::NodePtr{ast::Variable(varName.name)};
    }

    static ast::NodePtr visit(const lualex::Number& constNumber)
    {
        return ast::NodePtr{ast::Constant(constNumber.value)};
    }

    static ast::NodePtr visit(const lualex::String& constString)
    {
        return ast::NodePtr{ast::Constant(constString.string)};
    }

    static ast::NodePtr visit(ast::NodePtr& func, lualex::BracketRoundOp, lualex::BracketRoundCl)
    {
        ast::FunctionCall function(std::move(func));
        return ast::NodePtr{std::move(function)};
    }

    static ast::NodePtr visit(ast::NodePtr& func, lualex::BracketRoundOp,
        parser::Alternating<ast::NodePtr, lualex::Comma>& args, lualex::BracketRoundCl)
    {
        ast::FunctionCall function(std::move(func));
        for (ast::NodePtr& arg : args.parts)
        {
            function.args.emplace_front(std::move(arg));
        }
        return ast::NodePtr{std::move(function)};
    }
};

struct Assignment : parser::Grammar
<
    Assignment, ast::Assignment,

    // TODO: Implement batch assignment
    // TODO: Implement op= assignment
    parser::Sequence<ExprSingleton, parser::Lex<lualex::Assignment>, LValueExpression>
>
{
    static ast::Assignment visit(ast::NodePtr& subject, lualex::Assignment, ast::NodePtr& value)
    {
        return ast::Assignment{std::move(subject), std::move(value)};
    }
};

struct BranchBgn;

struct Branch : parser::Grammar
<
    Branch, ast::Branch,

    parser::Sequence<BranchBgn, parser::Lex<lualex::End>>,
    parser::Sequence<BranchBgn, parser::Lex<lualex::Else>, Block, parser::Lex<lualex::End>>
>
{
    static ast::Branch visit(ast::Branch& branch, lualex::End)
    {
        return ast::Branch{std::move(branch)};
    }

    static ast::Branch visit(ast::Branch& branch, lualex::Else, std::deque<ast::NodePtr>& block, lualex::End)
    {
        branch.ifFalse = std::move(block);
        return ast::Branch{std::move(branch)};
    }
};

struct BranchBgn : parser::Grammar
<
    BranchBgn, ast::Branch,

    parser::Sequence<parser::Lex<lualex::If>, LValueExpression, parser::Lex<lualex::Then>, Block>,
    parser::Sequence<BranchBgn, parser::Lex<lualex::ElseIf>, LValueExpression, parser::Lex<lualex::Then>, Block>
>
{
    static ast::Branch visit(ast::Branch& branch, lualex::ElseIf, ast::NodePtr& condition,
        lualex::Then, std::deque<ast::NodePtr>& block)
    {
        branch.cases.emplace_back(std::move(condition), std::move(block));
        return std::move(branch);
    }

    static ast::Branch visit(lualex::If, ast::NodePtr& condition, lualex::Then, std::deque<ast::NodePtr>& block)
    {
        ast::Branch branch;
        branch.cases.emplace_back(std::move(condition), std::move(block));
        return branch;
    }
};

struct WhileLoop : parser::Grammar
<
    WhileLoop, ast::Loop,

    parser::Sequence<parser::Lex<lualex::While>, LValueExpression, parser::Lex<lualex::Do>, Block,
        parser::Lex<lualex::End>>
>
{
    static ast::Loop visit(lualex::While, ast::NodePtr& condition, lualex::Do,
        std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Loop loop(std::move(condition));
        loop.body = std::move(body);
        return loop;
    }
};

}
