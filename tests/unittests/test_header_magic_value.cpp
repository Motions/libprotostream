#include <gtest/gtest.h>
#include "header/magic_value.h"

namespace {
constexpr auto offset = 2;

struct magic_value : protostream::detail::magic_value<magic_value, offset> {
    static constexpr const char* magic() {
        return "magic";
    }

    static constexpr auto size = 5;
};
}

TEST(header_magic_value, write_buffer) {
    std::array<std::uint8_t, 8> data;
    data.fill(0);

    magic_value{}.write(data.data());

    decltype(data) expected{{0, 0, 'm', 'a', 'g', 'i', 'c', 0}};

    EXPECT_EQ(expected, data);
}

TEST(header_magic_value, read_buffer) {
    auto data = std::array<std::uint8_t, 8>{{0, 0, 'm', 'a', 'g', 'i', 'c', 0}};
    magic_value::read(data.data());
}

TEST(header_magic_value, read_buffer_invalid) {
    auto data = std::array<std::uint8_t, 8>{{0, 0, 'w', 'r', 'o', 'n', 'g', 0}};
    EXPECT_THROW(magic_value::read(data.data()), std::logic_error);
}