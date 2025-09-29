#pragma once

#include "lua_lexemes.h"
#include "ast/ast_nodes.h"

namespace flua::luagrmr
{
using namespace flua;

struct Program;

struct Function;
struct ParamNames;
struct Block;
struct Action;
struct Return;

struct FunctionCall;
struct Assignment;

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
        ParamNames,
        parser::Lex<lualex::BracketRoundCl>,
        Block,
        parser::Lex<lualex::End>
    >
>
{
    static ast::Function visit(lualex::Function, const lualex::Name& name, lualex::BracketRoundOp,
        std::deque<ids::ResolvableName>& params, lualex::BracketRoundCl, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Function function;
        function.body = std::move(body);
        function.name = name.name;
        function.parameters = std::move(params);
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

    parser::Sequence<parser::Lex<lualex::Name>, parser::Lex<lualex::BracketRoundOp>, parser::Lex<lualex::BracketRoundCl>>
    // TODO: Consider function arguments
>
{
    static ast::FunctionCall visit(const lualex::Name& name, lualex::BracketRoundOp, lualex::BracketRoundCl)
    {
        ast::FunctionCall function;
        function.name = ids::ResolvableName(name.name);
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

using Parser = parser::Parser<Program, lualex::LuaLexer, lualex::LuaIgnored>;

}
