#pragma once

#include <optional>
#include <tuple>
#include <variant>
#include <expected>
#include <string>
#include <deque>

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

    template <class LexemeVariantPtr>
    static std::expected<ReturnType, ParsingError> tryParse(LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        std::expected<ReturnType, ParsingError> head = tryParseVariant<0, LexemeVariantPtr>(start, end);
        if (!head.has_value())
        {
            return head;
        }

        while (tryGrowHead<0, LexemeVariantPtr>(head.value(), start, end)) {}

        return head;
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

    template <unsigned VariantIndex, class LexemeVariantPtr>
    static bool tryGrowHead(ReturnType& head, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if (start == end)
        {
            return false;
        }

        if constexpr (VariantIndex < std::tuple_size_v<VariantTuple>)
        {
            LexemeVariantPtr formerStart = start;
            bool result =
                std::tuple_element_t<VariantIndex, VariantTuple>::template tryExtend<
                    ReturnType, ThisType, LexemeVariantPtr
                >(head, start, end);
            if (result)
            {
                return true;
            }

            start = formerStart;

            return tryGrowHead<VariantIndex + 1, LexemeVariantPtr>(head, start, end);
        }

        return false;
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

namespace
{
    template<class Grammar, class Tuple, std::size_t... I>
    auto call_with_tuple(Tuple& tuple, std::index_sequence<I...>) {
        return Grammar::visit(std::get<I>(tuple).value()...);
    }

    template<class Grammar, class Tuple>
    auto call_with_tuple(Tuple& tuple) {
        return call_with_tuple<Grammar>(tuple, std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
    }
}

template <class... Elements>
struct Sequence
{
    using SequenceTuple = std::tuple<Elements...>;

    template <class ReturnType, class Lexeme, class LexemeVariantPtr>
    static std::expected<ReturnType, ParsingError> tryParse(LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (std::is_same_v<get_first_type_t<SequenceTuple>, Lexeme>)
        {
            std::string what = "[INTERNAL PARSER ERROR] Parser fell into infinite recursion while processing rule ";
            what += typeid(Lexeme).name();
            return std::unexpected(ParsingError{.what = what, .where = posFromVariant(*start)});
        }
        else
        {
            using ArgumentTypeTuple = std::tuple<std::optional<typename Elements::RetType>...>;

            ArgumentTypeTuple args;
            std::optional<ParsingError> error = fillArgumentTuple<0, ArgumentTypeTuple, LexemeVariantPtr>(args, start, end);
            if (error)
            {
                return std::unexpected(*error);
            }

            return call_with_tuple<Lexeme, ArgumentTypeTuple>(args);
        }
    }

    template <class ReturnType, class Lexeme, class LexemeVariantPtr>
    static bool tryExtend(ReturnType& head, LexemeVariantPtr& start,
        const LexemeVariantPtr& end)
    {
        if constexpr (!std::is_same_v<get_first_type_t<SequenceTuple>, Lexeme>)
        {
            return false;
        }
        else
        {
            using ArgumentTypeTuple = std::tuple<std::optional<typename Elements::RetType>...>;

            ArgumentTypeTuple args;
            std::get<0>(args) = std::move(head);
            std::optional<ParsingError> error = fillArgumentTuple<1, ArgumentTypeTuple, LexemeVariantPtr>(args, start, end);
            if (error)
            {
                head = std::move(*std::get<0>(args));
                return false;
            }

            head = call_with_tuple<Lexeme, ArgumentTypeTuple>(args);
            return true;
        }
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
};

template <class Part, class ...OpTypes>
struct Alternating
{
    std::deque<Part> parts{};
    std::deque<std::variant<OpTypes...>> ops{};
};

template <class PartRt, class Part, class ...OpLexemes>
struct Repeating : Grammar
<
    Repeating<PartRt, Part, OpLexemes...>, Alternating<PartRt, OpLexemes...>,

    Sequence<Part, Lex<OpLexemes>, Repeating<PartRt, Part, OpLexemes...>>...,
    Sequence<Part>
>
{
    using AltSequence = Alternating<PartRt, OpLexemes...>;

    static AltSequence visit(PartRt& part)
    {
        return {.parts = {std::move(part)}, .ops = {}};
    }

    template <class OpLexeme>
    static AltSequence visit(PartRt& part, OpLexeme& op, AltSequence& sequence)
    {
        sequence.parts.emplace_front(std::move(part));
        sequence.ops.emplace_front(std::move(op));
        return std::move(sequence);
    }
};

template <class PartRt, class Part>
struct Repeating<PartRt, Part> : Grammar
<
    Repeating<PartRt, Part>, std::deque<PartRt>,

    Sequence<Part, Repeating<PartRt, Part>>,
    Sequence<>
>
{
    static std::deque<PartRt> visit()
    {
        return {};
    }

    static std::deque<PartRt> visit(PartRt& part, std::deque<PartRt>& sequence)
    {
        sequence.emplace_front(std::move(part));
        return std::move(sequence);
    }
};

}
