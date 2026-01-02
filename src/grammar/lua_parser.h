#pragma once

#include <functional>

#include "lua_lexemes.h"
#include "ast/ast_nodes.h"

namespace flua::luagrmr
{
using namespace flua;

struct Program;

using Parser = parser::CompleteParser<Program, lualex::LuaLexer, lualex::LuaIgnored>;

struct Function;
struct ParamNames;
struct Block;
struct Action;
struct Return;
struct Break;
struct Continue;

struct Assignment;
struct LocalAssignment;

struct HighLevelExpression;
struct ExprComparison;
struct ExprComputation;
struct ExprMultiplication;
struct ExprUnary;
struct ExprSingleton;

struct InlineFunction;

struct RValueExpression;

struct Branch;
struct WhileLoop;
struct RepeatUntil;
struct NumericFor;
struct GenericFor;

struct ProgramBody;
struct Program : parser::Grammar
<
    Program, ast::Program,

    parser::Sequence<ProgramBody, parser::Lex<parser::Eof>>
>
{
    static ast::Program visit(ast::Program& program, parser::Eof)
    {
        return std::move(program);
    }
};

struct ProgramBody : parser::Grammar
<
    ProgramBody, ast::Program,

    parser::Sequence<>,
    parser::Sequence<ProgramBody, Action>
>
{
    static ast::Program visit()
    {
        return ast::Program(parser::CharacterPos{1, 1});
    }

    static ast::Program visit(ast::Program& program, ast::NodePtr& node)
    {
        program.components.emplace_back(std::move(node));
        return std::move(program);
    }
};

struct Function : parser::Grammar
<
    Function, ast::Assignment,

    parser::Sequence<
        parser::Lex<lualex::Function>,
        parser::Lex<lualex::Name>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<lualex::Name, parser::Lex<lualex::Name>, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>,
        Block,
        parser::Lex<lualex::End>
    >,

    parser::Sequence<
        parser::Lex<lualex::Function>,
        parser::Lex<lualex::Name>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Lex<lualex::BracketRoundCl>,
        Block,
        parser::Lex<lualex::End>
    >
>
{
    static ast::Assignment visit(lualex::Function fnc, const lualex::Name& name, lualex::BracketRoundOp,
        std::deque<lualex::Name>& params, lualex::BracketRoundCl, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Function function(fnc.startingPos);
        function.body = std::move(body);
        ast::Variable assignee(name.startingPos, name.name);
        for (lualex::Name& paramName : params)
        {
            function.parameters.emplace_back(paramName.name);
        }
        ast::Assignment assignment(fnc.startingPos);
        assignment.subjects.emplace_back(std::move(assignee));
        assignment.data.emplace_back(std::move(function));
        return assignment;
    }

    static ast::Assignment visit(lualex::Function fnc, const lualex::Name& name, lualex::BracketRoundOp,
        lualex::BracketRoundCl, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Function function(fnc.startingPos);
        function.body = std::move(body);
        ast::Variable assignee(name.startingPos, name.name);
        ast::Assignment assignment(fnc.startingPos);
        assignment.subjects.emplace_back(std::move(assignee));
        assignment.data.emplace_back(std::move(function));
        return assignment;
    }
};

struct InlineFunction : parser::Grammar
<
    InlineFunction, ast::Function,

    parser::Sequence<
        parser::Lex<lualex::Function>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<lualex::Name, parser::Lex<lualex::Name>, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>,
        Block,
        parser::Lex<lualex::End>
    >,
    parser::Sequence<
        parser::Lex<lualex::Function>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Lex<lualex::BracketRoundCl>,
        Block,
        parser::Lex<lualex::End>
    >
>
{
    static ast::Function visit(lualex::Function fnc, lualex::BracketRoundOp,
        std::deque<lualex::Name>& params, lualex::BracketRoundCl, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Function function(fnc.startingPos);
        function.body = std::move(body);
        for (lualex::Name& paramName : params)
        {
            function.parameters.emplace_back(paramName.name);
        }
        return function;
    }

    static ast::Function visit(lualex::Function fnc, lualex::BracketRoundOp,
        lualex::BracketRoundCl, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::Function function(fnc.startingPos);
        function.body = std::move(body);
        return function;
    }
};

struct ParamNames : parser::Grammar
<
    ParamNames, std::deque<ids::ResolvableName>,

