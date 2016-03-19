#pragma once

#include <tuple>

namespace simple_tests {
struct small_test {
    static constexpr auto file = "data/small.data";
    static constexpr auto header = "protostream header";
    static constexpr auto keyframe_count = 10;
    static constexpr auto frame_count = 20;
    static constexpr auto frames_per_keyframe = 2;
};

struct medium_test {
    static constexpr auto file = "data/medium.data";
    static constexpr auto header = "header";
    static constexpr auto keyframe_count = 11;
    static constexpr auto frame_count = 109;
    static constexpr auto frames_per_keyframe = 10;
};

using tests = std::tuple<small_test, medium_test>;

}