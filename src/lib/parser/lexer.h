#pragma once

#include <string>
#include <vector>
#include <expected>

#include "char_pos.h"
#include "meta/string.h"

#include "error.h"

namespace flua::parser
{
using namespace flua;

struct Lexeme
{
    CharacterPos startingPos{};
};

template<meta::StringLiteral Value, meta::StringLiteral Name>
class DefiniteLexeme : public Lexeme
{
public:
    static constexpr meta::StringLiteral kName = Name;

    static std::optional<DefiniteLexeme> tryConstruct(std::string_view& view)
    {
        if (view.size() + 1 < sizeof(Value))
        {
            return std::nullopt;
        }

        bool canBeInName = true;

        for (unsigned id = 0; id < sizeof(Value) - 1; id++)
        {
            if (!std::isalnum(view[id]) && view[id] != '_')
            {
                canBeInName = false;
            }
            if (Value.value[id] != view[id])
            {
                return std::nullopt;
            }
        }

        view.remove_prefix(sizeof(Value) - 1);

        if (canBeInName && !view.empty() && (std::isalnum(view.front()) || view.front() == '_'))
        {
            return std::nullopt;
        }

        return DefiniteLexeme{};
    }
};

struct Eof : Lexeme
{
    static constexpr meta::StringLiteral kName = "EOF";

    static std::optional<Eof> tryConstruct(std::string_view& view)
    {
        return std::nullopt;
    }
};

template<class... Lexemes>
struct Lexer
{
    using LexemeVariant = std::variant<Lexemes..., Eof>;

    static std::expected<std::vector<LexemeVariant>, ParsingError> lex(const std::string& text)
    {
        return lex(text, [&](const LexemeVariant& argument) { return true; });
    }

    template<class... IgnoredLexemes>
    static std::expected<std::vector<LexemeVariant>, ParsingError> lexAllBut(const std::string& text)
    {
        return lex(text, [&](const LexemeVariant& argument)
        {
            return (... && !std::holds_alternative<IgnoredLexemes>(argument));
        });
    }

    template<class FilterT>
    static std::expected<std::vector<LexemeVariant>, ParsingError> lex(const std::string& text, FilterT&& filter)
    {
        std::vector<LexemeVariant> result;

        CharacterPos pos;

        std::string_view view = text;
        while (!view.empty())
        {
            std::string_view start = view;
            std::optional<LexemeVariant> lexeme = tryFetchLexeme<0>(view);

            if (!lexeme.has_value())
            {
                return std::unexpected(ParsingError
                    {
                        .what = "Unexpected input that doesn't match any valid token pattern",
                        .where = pos
                    });
            }

            std::visit([&](auto& specificLexeme)
            {
                specificLexeme.startingPos = pos;
            }, *lexeme);
            while (start.size() > view.size())
            {
                pos.considerChar(start[0]);
                start.remove_prefix(1);
            }

            if (filter(*lexeme))
            {
                result.push_back(*lexeme);
            }
        }

        Eof eof;
        eof.startingPos = pos;
        result.push_back(eof);

        return result;
    }

private:
    template<unsigned Index>
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