    parser::Sequence<parser::Lex<lualex::Name>, parser::Lex<lualex::Comma>, ParamNames>,
    parser::Sequence<parser::Lex<lualex::Name>>,
    parser::Sequence<>
>
{
    static std::deque<ids::ResolvableName> visit()
    {
        return {};
    }

    static std::deque<ids::ResolvableName> visit(const lualex::Name& name)
    {
        return {ids::ResolvableName(name.name)};
    }

    static std::deque<ids::ResolvableName> visit(const lualex::Name& name, lualex::Comma,
        std::deque<ids::ResolvableName>& params)
    {
        std::deque<ids::ResolvableName> newParams = std::move(params);
        newParams.emplace_front(name.name);
        return newParams;
    }
};

struct BlockBody;
struct BlockTerminator;
struct Block : parser::Grammar
<
    Block, std::deque<ast::NodePtr>,

    parser::Sequence<BlockBody, Return>,
    parser::Sequence<BlockBody>
>
{
    static std::deque<ast::NodePtr> visit(std::deque<ast::NodePtr>& body, ast::Return& terminator)
    {
        body.emplace_back(std::move(terminator));
        return std::move(body);
    }

    static std::deque<ast::NodePtr> visit(std::deque<ast::NodePtr>& body)
    {
        return std::move(body);
    }
};

struct BlockBody : parser::Grammar
<
    BlockBody, std::deque<ast::NodePtr>,

    parser::Sequence<BlockBody, Action>,
    parser::Sequence<>
>
{
    static std::deque<ast::NodePtr> visit(std::deque<ast::NodePtr>& body, ast::NodePtr& action)
    {
        std::deque<ast::NodePtr> newBody = std::move(body);
        newBody.emplace_back(action);
        return newBody;
    }

    static std::deque<ast::NodePtr> visit()
    {
        return {};
    }
};

struct Action : parser::Grammar
<
    Action, ast::NodePtr,

    parser::Sequence<Assignment>,
    parser::Sequence<ExprUnary>,
    parser::Sequence<Branch>,
    parser::Sequence<WhileLoop>,
    parser::Sequence<RepeatUntil>,
    parser::Sequence<NumericFor>,
    parser::Sequence<GenericFor>,
    parser::Sequence<Function>,
    parser::Sequence<LocalAssignment>,
    parser::Sequence<Break>,
    parser::Sequence<Continue>
>
{
    template <class T>
    static ast::NodePtr visit(T& exactNode)
    {
        return ast::NodePtr(std::move(exactNode));
    }
};

struct Return : parser::Grammar
<
    Return, ast::Return,

