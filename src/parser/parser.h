#pragma once

#include <optional>
#include <tuple>
#include <variant>
#include <expected>
#include <optional>
#include <string>
#include <deque>

#include "char_pos.h"
#include "error.h"

namespace flua::parser
{
using namespace flua;

namespace
{
    struct IParser
    {
        ParsingError error{.what = "No error", .where = CharacterPos{.line = 0, .column = 0}};

        void trySetError(const ParsingError& newError)
        {
            if (error.where < newError.where)
                error = newError;
        }
    };
}

template <class RootGrammar>
struct Parser : IParser
{
    template <class LexemeVariantPtr>
    static std::expected<typename RootGrammar::RetType, ParsingError> tryParse(
        LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        Parser parser;

        std::optional<typename RootGrammar::RetType> result = RootGrammar::tryParse(parser, start, end);
        if (!result.has_value())
            return std::unexpected(std::move(parser.error));
        return *result;
    }

private:
    Parser() = default;
};

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
}

template <class ThisType, class ReturnType, class... Variants>
struct Grammar
{
    using RetType = ReturnType;

    template <class ParserT, class LexemeVariantPtr>
    static std::optional<RetType> tryParse(ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        std::optional<ReturnType> head = tryParseVariant<0, ParserT, LexemeVariantPtr>(parser, start, end);
        if (!head.has_value())
        {
            return head;
        }

        while (tryGrowHead<0, ParserT, LexemeVariantPtr>(head.value(), parser, start, end)) {}

        return head;
    }

private:
    using VariantTuple = std::tuple<Variants...>;

    template <unsigned VariantIndex, class ParserT, class LexemeVariantPtr>
    static std::optional<ReturnType> tryParseVariant(ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (VariantIndex < std::tuple_size_v<VariantTuple>)
        {
            LexemeVariantPtr formerStart = start;
            std::optional<ReturnType> result =
                std::tuple_element_t<VariantIndex, VariantTuple>::template tryParse<
                    ReturnType, ThisType, ParserT, LexemeVariantPtr
                >(parser, start, end);
            if (result.has_value())
            {
                return result;
            }

            start = formerStart;

            if constexpr (VariantIndex + 1 < std::tuple_size_v<VariantTuple>)
            {
                return tryParseVariant<VariantIndex + 1, ParserT, LexemeVariantPtr>(parser, start, end);
            }

            return result;
        }

        auto what = "All options failed";
        parser.trySetError(ParsingError{.what = what, .where = posFromVariant(*start)});
        return std::nullopt;
    }

    template <unsigned VariantIndex, class ParserT, class LexemeVariantPtr>
    static bool tryGrowHead(ReturnType& head, ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
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
                    ReturnType, ThisType, ParserT, LexemeVariantPtr
                >(head, parser, start, end);
            if (result)
            {
                return true;
            }

            start = formerStart;

            return tryGrowHead<VariantIndex + 1, ParserT, LexemeVariantPtr>(head, parser, start, end);
        }

        return false;
    }
};

template <class Lexeme>
struct Lex final
{
    using RetType = Lexeme;

    template <class ParserT, class LexemeVariantPtr>
    static std::optional<Lexeme> tryParse(ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if (start != end && std::holds_alternative<Lexeme>(*start))
        {
            Lexeme result = std::get<Lexeme>(*start);
            ++start;
            return result;
        }

        std::string what = std::string("Expected ") + Lexeme::kName.value + ", but found a ";
        std::visit([&](const auto& specificLexeme){ what += specificLexeme.kName.value; }, *start);
        parser.trySetError(ParsingError{.what = what, .where = posFromVariant(*start)});
        return std::nullopt;
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

    template <class ReturnType, class Lexeme, class ParserT, class LexemeVariantPtr>
    static std::optional<ReturnType> tryParse(ParserT& parser,
        LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (std::is_same_v<get_first_type_t<SequenceTuple>, Lexeme>)
        {
            return std::nullopt;
        }
        else
        {
            using ArgumentTypeTuple = std::tuple<std::optional<typename Elements::RetType>...>;

            ArgumentTypeTuple args;
            bool success = fillArgumentTuple<0, ArgumentTypeTuple, ParserT, LexemeVariantPtr>(args, parser, start, end);
            if (!success)
            {
                return std::nullopt;
            }

            return call_with_tuple<Lexeme, ArgumentTypeTuple>(args);
        }
    }

    template <class ReturnType, class Lexeme, class ParserT, class LexemeVariantPtr>
    static bool tryExtend(ReturnType& head, ParserT& parser, LexemeVariantPtr& start,
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
            bool success =
                fillArgumentTuple<1, ArgumentTypeTuple, ParserT, LexemeVariantPtr>(args,parser, start, end);
            if (!success)
            {
                head = std::move(*std::get<0>(args));
                return false;
            }

            head = call_with_tuple<Lexeme, ArgumentTypeTuple>(args);
            return true;
        }
    }

private:
    template <unsigned Index, class Tuple, class ParserT, class LexemeVariantPtr>
    static bool fillArgumentTuple(Tuple& tuple, ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (Index < std::tuple_size_v<SequenceTuple>)
        {
            using CurrentGrammar = std::tuple_element_t<Index, SequenceTuple>;
            auto result = CurrentGrammar::tryParse(parser, start, end);
            if (!result.has_value())
            {
                return false;
            }

            std::get<Index>(tuple) = result.value();

            return fillArgumentTuple<Index + 1, Tuple, ParserT, LexemeVariantPtr>(tuple, parser, start, end);
        }

        return true;
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
    Repeating<PartRt, Part, OpLexemes...>, std::deque<PartRt>,

    Sequence<Repeating<PartRt, Part, OpLexemes...>, Lex<OpLexemes>, Part>...,
    Sequence<Part>
>
{
    static std::deque<PartRt> visit(PartRt& part)
    {
        return {std::move(part)};
    }

    template <class OpLexeme>
    static std::deque<PartRt> visit(std::deque<PartRt>& sequence, OpLexeme&&, PartRt& part)
    {
        sequence.emplace_back(std::move(part));
        return std::move(sequence);
    }
};

template <class PartRt, class Part>
struct Repeating<PartRt, Part> : Grammar
<
    Repeating<PartRt, Part>, std::deque<PartRt>,

    Sequence<Repeating<PartRt, Part>, Part>,
    Sequence<>
>
{
    static std::deque<PartRt> visit()
    {
        return {};
    }

    static std::deque<PartRt> visit(std::deque<PartRt>& sequence, PartRt& part)
    {
        sequence.emplace_back(std::move(part));
        return std::move(sequence);
    }
};

}
