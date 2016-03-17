#include <gtest/gtest.h>
#include "header/magic_value.h"

namespace {
constexpr const char magic_value[] = "magic";
constexpr auto magic_size = sizeof(magic_value) - 1;
constexpr auto offset = 2;

struct magic : protostream::detail::magic_value<magic_value, magic_size, offset> {};
}

TEST(header_magic_value, write_buffer) {
    std::array<std::uint8_t, 8> data;
    data.fill(0);

    magic{}.write(data.data());

    decltype(data) expected{{0, 0, 'm', 'a', 'g', 'i', 'c', 0}};

    EXPECT_EQ(expected, data);
}

TEST(header_magic_value, read_buffer) {
    auto data = std::array<std::uint8_t, 8>{{0, 0, 'm', 'a', 'g', 'i', 'c', 0}};
    magic::read(data.data());
}

TEST(header_magic_value, read_buffer_invalid) {
    auto data = std::array<std::uint8_t, 8>{{0, 0, 'w', 'r', 'o', 'n', 'g', 0}};
    EXPECT_THROW(magic::read(data.data()), std::logic_error);
}