    parser::Sequence<parser::Lex<lualex::Return>, parser::Repeating<ast::NodePtr, HighLevelExpression, lualex::Comma>>,
    parser::Sequence<parser::Lex<lualex::Return>>
>
{
    static ast::Return visit(lualex::Return rt)
    {
        return ast::Return(rt.startingPos);
    }

    static ast::Return visit(lualex::Return rt, std::deque<ast::NodePtr>& values)
    {
        ast::Return ret(rt.startingPos);
        ret.values = std::move(values);
        return {std::move(ret)};
    }
};

struct ExprLogical;

template <class Lex>
constexpr ast::BinaryOperator::Type lex2bin_v = ast::BinaryOperator::Type::Add;

template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::CmpEq> = ast::BinaryOperator::Type::CmpEq;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::CmpNeq> = ast::BinaryOperator::Type::CmpNeq;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::CmpGe> = ast::BinaryOperator::Type::CmpGe;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::CmpLe> = ast::BinaryOperator::Type::CmpLe;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::CmpGt> = ast::BinaryOperator::Type::CmpGt;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::CmpLt> = ast::BinaryOperator::Type::CmpLt;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Plus> = ast::BinaryOperator::Type::Add;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Minus> = ast::BinaryOperator::Type::Subtract;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Multiply> = ast::BinaryOperator::Type::Multiply;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Divide> = ast::BinaryOperator::Type::Divide;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Mod> = ast::BinaryOperator::Type::Mod;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Pow> = ast::BinaryOperator::Type::Pow;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::And> = ast::BinaryOperator::Type::And;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Or> = ast::BinaryOperator::Type::Or;
template <> constexpr ast::BinaryOperator::Type lex2bin_v<lualex::Concat> = ast::BinaryOperator::Type::Concatenate;

template <class Lex>
constexpr ast::UnaryOperator::Type lex2un_v = ast::UnaryOperator::Type::Length;

template <> constexpr ast::UnaryOperator::Type lex2un_v<lualex::Length> = ast::UnaryOperator::Type::Length;
template <> constexpr ast::UnaryOperator::Type lex2un_v<lualex::Not> = ast::UnaryOperator::Type::Not;
template <> constexpr ast::UnaryOperator::Type lex2un_v<lualex::Minus> = ast::UnaryOperator::Type::Negate;

template <class ...Descriptors>
struct OperatorLike : parser::Grammar<Descriptors...>
{
    static ast::NodePtr visit(ast::NodePtr& node)
    {
        return std::move(node);
    }

    template <class Lex>
    static ast::NodePtr visit(ast::NodePtr& left, const Lex lex, ast::NodePtr& right)
    {
        ast::BinaryOperator op(lex.startingPos, lex2bin_v<Lex>,
            std::move(left), std::move(right));
        return ast::NodePtr{std::move(op)};
    }

    template <class Lex>
    static ast::NodePtr visit(const Lex lex, ast::NodePtr& value)
    {
        ast::UnaryOperator op(lex.startingPos, lex2un_v<Lex>, std::move(value));
        return ast::NodePtr{std::move(op)};
    }
};

struct HighLevelExpression : OperatorLike
<
    HighLevelExpression, ast::NodePtr,

    parser::Sequence<ExprLogical>,
    parser::Sequence<HighLevelExpression, parser::Lex<lualex::Concat>, ExprLogical>
>
{};

struct ExprLogical : OperatorLike
<
    ExprLogical, ast::NodePtr,

    parser::Sequence<ExprComparison>,
    parser::Sequence<ExprLogical, parser::Lex<lualex::And>, ExprComparison>,
    parser::Sequence<ExprLogical, parser::Lex<lualex::Or>, ExprComparison>
>
{};

struct ExprComparison : OperatorLike
<
    ExprComparison, ast::NodePtr,

    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpEq>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpGe>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpGt>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpLe>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpLt>, ExprComputation>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::CmpNeq>, ExprComputation>,
    parser::Sequence<ExprComputation>
>
{};

struct ExprComputation : OperatorLike
<
    ExprComputation, ast::NodePtr,

    parser::Sequence<ExprComputation, parser::Lex<lualex::Plus>, ExprMultiplication>,
    parser::Sequence<ExprComputation, parser::Lex<lualex::Minus>, ExprMultiplication>,
    parser::Sequence<ExprMultiplication>
>
{};

struct ExprPower;

struct ExprMultiplication : OperatorLike
<
    ExprMultiplication, ast::NodePtr,

    parser::Sequence<ExprPower>,
    parser::Sequence<ExprMultiplication, parser::Lex<lualex::Multiply>, ExprPower>,
    parser::Sequence<ExprMultiplication, parser::Lex<lualex::Divide>, ExprPower>,
    parser::Sequence<ExprMultiplication, parser::Lex<lualex::Mod>, ExprPower>
>
{};

struct ExprPower : OperatorLike
<
    ExprPower, ast::NodePtr,

    parser::Sequence<ExprUnary, parser::Lex<lualex::Pow>, ExprPower>,
    parser::Sequence<ExprUnary>
>
{};

struct ExprUnary : OperatorLike
<
    ExprUnary, ast::NodePtr,

