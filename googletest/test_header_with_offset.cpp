#include <gtest/gtest.h>
#include "header/with_offset.h"

namespace {
struct field : public protostream::detail::with_offset<std::uint32_t, 4> { };
}

TEST(header_with_offset, read) {
    const auto data = std::array<std::uint8_t, 8>{
            '\x00', '\x00', '\x00', '\x00',
            '\x12', '\x34', '\x56', '\x78'
    };

    EXPECT_EQ(0x12345678u, field::read(data.data()));
}

TEST(header_with_offset, write) {
    auto data = std::array<std::uint8_t, 8>{
            '\x01', '\x02', '\x03', '\x04',
            '\x12', '\x34', '\x56', '\x78'
    };

    auto tested = field{};
    tested.value = 0x43211234u;
    tested.write(data.data());

    const auto expected = decltype(data){
            '\x01', '\x02', '\x03', '\x04',
            '\x43', '\x21', '\x12', '\x34'
    };

    EXPECT_EQ(expected, data);
}