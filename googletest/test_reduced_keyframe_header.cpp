#include <gtest/gtest.h>
#include "header.h"

namespace {
constexpr const char serialised[] =
        "\x01\x23\x45\x67\x76\x54\x32\x10" /* keyframe number */
        "\xde\xad\xbe\xef\x01\x9d\xf3\x7a" /* delta start */
        "\xff\x00\x00\x00\x00\x00\x00\xff" /* skiplist[0] */
        "\x00\xff\x00\x00\x00\x00\xff\x00" /* skiplist[1] */
        "\x00\x00\xff\x00\x00\xff\x00\x00" /* skiplist[2] */
        "\x00\x00\x00\xff\xff\x00\x00\x00" /* skiplist[3] */
        "\x00\x00\x00\xff\xff\x00\x00\x00" /* skiplist[4] */
        "\x00\x00\xff\x00\x00\xff\x00\x00" /* skiplist[5] */
        "\x00\xff\x00\x00\x00\x00\xff\x00" /* skiplist[6] */
        "\xff\x00\x00\x00\x00\x00\x00\xff" /* skiplist[7] */
        "\x00\x00\x00\xee\xee\x00\x00\x00" /* skiplist[8] */
        "\x00\x00\xdd\xee\xee\xdd\x00\x00" /* skiplist[9] */
        "\xf0\x0f\x12\x34";                /* keyframe size */
constexpr auto kf_num = std::uint64_t{0x0123456776543210llu};
constexpr auto delta_offset = std::uint64_t{0xdeadbeef019df37allu};
constexpr auto kf_size = std::uint32_t{0xf00f1234u};
}

static_assert(protostream::reduced_keyframe_header::size == sizeof(serialised) - 1, "Wrong header size");

TEST(reduced_keyframe_header, read) {
    const auto header = protostream::reduced_keyframe_header::read(reinterpret_cast<const std::uint8_t*>(serialised));

    EXPECT_EQ(kf_num, header.get<protostream::fields::kf_num>());
    EXPECT_EQ(delta_offset, header.get<protostream::fields::delta_offset>());
    EXPECT_EQ(kf_size, header.get<protostream::fields::kf_size>());
}

TEST(reduced_keyframe_header, write) {
    auto header = protostream::reduced_keyframe_header{};

    header.get<protostream::fields::kf_num>() = kf_num;
    header.get<protostream::fields::delta_offset>() = delta_offset;
    header.get<protostream::fields::kf_size>() = kf_size;

    auto data = std::array<std::uint8_t, sizeof(serialised)>{};
    header.write(data.data());

    EXPECT_EQ(std::string(std::begin(serialised), std::begin(serialised) + 16),
              std::string(std::begin(data), std::begin(data) + 16));

    EXPECT_EQ(std::string(std::end(serialised) - 4, std::end(serialised)),
              std::string(std::end(data) - 4, std::end(data)));
}