    parser::Sequence<parser::Lex<lualex::Minus>, ExprUnary>,
    parser::Sequence<parser::Lex<lualex::Length>, ExprUnary>,
    parser::Sequence<parser::Lex<lualex::Not>, ExprUnary>,
    parser::Sequence<ExprSingleton>
>
{};

struct KeyValuePair : parser::Grammar
<
    KeyValuePair, ast::MakeTable::KeyValuePair,

    parser::Sequence<parser::Lex<lualex::BracketSquareOp>, HighLevelExpression, parser::Lex<lualex::BracketSquareCl>,
        parser::Lex<lualex::Assignment>, HighLevelExpression>,
    parser::Sequence<parser::Lex<lualex::Name>, parser::Lex<lualex::Assignment>, HighLevelExpression>,
    parser::Sequence<HighLevelExpression>
>
{
    static ast::MakeTable::KeyValuePair visit(ast::NodePtr& node)
    {
        return ast::MakeTable::KeyValuePair(std::move(node));
    }

    static ast::MakeTable::KeyValuePair visit(lualex::BracketSquareOp, ast::NodePtr& index, lualex::BracketSquareCl, lualex::Assignment, ast::NodePtr& node)
    {
        ast::MakeTable::KeyValuePair pair(std::move(node));
        pair.index = std::move(index);
        return pair;
    }

    static ast::MakeTable::KeyValuePair visit(const lualex::Name& name, lualex::Assignment, ast::NodePtr& node)
    {
        ast::MakeTable::KeyValuePair pair(std::move(node));
        pair.index = ast::NodePtr{ast::Constant(name.startingPos, name.name)};
        return pair;
    }
};

struct ExprSingleton : parser::Grammar
<
    ExprSingleton, ast::NodePtr,

