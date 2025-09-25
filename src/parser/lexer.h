#pragma once

#include <string>
#include <vector>

#include "meta/string.h"

namespace flua::parser
{
using namespace flua;

struct Lexeme
{
    // TODO: Add position info
};

template <meta::StringLiteral Value>
class DefiniteLexeme
{
public:
    static std::optional<DefiniteLexeme> tryConstruct(std::string_view& view)
    {
        if (view.size() < sizeof(Value))
        {
            return std::nullopt;
        }

        for (unsigned id = 0; id < sizeof(Value) - 1; id++)
        {
            if (Value.value[id] != view[id])
            {
                return std::nullopt;
            }
        }

        view.remove_prefix(sizeof(Value) - 1);

        return DefiniteLexeme{};
    }
};

template <class ...Lexemes>
struct Lexer
{
    using LexemeVariant = std::variant<Lexemes...>;

    static std::vector<LexemeVariant> lex(const std::string& text)
    {
        return lex(text, [&](const LexemeVariant& argument) { return true; });
    }

    template <class ...IgnoredLexemes>
    static std::vector<LexemeVariant> lexAllBut(const std::string& text)
    {
        return lex(text, [&](const LexemeVariant& argument)
        {
            return (... && !std::holds_alternative<IgnoredLexemes>(argument));
        });
    }

    template <class FilterT>
    static std::vector<LexemeVariant> lex(const std::string& text, FilterT&& filter)
    {
        std::vector<LexemeVariant> result;

        std::string_view view = text;
        while (!view.empty())
        {
            std::optional<LexemeVariant> lexeme = tryFetchLexeme<0>(view);
            if (!lexeme.has_value())
            {
                return {};
            }

            if (filter(*lexeme))
            {
                result.push_back(*lexeme);
            }
        }

        return result;
    }

private:
    template <unsigned Index>
    static std::optional<LexemeVariant> tryFetchLexeme(std::string_view& view)
    {
        std::string_view anchor = view;

        using LexemeTuple = std::tuple<Lexemes...>;

        if constexpr (Index < std::tuple_size_v<LexemeTuple>)
        {
            using LexemeType = std::tuple_element_t<Index, LexemeTuple>;
            std::optional<LexemeType> lexeme = LexemeType::tryConstruct(view);

            if (lexeme.has_value())
            {
                return *lexeme;
            }

            view = anchor;
            return tryFetchLexeme<Index + 1>(view);
        }

        return std::nullopt;
    }
};

}
