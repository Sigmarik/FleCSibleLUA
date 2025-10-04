#pragma once

#include "parser/complete.h"

namespace flua::lualex
{
using namespace flua;

using Semicolon = parser::DefiniteLexeme<";", ";">;

using Comma = parser::DefiniteLexeme<",", ",">;

using BracketRoundOp = parser::DefiniteLexeme<"(", "'('">;
using BracketRoundCl = parser::DefiniteLexeme<")", "')'">;
using BracketSquareOp = parser::DefiniteLexeme<"[", "'['">;
using BracketSquareCl = parser::DefiniteLexeme<"]", "']'">;
using BracketCurlyOp = parser::DefiniteLexeme<"{", "'{'">;
using BracketCurlyCl = parser::DefiniteLexeme<"}", "'}'">;

using CmpEq = parser::DefiniteLexeme<"==", "'=='">;
using CmpNeq = parser::DefiniteLexeme<"!=", "'!='">;  // TODO: Add ~=
using CmpGe = parser::DefiniteLexeme<">=", "'>='">;
using CmpLe = parser::DefiniteLexeme<"<=", "'<='">;
using CmpGt = parser::DefiniteLexeme<">", "'>'">;
using CmpLt = parser::DefiniteLexeme<"<", "'<'">;

using Assignment = parser::DefiniteLexeme<"=", "'='">;

using Plus = parser::DefiniteLexeme<"+", "'+'">;
using Minus = parser::DefiniteLexeme<"-", "'-'">;
using Multiply = parser::DefiniteLexeme<"*", "'*'">;
using Divide = parser::DefiniteLexeme<"/", "'/'">;
using Mod = parser::DefiniteLexeme<"%", "'%'">;
using Pow = parser::DefiniteLexeme<"^", "'^'">;
using Concat = parser::DefiniteLexeme<"..", "'..'">;
using Length = parser::DefiniteLexeme<"#", "'#'">;
using Not = parser::DefiniteLexeme<"!", "'!'">;  // TODO: Add unary ~

using And = parser::DefiniteLexeme<"and", "'and'">;
using Or = parser::DefiniteLexeme<"or", "'or'">;

using Dot = parser::DefiniteLexeme<".", "'.'">;
using Method = parser::DefiniteLexeme<":", "':'">;

using Local = parser::DefiniteLexeme<"local", "'local'">;

using If = parser::DefiniteLexeme<"if", "'if'">;
using ElseIf = parser::DefiniteLexeme<"elseif", "'elseif'">;
using Else = parser::DefiniteLexeme<"else", "'else'">;
using Then = parser::DefiniteLexeme<"then", "'then'">;
using While = parser::DefiniteLexeme<"while", "'while'">;
using Repeat = parser::DefiniteLexeme<"repeat", "'repeat'">;
using Until = parser::DefiniteLexeme<"until", "'until'">;
using For = parser::DefiniteLexeme<"for", "'for'">;
using In = parser::DefiniteLexeme<"in", "'in'">;
using Break = parser::DefiniteLexeme<"break", "'break'">;
using Continue = parser::DefiniteLexeme<"continue", "'continue'">;

using Do = parser::DefiniteLexeme<"do", "'do'">;
using End = parser::DefiniteLexeme<"end", "'end'">;

using Function = parser::DefiniteLexeme<"function", "function">;
using System = parser::DefiniteLexeme<"system", "system">;

using Query = parser::DefiniteLexeme<"query", "query">;

using Nil = parser::DefiniteLexeme<"nil", "'nil'">;
using True = parser::DefiniteLexeme<"true", "'true'">;
using False = parser::DefiniteLexeme<"false", "'false'">;
using Return = parser::DefiniteLexeme<"return", "return">;

struct Number : parser::Lexeme
{
    static constexpr meta::StringLiteral kName = "number";

    static std::optional<Number> tryConstruct(std::string_view& view);

    float value = 0.0;
};

struct Name : parser::Lexeme
{
    static constexpr meta::StringLiteral kName = "name";

    static std::optional<Name> tryConstruct(std::string_view& view);

    std::string name;
};

struct String : parser::Lexeme
{
    static constexpr meta::StringLiteral kName = "string";

    static std::optional<String> tryConstruct(std::string_view& view);

    std::string string;
};

struct SingleLineComment : parser::Lexeme
{
    static constexpr meta::StringLiteral kName = "single-line comment";

    static std::optional<SingleLineComment> tryConstruct(std::string_view& view);
};

struct MultiLineComment : parser::Lexeme
{
    static constexpr meta::StringLiteral kName = "multi-line comment";

    static std::optional<MultiLineComment> tryConstruct(std::string_view& view);
};

struct Whitespace : parser::Lexeme
{
    static constexpr meta::StringLiteral kName = "whitespace";

    static std::optional<Whitespace> tryConstruct(std::string_view& view);
};

using LuaLexer = parser::Lexer<
    SingleLineComment,
    Comma,
    MultiLineComment,
    Whitespace,
    Semicolon,
    BracketRoundOp,
    BracketRoundCl,
    BracketSquareOp,
    BracketSquareCl,
    BracketCurlyOp,
    BracketCurlyCl,
    CmpEq,
    CmpNeq,
    CmpGe,
    CmpLe,
    CmpGt,
    CmpLt,
    Assignment,
    Plus,
    Minus,
    Multiply,
    Divide,
    Mod,
    Pow,
    Length,
    Not,
    And,
    Or,
    Method,
    Local,
    If,
    ElseIf,
    Then,
    Else,
    While,
    Repeat,
    Until,
    For,
    In,
    Do,
    End,
    Break,
    Continue,
    Function,
    System,
    Query,
    Nil,
    True,
    False,
    Return,
    Number,
    Concat,
    Dot,
    Name,
    String
>;

using LuaIgnored = parser::IgnoreLexemes
<
    Semicolon,
    Whitespace,
    SingleLineComment,
    MultiLineComment
>;
}