    parser::Sequence<parser::Lex<lualex::BracketRoundOp>, HighLevelExpression, parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<ExprSingleton,
        parser::Lex<lualex::BracketSquareOp>, HighLevelExpression, parser::Lex<lualex::BracketSquareCl>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::Dot>, parser::Lex<lualex::Name>>,
    parser::Sequence<parser::Lex<lualex::Nil>>,
    parser::Sequence<parser::Lex<lualex::True>>,
    parser::Sequence<parser::Lex<lualex::False>>,
    parser::Sequence<parser::Lex<lualex::Name>>,
    parser::Sequence<parser::Lex<lualex::Number>>,
    parser::Sequence<parser::Lex<lualex::String>>,
    parser::Sequence<InlineFunction>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::BracketRoundOp>,
        parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<ast::NodePtr, HighLevelExpression, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<parser::Lex<lualex::BracketCurlyOp>,
        parser::Repeating<ast::MakeTable::KeyValuePair, KeyValuePair, lualex::Comma>,
        parser::Lex<lualex::BracketCurlyCl>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::Method>, parser::Lex<lualex::Name>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Repeating<ast::NodePtr, HighLevelExpression, lualex::Comma>,
        parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<ExprSingleton, parser::Lex<lualex::Method>, parser::Lex<lualex::Name>,
        parser::Lex<lualex::BracketRoundOp>,
        parser::Lex<lualex::BracketRoundCl>>,
    parser::Sequence<parser::Lex<lualex::BracketCurlyOp>, parser::Lex<lualex::BracketCurlyCl>>
>
{
    static ast::NodePtr visit(ast::NodePtr& root, lualex::Method, const lualex::Name& name, lualex::BracketRoundOp bOp,
        std::deque<ast::NodePtr>& args, lualex::BracketRoundCl)
    {
        args.emplace_front(std::move(root));
        ast::Variable var(name.startingPos, name.name);
        ast::NodePtr function(std::move(var));
        ast::FunctionCall call(bOp.startingPos, std::move(function));
        call.args = std::move(args);
        return ast::NodePtr{std::move(call)};
    }

    static ast::NodePtr visit(ast::NodePtr& root, lualex::Method, const lualex::Name& name, lualex::BracketRoundOp bOp,
        lualex::BracketRoundCl)
    {
        ast::Variable var(name.startingPos, name.name);
        ast::NodePtr function(std::move(var));
        ast::FunctionCall call(bOp.startingPos, std::move(function));
        call.args.emplace_front(std::move(root));
        return ast::NodePtr{std::move(call)};
    }

    static ast::NodePtr visit(lualex::BracketCurlyOp bOp, lualex::BracketCurlyCl)
    {
        return ast::NodePtr{ast::MakeTable(bOp.startingPos)};
    }

    static ast::NodePtr visit(lualex::BracketCurlyOp bOp, std::deque<ast::MakeTable::KeyValuePair>& pairs,
        lualex::BracketCurlyCl)
    {
        ast::MakeTable makeTable(bOp.startingPos);
        makeTable.values = std::move(pairs);
        return ast::NodePtr{std::move(makeTable)};
    }

    static ast::NodePtr visit(ast::NodePtr& body, lualex::BracketSquareOp bOp, ast::NodePtr& index, lualex::BracketSquareCl)
    {
        return ast::NodePtr{ast::IndexRequest(bOp.startingPos, std::move(body), std::move(index))};
    }

    static ast::NodePtr visit(ast::NodePtr& body, lualex::Dot dot, const lualex::Name& field)
    {
        ast::NodePtr index = ast::NodePtr{ast::Constant(field.startingPos, field.name)};
        return ast::NodePtr{ast::IndexRequest(dot.startingPos, std::move(body), std::move(index))};
    }

    static ast::NodePtr visit(lualex::BracketRoundOp, ast::NodePtr& value, lualex::BracketRoundCl)
    {
        return std::move(value);
    }

    static ast::NodePtr visit(lualex::Nil lex)
    {
        return ast::NodePtr{ast::Constant(lex.startingPos, data::Nil())};
    }

    static ast::NodePtr visit(lualex::True lex)
    {
        return ast::NodePtr{ast::Constant(lex.startingPos, true)};
    }

    static ast::NodePtr visit(lualex::False lex)
    {
        return ast::NodePtr{ast::Constant(lex.startingPos, false)};
    }

    static ast::NodePtr visit(const lualex::Name& varName)
    {
        return ast::NodePtr{ast::Variable(varName.startingPos, varName.name)};
    }

    static ast::NodePtr visit(const lualex::Number& constNumber)
    {
        return ast::NodePtr{ast::Constant(constNumber.startingPos, constNumber.value)};
    }

    static ast::NodePtr visit(const lualex::String& constString)
    {
        return ast::NodePtr{ast::Constant(constString.startingPos, constString.string)};
    }

    static ast::NodePtr visit(ast::Function& lambda)
    {
        return ast::NodePtr{std::move(lambda)};
    }

    static ast::NodePtr visit(ast::NodePtr& func, lualex::BracketRoundOp bOp, lualex::BracketRoundCl)
    {
        ast::FunctionCall function(bOp.startingPos, std::move(func));
        return ast::NodePtr{std::move(function)};
    }

    static ast::NodePtr visit(ast::NodePtr& func, lualex::BracketRoundOp bOp,
        std::deque<ast::NodePtr>& args, lualex::BracketRoundCl)
    {
        ast::FunctionCall function(bOp.startingPos, std::move(func));
        for (ast::NodePtr& arg : args)
        {
            function.args.emplace_back(std::move(arg));
        }
        return ast::NodePtr{std::move(function)};
    }
};

struct Assignment : parser::Grammar
<
    Assignment, ast::Assignment,

    // TODO: Implement op= assignment
    parser::Sequence<parser::Repeating<ast::NodePtr, ExprSingleton, lualex::Comma>,
        parser::Lex<lualex::Assignment>, parser::Repeating<ast::NodePtr, HighLevelExpression, lualex::Comma>>
>
{
    static ast::Assignment visit(std::deque<ast::NodePtr>& subjects, lualex::Assignment lex, std::deque<ast::NodePtr>& values)
    {
        ast::Assignment assignment(lex.startingPos);
        assignment.subjects = std::move(subjects);
        assignment.data = std::move(values);
        return assignment;
    }
};

struct LocalAssignment : parser::Grammar
<
    LocalAssignment, ast::LocalAssignment,

