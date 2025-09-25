#pragma once

#include "parser/parsing_engine.h"
#include "parser/lexer.h"

namespace flua::grammar
{
using namespace flua;

struct Expression;
struct MulFragment;
struct Constant;

struct Add : parser::Lexeme
{};

struct Subtract : parser::Lexeme
{};

struct Multiply : parser::Lexeme
{};

struct Divide : parser::Lexeme
{};

struct BracketL : parser::Lexeme
{};

struct BracketR : parser::Lexeme
{};

struct Number : parser::Lexeme
{
    int value;
};

using LexemeVariant = std::variant<Add, Subtract, Multiply, Divide, BracketL, BracketR, Number>;

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
