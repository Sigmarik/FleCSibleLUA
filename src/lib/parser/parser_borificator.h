//! WARNING: This file is intended to be included in the middle of parser.h

#pragma once

template<bool ConditionV, class IfTrueT, class IfFalseT>
struct choose_type;
template<bool ConditionV, class IfTrueT, class IfFalseT>
using choose_type_t = typename choose_type<ConditionV, IfTrueT, IfFalseT>::type;

template<class IfTrueT, class IfFalseT>
struct choose_type<true, IfTrueT, IfFalseT>
{
    using type = IfTrueT;
};

template<class IfTrueT, class IfFalseT>
struct choose_type<false, IfTrueT, IfFalseT>
{
    using type = IfFalseT;
};


template<class... Sequences>
struct make_forest;
template<class... Sequences>
using make_forest_t = typename make_forest<Sequences...>::type;

template<class SeqT, class ForestT>
struct append_to_forest;
template<class SeqT, class ForestT>
using append_to_forest_t = typename append_to_forest<SeqT, ForestT>::type;

template<>
struct make_forest<>
{
    using type = Forest<false>;
};

template<class FirstElement, class... Elements>
struct make_forest<Sequence<FirstElement, Elements...> >
{
    using type = Forest<false, Tree<FirstElement, make_forest_t<Sequence<Elements...> > > >;
};

template<>
struct make_forest<Sequence<> >
{
    using type = Forest<true>;
};

template<class FirstSeq, class... Sequences>
struct make_forest<FirstSeq, Sequences...>
{
    using type = append_to_forest_t<FirstSeq, make_forest_t<Sequences...> >;
};

// Empty sequence, empty forest
template<bool ForestFinalV>
struct append_to_forest<Sequence<>, Forest<ForestFinalV> >
{
    using type = Forest<true>;
};

// Empty sequence, full forest
template<bool ForestFinalV, class... ForestTrees>
struct append_to_forest<Sequence<>, Forest<ForestFinalV, ForestTrees...> >
{
    using type = Forest<true, ForestTrees...>;
};

// Empty forest, full sequence
template<class SequenceFirstT, class... SequenceTs, bool ForestFinalV>
struct append_to_forest<Sequence<SequenceFirstT, SequenceTs...>, Forest<ForestFinalV> >
{
    using type = Forest<ForestFinalV, Tree<SequenceFirstT, make_forest_t<Sequence<SequenceTs...>>>>;
};

// Full sequence and forest
template<class SequenceFirstT, class... SequenceRestT, bool ForestFinalV, class ForestFirstT, class... ForestTrees>
struct append_to_forest<Sequence<SequenceFirstT, SequenceRestT...>, Forest<ForestFinalV, ForestFirstT, ForestTrees...> >
{
    using type = choose_type_t<std::is_same_v<SequenceFirstT, typename ForestFirstT::Key>,
        Forest<ForestFinalV,
            Tree<SequenceFirstT, append_to_forest_t<Sequence<SequenceRestT...>, typename ForestFirstT::Forest> >,
            ForestTrees...>,
        Forest<ForestFinalV,
            Tree<SequenceFirstT, make_forest_t<Sequence<SequenceRestT...> > >, ForestFirstT,
            ForestTrees...>
    >;
};
