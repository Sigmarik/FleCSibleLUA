#pragma once

#include <optional>
#include <tuple>
#include <variant>
#include <expected>
#include <optional>
#include <string>
#include <deque>

#include "meta/tuple_helper.h"

#include "char_pos.h"
#include "error.h"

#ifdef DEBUG_FLUA_PARSER
#include <iostream>
#define __FLUA_PARSER_DEBUG_PRINT(sequence) std::cerr << sequence << std::endl
#else
#define __FLUA_PARSER_DEBUG_PRINT(sequence)
#endif

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

template<class RootGrammar>
struct Parser : IParser
{
    template<class LexemeVariantPtr>
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

    template<class Tuple>
    struct get_first_type
    {
        using type = std::tuple_element_t<0, Tuple>;
    };

    template<>
    struct get_first_type<std::tuple<> >
    {
        using type = EmptyType;
    };

    template<class Tuple>
    using get_first_type_t = get_first_type<Tuple>::type;
}

template<class KeyT, class ForestT>
struct Tree
{
    using Key = KeyT;
    using Forest = ForestT;
};

namespace
{
    template<class Grammar, class Tuple, std::size_t... I>
    auto call_with_tuple(Tuple& tuple, std::index_sequence<I...>)
    {
        return Grammar::visit(std::get<I>(tuple)...);
    }

    template<class Grammar, class Tuple>
    auto call_with_tuple(Tuple& tuple)
    {
        return call_with_tuple<Grammar>(tuple, std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple> > >{});
    }
}

template<bool HasEmptyTreeV, class... TreesT>
struct Forest
{
    static constexpr bool kHasEmptyTree = HasEmptyTreeV;

    using TreesTuple = std::tuple<TreesT...>;

    template<class RetType, class CurrentGrmr, class ParserT, class LexemeVariantPtr, unsigned treeN>
    static bool tryExtend(RetType& current, ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if constexpr (treeN >= std::tuple_size_v<TreesTuple>)
        {
            return false;
        }
        else
        {
            using CurrentTree = std::tuple_element_t<treeN, TreesTuple>;

            if constexpr (!std::is_same_v<CurrentGrmr, typename CurrentTree::Key>)
            {
                return tryExtend
                        <RetType, CurrentGrmr, ParserT, LexemeVariantPtr, treeN + 1>
                        (current, parser, start, end);
            }
            else
            {
                LexemeVariantPtr formerStart = start;
                std::tuple<RetType> headTuple = std::make_tuple(current);
                auto stageResult = CurrentTree::Forest::template tryParse
                        <RetType, CurrentGrmr, true, ParserT, std::tuple<RetType>, LexemeVariantPtr, 0>
                        (parser, start, end, headTuple);
                if (!stageResult)
                {
                    start = formerStart;
                    return tryExtend
                        <RetType, CurrentGrmr, ParserT, LexemeVariantPtr, treeN + 1>
                        (current, parser, start, end);
                }

                current = std::move(*stageResult);

                return true;
            }
        }
    }