    parser::Sequence<
        parser::Lex<lualex::Local>,
        parser::Repeating<lualex::Name, parser::Lex<lualex::Name>, lualex::Comma>,
        parser::Lex<lualex::Assignment>,
        parser::Repeating<ast::NodePtr, HighLevelExpression, lualex::Comma>
    >,
    parser::Sequence<
        parser::Lex<lualex::Local>,
        parser::Repeating<lualex::Name, parser::Lex<lualex::Name>, lualex::Comma>
    >,

    parser::Sequence<
        parser::Lex<lualex::Local>,
        Function
    >
>
{
    static ast::LocalAssignment visit(lualex::Local, std::deque<lualex::Name>& names, lualex::Assignment lex,
        std::deque<ast::NodePtr>& values)
    {
        ast::LocalAssignment assignment(lex.startingPos);
        for (lualex::Name& name : names)
        {
            assignment.names.emplace_back(name.name);
        }
        assignment.values = std::move(values);
        return assignment;
    }

    static ast::LocalAssignment visit(lualex::Local lex, std::deque<lualex::Name>& names)
    {
        ast::LocalAssignment assignment(lex.startingPos);
        for (lualex::Name& name : names)
        {
            assignment.names.emplace_back(name.name);
        }
        return assignment;
    }

    static ast::LocalAssignment visit(lualex::Local lex, ast::Assignment& fncAssignment)
    {
        ast::LocalAssignment assignment(lex.startingPos);
        for (ast::NodePtr& var : fncAssignment.subjects)
        {
            assert(std::holds_alternative<ast::Variable>(*var));
            assignment.names.emplace_back(std::get<ast::Variable>(*var).name);
        }
        for (ast::NodePtr& value : fncAssignment.data)
        {
            assignment.values.emplace_back(std::move(value));
        }
        return assignment;
    }
};

struct BranchBgn;

struct Branch : parser::Grammar
<
    Branch, ast::Branch,

    parser::Sequence<BranchBgn, parser::Lex<lualex::End>>,
    parser::Sequence<BranchBgn, parser::Lex<lualex::Else>, Block, parser::Lex<lualex::End>>
>
{
    static ast::Branch visit(ast::Branch& branch, lualex::End)
    {
        return ast::Branch{std::move(branch)};
    }

    static ast::Branch visit(ast::Branch& branch, lualex::Else, std::deque<ast::NodePtr>& block, lualex::End)
    {
        branch.ifFalse = std::move(block);
        return ast::Branch{std::move(branch)};
    }
};

struct BranchBgn : parser::Grammar
<
    BranchBgn, ast::Branch,

    parser::Sequence<parser::Lex<lualex::If>, HighLevelExpression, parser::Lex<lualex::Then>, Block>,
    parser::Sequence<BranchBgn, parser::Lex<lualex::ElseIf>, HighLevelExpression, parser::Lex<lualex::Then>, Block>
>
{
    static ast::Branch visit(ast::Branch& branch, lualex::ElseIf, ast::NodePtr& condition,
        lualex::Then, std::deque<ast::NodePtr>& block)
    {
        branch.cases.emplace_back(std::move(condition), std::move(block));
        return std::move(branch);
    }

    static ast::Branch visit(lualex::If lex, ast::NodePtr& condition, lualex::Then, std::deque<ast::NodePtr>& block)
    {
        ast::Branch branch(lex.startingPos);
        branch.cases.emplace_back(std::move(condition), std::move(block));
        return branch;
    }
};

struct WhileLoop : parser::Grammar
<
    WhileLoop, ast::WhileLoop,

    parser::Sequence<parser::Lex<lualex::While>, HighLevelExpression, parser::Lex<lualex::Do>, Block,
        parser::Lex<lualex::End>>
>
{
    static ast::WhileLoop visit(lualex::While lex, ast::NodePtr& condition, lualex::Do,
        std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::WhileLoop loop(lex.startingPos, std::move(condition));
        loop.body = std::move(body);
        return loop;
    }
};

struct RepeatUntil : parser::Grammar
<
    RepeatUntil, ast::RepeatUntil,

