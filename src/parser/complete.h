#pragma once

#include "lexer.h"
#include "parser.h"

namespace flua::parser
{
using namespace flua;

template <class ...Lexemes>
struct IgnoreLexemes
{
    template <class LexerT>
    static auto lex(const std::string& text)
    {
        return LexerT::template lexAllBut<Lexemes...>(text);
    }
};

template <class MainGrammar, class LexerT, class IgnoredLexemes>
struct CompleteParser
{
    using ParsingResult = std::expected<typename MainGrammar::RetType, ParsingError>;

    using ParserT = Parser<MainGrammar>;

    static ParsingResult parse(const std::string& text)
    {
        auto lexemes = IgnoredLexemes::template lex<LexerT>(text);

        if (!lexemes.has_value())
        {
            return std::unexpected(lexemes.error());
        }

        auto bgn = lexemes.value().begin();
        auto end = lexemes.value().end();
        return ParserT::tryParse(bgn, end);
    }
};
}