    template<class RetType, class CurrentGrmr, bool ignoreCurrentGrmrKeysV, class ParserT, class PreviousIterationsTuple, class LexemeVariantPtr, unsigned treeN>
    static std::optional<RetType> tryParse(ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end,
                                           PreviousIterationsTuple& args)
    {
        if constexpr (treeN >= std::tuple_size_v<TreesTuple>)
        {
            if constexpr (kHasEmptyTree) return call_with_tuple<CurrentGrmr, PreviousIterationsTuple>(args);
            else return std::nullopt;
        }
        else
        {
            using CurrentTree = std::tuple_element_t<treeN, TreesTuple>;

            if constexpr (ignoreCurrentGrmrKeysV && std::is_same_v<CurrentGrmr, typename CurrentTree::Key>)
            {
                return tryParse
                    <RetType, CurrentGrmr, true, ParserT, PreviousIterationsTuple, LexemeVariantPtr, treeN + 1>
                    (parser, start, end, args);
            }
            else
            {
                LexemeVariantPtr formerStart = start;
                auto stageResult = CurrentTree::Key::tryParse(parser, start, end);
                if (!stageResult)
                {
                    start = formerStart;
                    return tryParse
                        <RetType, CurrentGrmr, true, ParserT, PreviousIterationsTuple, LexemeVariantPtr, treeN + 1>
                        (parser, start, end, args);
                }
                auto expanded = meta::expand(args, std::move(*stageResult));

                auto deepResult = CurrentTree::Forest::template tryParse
                        <RetType, CurrentGrmr, false, ParserT, decltype(expanded), LexemeVariantPtr, 0>
                        (parser, start, end, expanded);
                if (deepResult)
                    return deepResult;
                start = formerStart;
                return tryParse
                        <RetType, CurrentGrmr, true, ParserT, PreviousIterationsTuple, LexemeVariantPtr, treeN + 1>
                        (parser, start, end, args);
            }
        }
    }
};

template<class...>
struct Sequence {};

#include "parser_borificator.h"

template<class ThisType, class ReturnType, class... Variants>
struct Grammar
{
    using RetType = ReturnType;
    using BorForest = make_forest_t<Variants...>;

    template<class ParserT, class LexemeVariantPtr>
    static std::optional<RetType> tryParse(ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        CharacterPos startingPos{};
        std::visit([&](const auto& lex) { startingPos = lex.startingPos; }, *start);
        __FLUA_PARSER_DEBUG_PRINT("Entered " << typeid(ThisType).name() << " at " <<
            startingPos.line << ":" << startingPos.column);
        std::tuple<> emptyTuple;
        std::optional<ReturnType> head = BorForest::template tryParse
            <RetType, ThisType, true, ParserT, std::tuple<>, LexemeVariantPtr, 0>
            (parser, start, end, emptyTuple);

        if (!head.has_value())
        {
            __FLUA_PARSER_DEBUG_PRINT(typeid(ThisType).name() << " failed");
            return head;
        }
        __FLUA_PARSER_DEBUG_PRINT(typeid(ThisType).name() << " succeeded, trying to extend...");

        while (BorForest::template tryExtend
            <RetType, ThisType, ParserT, LexemeVariantPtr, 0>
            (*head, parser, start, end)) {}

        __FLUA_PARSER_DEBUG_PRINT("Exiting " << typeid(ThisType).name());

        return head;
    }
};

template<class Lexeme>
struct Lex final
{
    using RetType = Lexeme;

    template<class ParserT, class LexemeVariantPtr>
    static std::optional<Lexeme> tryParse(ParserT& parser, LexemeVariantPtr& start, const LexemeVariantPtr& end)
    {
        if (start != end && std::holds_alternative<Lexeme>(*start))
        {
            Lexeme result = std::get<Lexeme>(*start);
            __FLUA_PARSER_DEBUG_PRINT(typeid(Lexeme).name() << " at " <<
                result.startingPos.line << ":" << result.startingPos.column << " confirmed");
            ++start;
            return result;
        }

        __FLUA_PARSER_DEBUG_PRINT(typeid(Lexeme).name() << " mismatch");

        std::string what = std::string(Lexeme::kName.value) + " expected near ";
        std::visit([&](const auto& specificLexeme) { what += specificLexeme.kName.value; }, *start);
        parser.trySetError(ParsingError{.what = what, .where = posFromVariant(*start)});
        return std::nullopt;
    }
};

template<class Part, class... OpTypes>
struct Alternating
{
    std::deque<Part> parts{};
    std::deque<std::variant<OpTypes...> > ops{};
};

template<class PartRt, class Part, class... OpLexemes>
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

    template<class OpLexeme>
    static std::deque<PartRt> visit(std::deque<PartRt>& sequence, OpLexeme&&, PartRt& part)
    {
        sequence.emplace_back(std::move(part));
        return std::move(sequence);
    }
};

template<class PartRt, class Part>
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
