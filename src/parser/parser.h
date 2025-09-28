#pragma once

#include <optional>
#include <tuple>
#include <variant>
#include <expected>
#include <string>

#include "char_pos.h"
#include "error.h"

namespace flua::parser
{
using namespace flua;

namespace
{
    struct EmptyType {};

    template <class Tuple>
    struct get_first_type
    {
        using type = std::tuple_element_t<0, Tuple>;
    };

    template <>
    struct get_first_type<std::tuple<>>
    {
        using type = EmptyType;
    };

    template <class Tuple>
    using get_first_type_t = get_first_type<Tuple>::type;

    template <class Sequence, class... CallStack>
    class has_head_recursion
    {
        using CallTuple = std::tuple<CallStack...>;

        static constexpr bool hasImmediateRecursion =
            (... || std::is_same_v<get_first_type_t<typename Sequence::SequenceTuple>, CallStack>);

    public:
        static constexpr bool value = hasImmediateRecursion;
        // TODO: Recursive check for head recursion
    };

    template <class Sequence, class... CallStack>
    constexpr bool has_head_recursion_v = has_head_recursion<Sequence, CallStack...>::value;
}

template <class ThisType, class ReturnType, class... Variants>
struct Grammar
{
    using RetType = ReturnType;

    template <class ...TypeStack>
    static constexpr bool kRecursiveDescentParsable = (... && !has_head_recursion_v<Variants, ThisType, TypeStack...>);
    static_assert(kRecursiveDescentParsable<>);

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

        std::string what = std::string("Expected ") + Lexeme::kName.value + ", but found a ";
        std::visit([&](const auto& specificLexeme){ what += specificLexeme.kName.value; }, *start);
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
    static auto callWithTuple(Tuple& tuple, std::index_sequence<I...>) {
        return Grammar::visit(std::get<I>(tuple).value()...);
    }

    template<class Grammar, class Tuple>
    static auto callWithTuple(Tuple& tuple) {
        return callWithTuple<Grammar>(tuple, std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
    }
};

}
