#pragma once

#include "parser/parser.h"
#include "parser/lexer.h"
#include "parser/lexer.h"

namespace flua::grammar
{
using namespace flua;

struct Expression;
struct MulFragment;
struct Constant;

using Add = parser::DefiniteLexeme<"+">;
using Subtract = parser::DefiniteLexeme<"-">;
using Multiply = parser::DefiniteLexeme<"*">;
using Divide = parser::DefiniteLexeme<"/">;
using BracketL = parser::DefiniteLexeme<"(">;
using BracketR = parser::DefiniteLexeme<")">;
using Whitespace = parser::DefiniteLexeme<" ">;

struct Number
{
    static std::optional<Number> tryConstruct(std::string_view& view)
    {
        if (view.empty())
        {
            return std::nullopt;
        }

        if (std::isdigit(view[0]))
        {
            char symbol = view[0];
            view.remove_prefix(1);
            return Number{.value = symbol - '0'};
        }

        return std::nullopt;
    }

    int value;
};

using ExpressionParser = parser::Lexer
<
    Add, Subtract,
    Multiply, Divide,
    BracketL, BracketR,
    Whitespace, Number
>;

struct Expression : parser::Grammar
<
    Expression, int,

    parser::Sequence<MulFragment, parser::Lex<Add>, Expression>,
    parser::Sequence<MulFragment, parser::Lex<Subtract>, Expression>,
    parser::Sequence<MulFragment>
>
{
    static int visit(int alpha, Add, int beta)
    {
        return alpha + beta;
    }

    static int visit(int alpha, Subtract, int beta)
    {
        return alpha - beta;
    }

    static int visit(int value)
    {
        return value;
    }
};

struct MulFragment : parser::Grammar
<
    MulFragment, int,

    parser::Sequence<Constant, parser::Lex<Multiply>, MulFragment>,
    parser::Sequence<Constant, parser::Lex<Divide>, MulFragment>,
    parser::Sequence<Constant>
>
{
    static int visit(int alpha, Multiply, int beta)
    {
        return alpha * beta;
    }

    static int visit(int alpha, Divide, int beta)
    {
        return alpha / beta;
    }

    static int visit(int value)
    {
        return value;
    }
};

struct Constant : parser::Grammar
<
    Constant, int,

    parser::Sequence<parser::Lex<Number>>,
    parser::Sequence<parser::Lex<BracketL>, Expression, parser::Lex<BracketR>>
>
{
    static int visit(Number number)
    {
        return number.value;
    }

    static int visit(BracketL, int val, BracketR)
    {
        return val;
    }
};

int solve_something();
}
