#pragma once

#include "lua_lexemes.h"
#include "ast/ast_nodes.h"

namespace flua::luagrmr
{
using namespace flua;

struct Program;

struct Function;
struct Block;

struct Program : parser::Grammar
<
    Program, ast::NodePtr,

    parser::Sequence<parser::Lex<parser::Eof>>,
    parser::Sequence<Function, Program>
>
{
    static ast::NodePtr visit(parser::Eof)
    {
        return ast::NodePtr(ast::Program{});
    }

    static ast::NodePtr visit(ast::NodePtr& function, ast::NodePtr& program)
    {
        return std::move(program);
    }
};

struct Function : parser::Grammar
<
    Function, ast::NodePtr,

    parser::Sequence<parser::Lex<lualex::Number>>
>
{
    static ast::NodePtr visit(lualex::Number)
    {
        return ast::NodePtr(ast::Program{});
    }
};

using Parser = parser::Parser<Program, lualex::LuaLexer, lualex::LuaIgnored>;

}
