#pragma once

#include <tuple>

namespace flua::meta
{
template<typename T1, typename T2>
struct tuple_concat;

template<typename T1, typename T2>
using tuple_concat_t = typename tuple_concat<T1, T2>::type;

template<typename... A, typename... B>
struct tuple_concat<std::tuple<A...>, std::tuple<B...>>
{
    using type = std::tuple<A..., B...>;
};

namespace
{
    template <class... TupleTs, class ArgType, std::size_t... I>
    tuple_concat_t<std::tuple<TupleTs...>, std::tuple<std::decay_t<ArgType>>>
    expand_impl(std::tuple<TupleTs...>& tuple, ArgType&& arg, std::index_sequence<I...>)
    {
        return std::tuple_cat(
            std::make_tuple(std::move(std::get<I>(tuple))...),
            std::make_tuple(std::forward<ArgType>(arg))
        );
    }
}

template <class... TupleTs, class ArgType>
tuple_concat_t<std::tuple<TupleTs...>, std::tuple<std::decay_t<ArgType>>>
expand(std::tuple<TupleTs...>& tuple, ArgType&& arg)
{
    return expand_impl<TupleTs...>(
        tuple,
        std::forward<ArgType>(arg),
        std::index_sequence_for<TupleTs...>{}
    );
}
}
