#pragma once

#include <optional>
#include <tuple>
#include <variant>
#include <expected>
#include <string>

#include "char_pos.h"

namespace flua::parser
{
using namespace flua;

struct ParsingError
{
    std::string what{};
    CharacterPos where{};
};

template <class ThisType, class ReturnType, class... Variants>
struct Grammar
{
    using RetType = ReturnType;

    template <class LexemeVariantPtr>
    static std::expected<ReturnType, ParsingError> tryParse(LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        return tryParseVariant<0, LexemeVariantPtr>(start, end);
    }

private:
    using VariantTuple = std::tuple<Variants...>;

    template <unsigned VariantIndex, class LexemeVariantPtr>
    static std::expected<ReturnType, ParsingError> tryParseVariant(LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (VariantIndex < std::tuple_size_v<VariantTuple>)
        {
            LexemeVariantPtr formerStart = start;
            std::expected<ReturnType, ParsingError> result =
                std::tuple_element_t<VariantIndex, VariantTuple>::template tryParse<
                    ReturnType, ThisType, LexemeVariantPtr
                >(start, end);
            if (result.has_value())
            {
                return result;
            }

            start = formerStart;

            if constexpr (VariantIndex + 1 < std::tuple_size_v<VariantTuple>)
                return tryParseVariant<VariantIndex + 1, LexemeVariantPtr>(start, end);

            return result;
        }

        auto what = "All options failed";
        return std::unexpected(ParsingError{.what = what, .where = posFromVariant(*start)});
    }
};

template <class Lexeme>
struct Lex final
{
    using RetType = Lexeme;

    template <class LexemeVariantPtr>
    static std::expected<Lexeme, ParsingError> tryParse(LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if (start != end && std::holds_alternative<Lexeme>(*start))
        {
            Lexeme result = std::get<Lexeme>(*start);
            ++start;
            return result;
        }

        std::string what = std::string("Expected ") + Lexeme::kName.value + ", got ";
        std::visit([&](const auto& specificLexeme){ what += specificLexeme.kName.value; }, *start);
        what += " instead";
        return std::unexpected(ParsingError{.what = what, .where = posFromVariant(*start)});
    }
};

template <class... Elements>
struct Sequence
{
    using SequenceTuple = std::tuple<Elements...>;

    template <class ReturnType, class Lexeme, class LexemeVariantPtr>
    static std::expected<ReturnType, ParsingError> tryParse(LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        using ArgumentTypeTuple = std::tuple<std::optional<typename Elements::RetType>...>;

        ArgumentTypeTuple args;
        std::optional<ParsingError> error = fillArgumentTuple<0, ArgumentTypeTuple, LexemeVariantPtr>(args, start, end);
        if (error)
        {
            return std::unexpected(*error);
        }

        return callWithTuple<Lexeme, ArgumentTypeTuple>(args);
    }

private:
    template <unsigned Index, class Tuple, class LexemeVariantPtr>
    static std::optional<ParsingError> fillArgumentTuple(Tuple& tuple, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (Index < std::tuple_size_v<SequenceTuple>)
        {
            using CurrentGrammar = std::tuple_element_t<Index, SequenceTuple>;
            auto result = CurrentGrammar::tryParse(start, end);
            if (!result.has_value())
            {
                return result.error();
            }

            std::get<Index>(tuple) = result.value();

            return fillArgumentTuple<Index + 1, Tuple, LexemeVariantPtr>(tuple, start, end);
        }

        return std::nullopt;
    }

    template<class Grammar, class Tuple, std::size_t... I>
    static auto callWithTuple(const Tuple& tuple, std::index_sequence<I...>) {
        return Grammar::visit(std::get<I>(tuple).value()...);
    }

    template<class Grammar, class Tuple>
    static auto callWithTuple(const Tuple& tuple) {
        return callWithTuple<Grammar>(tuple, std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
    }
};

}