    parser::Sequence<parser::Lex<lualex::Repeat>, Block, parser::Lex<lualex::Until>, HighLevelExpression>
>
{
    static ast::RepeatUntil visit(lualex::Repeat lex, std::deque<ast::NodePtr>& body, lualex::Until, ast::NodePtr& condition)
    {
        ast::RepeatUntil loop(lex.startingPos, std::move(condition));
        loop.body = std::move(body);
        return loop;
    }
};

struct NumericFor : parser::Grammar
<
    NumericFor, ast::ForLoopNumeric,

    parser::Sequence<
        parser::Lex<lualex::For>,
        parser::Lex<lualex::Name>,
        parser::Lex<lualex::Assignment>,
        HighLevelExpression,
        parser::Lex<lualex::Comma>,
        HighLevelExpression,
        parser::Lex<lualex::Do>,
        Block,
        parser::Lex<lualex::End>
    >,

    parser::Sequence<
        parser::Lex<lualex::For>,
        parser::Lex<lualex::Name>,
        parser::Lex<lualex::Assignment>,
        HighLevelExpression,
        parser::Lex<lualex::Comma>,
        HighLevelExpression,
        parser::Lex<lualex::Comma>,
        HighLevelExpression,
        parser::Lex<lualex::Do>,
        Block,
        parser::Lex<lualex::End>
    >
>
{
    static ast::ForLoopNumeric visit(lualex::For lex, const lualex::Name& name, lualex::Assignment,
        ast::NodePtr& start, lualex::Comma, ast::NodePtr& limit,
        lualex::Do, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::NodePtr step = ast::NodePtr{ast::Constant(lex.startingPos, 1.0)};
        ast::ForLoopNumeric loop(lex.startingPos, name.name, std::move(start), std::move(limit), std::move(step));
        loop.body = std::move(body);
        return loop;
    }

    static ast::ForLoopNumeric visit(lualex::For lex, const lualex::Name& name, lualex::Assignment,
        ast::NodePtr& start, lualex::Comma, ast::NodePtr& limit, lualex::Comma, ast::NodePtr& step,
        lualex::Do, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::ForLoopNumeric loop(lex.startingPos, name.name, std::move(start), std::move(limit), std::move(step));
        loop.body = std::move(body);
        return loop;
    }
};

struct GenericFor : parser::Grammar
<
    GenericFor, ast::ForLoopGeneric,

    parser::Sequence<
        parser::Lex<lualex::For>,
        parser::Repeating<lualex::Name, parser::Lex<lualex::Name>, lualex::Comma>,
        parser::Lex<lualex::In>,
        HighLevelExpression,
        parser::Lex<lualex::Do>,
        Block,
        parser::Lex<lualex::End>
    >
>
{
    static ast::ForLoopGeneric visit(lualex::For lex, const std::deque<lualex::Name>& names,
        lualex::In, ast::NodePtr& iterator, lualex::Do, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::ForLoopGeneric loop(lex.startingPos, std::move(iterator));
        for (const lualex::Name& name : names)
        {
            loop.names.emplace_back(name.name);
        }
        loop.body = std::move(body);
        return loop;
    }

    static ast::ForLoopNumeric visit(lualex::For lex, const lualex::Name& name, lualex::Assignment,
        ast::NodePtr& start, lualex::Comma, ast::NodePtr& limit, lualex::Comma, ast::NodePtr& step,
        lualex::Do, std::deque<ast::NodePtr>& body, lualex::End)
    {
        ast::ForLoopNumeric loop(lex.startingPos, name.name, std::move(start), std::move(limit), std::move(step));
        loop.body = std::move(body);
        return loop;
    }
};

struct Break : parser::Grammar
<
    Break, ast::Break,

    parser::Sequence<parser::Lex<lualex::Break>>
>
{
    static ast::Break visit(lualex::Break lex)
    {
        return ast::Break(lex.startingPos);
    }
};

struct Continue : parser::Grammar
<
    Continue, ast::Continue,

    parser::Sequence<parser::Lex<lualex::Continue>>
>
{
    static ast::Continue visit(lualex::Continue lex)
    {
        return ast::Continue(lex.startingPos);
    }
};

}
