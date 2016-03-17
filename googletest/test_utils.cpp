#include <gtest/gtest.h>
#include "utils.h"

namespace detail = protostream::detail;

TEST(utils, endian64) {
    const auto value     = static_cast<std::uint64_t>(0x1234567887654321llu);
    const auto converted = detail::htobe(value);
    const auto back      = detail::betoh(converted);
    const auto bytes     = reinterpret_cast<const std::uint8_t*>(&converted);

    EXPECT_EQ(0x12, bytes[0]);
    EXPECT_EQ(0x34, bytes[1]);
    EXPECT_EQ(0x56, bytes[2]);
    EXPECT_EQ(0x78, bytes[3]);
    EXPECT_EQ(0x87, bytes[4]);
    EXPECT_EQ(0x65, bytes[5]);
    EXPECT_EQ(0x43, bytes[6]);
    EXPECT_EQ(0x21, bytes[7]);

    EXPECT_EQ(value, back);
}

TEST(utils, endian32) {
    const auto value     = static_cast<std::uint32_t>(0x12345678u);
    const auto converted = detail::htobe(value);
    const auto back      = detail::betoh(converted);
    const auto bytes     = reinterpret_cast<const std::uint8_t*>(&converted);

    EXPECT_EQ(0x12, bytes[0]);
    EXPECT_EQ(0x34, bytes[1]);
    EXPECT_EQ(0x56, bytes[2]);
    EXPECT_EQ(0x78, bytes[3]);

    EXPECT_EQ(value, back);
}

TEST(utils, endian16) {
    const auto value     = static_cast<std::uint16_t>(0x1234u);
    const auto converted = detail::htobe(value);
    const auto back      = detail::betoh(converted);
    const auto bytes     = reinterpret_cast<const std::uint8_t*>(&converted);

    EXPECT_EQ(0x12, bytes[0]);
    EXPECT_EQ(0x34, bytes[1]);

    EXPECT_EQ(value, back);
}

TEST(utils, writebuf) {
    const auto value = static_cast<std::uint32_t>(0xdeadbeefu);
    auto buffer = std::uint32_t{};

    detail::writebuf(&buffer, value);

    EXPECT_EQ(detail::htobe(value), buffer);
}

TEST(utils, readbuf_aligned) {
    const auto value = static_cast<std::uint32_t>(0xdeadbeefu);
    const auto buffer = detail::htobe(value);

    EXPECT_EQ(value, detail::readbuf_aligned<std::uint32_t>(&buffer));
}

TEST(utils, readbuf_unaligned) {
    const auto value = static_cast<std::uint32_t>(0xdeadbeefu);
    const auto buffer = detail::htobe(value);

    EXPECT_EQ(value, detail::readbuf_unaligned<std::uint32_t>(&buffer));
}