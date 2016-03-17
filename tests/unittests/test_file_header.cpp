#include <gtest/gtest.h>
#include "header.h"

namespace {
constexpr const char serialised[] =
        "PROTOSTR"                         /* magic value */
        ""                                 /* padding */
        "\xde\xad\xbe\xef\xfe\xbe\xad\xde" /* file size */
        "\x12\x23\x34\x45\x98\x87\x76\x65" /* header offset */
        "\x99\x88\x77\x66\x12\x83\x29\x38" /* kf0 offset */
        "\x8f\x1e\x0b\x3d\x9a\x7b\x00\xff" /* keyframe count */
        "\x00\x01\x02\x03\x29\x10\xff\xff" /* frame count */
        "\xff\x0d\xe9\x21"                 /* frames per keyframe */
        "\x00\x00\x00\x00";                /* reserved */

constexpr auto file_size = std::uint64_t{0xdeadbeeffebeaddellu};
constexpr auto proto_header_offset = std::uint64_t{0x1223344598877665llu};
constexpr auto kf0_offset = std::uint64_t{0x9988776612832938llu};
constexpr auto keyframe_count = std::uint64_t{0x8f1e0b3d9a7b00ffllu};
constexpr auto frame_count = std::uint64_t{0x000102032910ffffllu};
constexpr auto frames_per_kf = std::uint32_t{0xff0de921u};
}

static_assert(protostream::file_header::size == sizeof(serialised) - 1, "Wrong header size");

TEST(file_header, read) {
    const auto header = protostream::file_header::read(reinterpret_cast<const std::uint8_t*>(serialised));

    EXPECT_EQ(file_size, header.get<protostream::fields::file_size>());
    EXPECT_EQ(proto_header_offset, header.get<protostream::fields::proto_header_offset>());
    EXPECT_EQ(kf0_offset, header.get<protostream::fields::kf0_offset>());
    EXPECT_EQ(keyframe_count, header.get<protostream::fields::keyframe_count>());
    EXPECT_EQ(frame_count, header.get<protostream::fields::frame_count>());
    EXPECT_EQ(frames_per_kf, header.get<protostream::fields::frames_per_kf>());
}

TEST(file_header, write) {
    auto header = protostream::file_header{};

    header.get<protostream::fields::file_size>() = file_size;
    header.get<protostream::fields::proto_header_offset>() = proto_header_offset;
    header.get<protostream::fields::kf0_offset>() = kf0_offset;
    header.get<protostream::fields::keyframe_count>() = keyframe_count;
    header.get<protostream::fields::frame_count>() = frame_count;
    header.get<protostream::fields::frames_per_kf>() = frames_per_kf;

    auto data = std::array<std::uint8_t, sizeof(serialised)>{};
    data.fill(0);
    header.write(data.data());

    EXPECT_EQ(std::string(std::begin(serialised), std::end(serialised)),
              std::string(std::begin(data), std::end(data)));
}