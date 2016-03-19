#pragma once

#include <tuple>

namespace {
/** A simple wrapper used to create a product of type-lists */
template<class Stream, class Test>
struct wrapper {
    using stream = Stream;
    using test = Test;
};

/** Concatenates type-level lists
 * concat<List1<Elts1...>, List2<Elts2...>, List3<Elts3...>, ...>::type<List>
 *   ==
 * List<Elts1..., Elts2..., Elts3..., ...>
 */
template<class...>
struct concat;

template<>
struct concat<> {
    template<template<class...> class List>
    using type = List<>;
};

template<template<class...> class List1, class... Elts1, class... Rest>
struct concat<List1<Elts1...>, Rest...> {
private:
    template<template<class...> class List>
    struct add_self {
        template<class... Elts>
        using type = List<Elts1..., Elts...>;
    };
public:
    template<template<class...> class List>
    using type = typename concat<Rest...>::template type<add_self<List>::template type>;
};

/** Computes the Cartesian product of two lists */
template<class, class>
struct product;

template<
        template<class...> class List1,
        template<class...> class List2,
        class... Elts1,
        class... Elts2>
struct product<List1<Elts1...>, List2<Elts2...>> {
private:
    template<template<class...> class List, class Elt1>
    using aux = List<wrapper<Elt1, Elts2>...>;
public:
    template<template<class...> class List>
    using type = typename concat<aux<std::tuple, Elts1>...>::template type<List>;
};

template<class>
struct unapply;

template<template<class...> class Constructor, class... Args>
struct unapply<Constructor<Args...>> {
    template<template<class...> class NewConstructor>
    using type = NewConstructor<Args...>;
};
}