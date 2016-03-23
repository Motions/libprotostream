#include "type_list.h"

#include <type_traits>

namespace type_list {

/** This file contains tests of the type_list module */

static_assert(
    std::is_same<
        concat<std::tuple<int, unsigned>, std::tuple<int, int>, std::tuple<>>::type<std::tuple>,
        std::tuple<int, unsigned, int, int>>{},
    "Sample concatenation");

static_assert(std::is_same<concat<>::type<std::tuple>, std::tuple<>>{},
              "Concatenation of zero lists");

static_assert(
    std::is_same<product<std::tuple<int, unsigned>, std::tuple<float, double>>::type<std::tuple>,
                 std::tuple<wrapper<int, float>,
                            wrapper<int, double>,
                            wrapper<unsigned, float>,
                            wrapper<unsigned, double>>>{},
    "Sample product");

static_assert(
    std::is_same<unapply<std::pair<int, float>>::type<std::tuple>, std::tuple<int, float>>{},
    "Sample unapply");
}