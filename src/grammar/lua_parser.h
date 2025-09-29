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

struct FunctionCall;
struct Assignment;

struct LValueExpression;
struct ExprComparison;
struct ExprComputation;
struct ExprMultiplication;
struct ExprUnary;
struct ExprSingleton;

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
        parser::Repeating<ast::NodePtr, Action>,
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

    parser::Sequence<FunctionCall>
    // TODO: Add assignments, branches, for/while loops and other stuff
>
{
    template <class T>
    static ast::NodePtr visit(T& exactNode)
    {
        return ast::NodePtr(std::move(exactNode));
    }
};

struct FunctionCall : parser::Grammar
<
    FunctionCall, ast::FunctionCall,

    parser::Sequence<parser::Lex<lualex::Name>, parser::Lex<lualex::BracketRoundOp>,
        parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<parser::Lex<lualex::Name>, parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<ast::NodePtr, LValueExpression, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>>
    // TODO: Consider function arguments
>
{
    static ast::FunctionCall visit(const lualex::Name& name, lualex::BracketRoundOp, lualex::BracketRoundCl)
    {
        ast::FunctionCall function;
        function.name = ids::ResolvableName(name.name);
        return function;
    }

    static ast::FunctionCall visit(const lualex::Name& name, lualex::BracketRoundOp,
        parser::Alternating<ast::NodePtr, lualex::Comma>& args, lualex::BracketRoundCl)
    {
        ast::FunctionCall function;
        function.name = ids::ResolvableName(name.name);
        for (ast::NodePtr& arg : args.parts)
        {
            function.args.emplace_front(std::move(arg));
        }
        return function;
    }
};

struct Return : parser::Grammar
<
    Return, ast::Return,

    parser::Sequence<parser::Lex<lualex::Return>>
    // TODO: Add return with an argument
>
{
    static ast::Return visit(lualex::Return)
    {
        return {};
    }
};

struct LValueExpression : parser::Grammar
<
    LValueExpression, ast::NodePtr,

    parser::Sequence<parser::Repeating<ast::NodePtr, ExprComparison, lualex::And, lualex::Or>>
>
{
    static ast::NodePtr visit(parser::Alternating<ast::NodePtr, lualex::And, lualex::Or>& alternating)
    {
        ast::NodePtr result = std::move(alternating.parts.front());
        for (size_t id = 1; id < alternating.parts.size(); ++id)
        {
            ast::BinaryOperator::Type type = ast::BinaryOperator::Type::And;
            auto lexeme = alternating.ops[id - 1];
            if (std::holds_alternative<lualex::And>(lexeme))
            {
                type = ast::BinaryOperator::Type::And;
            } else if (std::holds_alternative<lualex::Or>(lexeme))
            {
                type = ast::BinaryOperator::Type::Or;
            }

            ast::NodePtr ptr = std::move(alternating.parts.at(id));
            result = ast::NodePtr(ast::BinaryOperator(type, std::move(result), std::move(ptr)));
        }
        return result;
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
        ast::BinaryOperator result(ast::BinaryOperator::Type::CmpEq, std::move(left), std::move(right));
        return ast::NodePtr{std::move(result)};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpGe, ast::NodePtr& right)
    {
        ast::BinaryOperator result(ast::BinaryOperator::Type::CmpGe, std::move(left), std::move(right));
        return ast::NodePtr{std::move(result)};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpGt, ast::NodePtr& right)
    {
        ast::BinaryOperator result(ast::BinaryOperator::Type::CmpGt, std::move(left), std::move(right));
        return ast::NodePtr{std::move(result)};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpLe, ast::NodePtr& right)
    {
        ast::BinaryOperator result(ast::BinaryOperator::Type::CmpLe, std::move(left), std::move(right));
        return ast::NodePtr{std::move(result)};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpLt, ast::NodePtr& right)
    {
        ast::BinaryOperator result(ast::BinaryOperator::Type::CmpLt, std::move(left), std::move(right));
        return ast::NodePtr{std::move(result)};
    }

    static ast::NodePtr visit(ast::NodePtr& left, lualex::CmpNeq, ast::NodePtr& right)
    {
        ast::BinaryOperator result(ast::BinaryOperator::Type::CmpNeq, std::move(left), std::move(right));
        return ast::NodePtr{std::move(result)};
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

    parser::Sequence<parser::Repeating<ast::NodePtr, ExprUnary, lualex::Multiply, lualex::Divide, lualex::Mod>>
>
{
    static ast::NodePtr visit(parser::Alternating<ast::NodePtr, lualex::Multiply, lualex::Divide, lualex::Mod>& alternating)
    {
        ast::NodePtr result = std::move(alternating.parts.front());
        for (size_t id = 1; id < alternating.parts.size(); ++id)
        {
            ast::BinaryOperator::Type type = ast::BinaryOperator::Type::Multiply;
            auto lexeme = alternating.ops[id - 1];
            if (std::holds_alternative<lualex::Multiply>(lexeme))
            {
                type = ast::BinaryOperator::Type::Multiply;
            }
            else if (std::holds_alternative<lualex::Divide>(lexeme))
            {
                type = ast::BinaryOperator::Type::Divide;
            }
            else if (std::holds_alternative<lualex::Mod>(lexeme))
            {
                type = ast::BinaryOperator::Type::Mod;
            }

            ast::NodePtr ptr = std::move(alternating.parts.at(id));
            result = ast::NodePtr{ast::BinaryOperator(type, std::move(result), std::move(ptr))};
        }
        return result;
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
    // TODO: Add access by index (`[]` and `.`)
    // TODO: Add in-place dictionary parsing
    parser::Sequence<FunctionCall>,
    parser::Sequence<parser::Lex<lualex::Name>>,
    parser::Sequence<parser::Lex<lualex::Number>>,
    parser::Sequence<parser::Lex<lualex::String>>
>
{
    static ast::NodePtr visit(lualex::BracketRoundOp, ast::NodePtr& value, lualex::BracketRoundCl)
    {
        return std::move(value);
    }

    static ast::NodePtr visit(ast::FunctionCall& call)
    {
        return ast::NodePtr{std::move(call)};
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
};

}
