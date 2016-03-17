#include <gtest/gtest.h>
#include "posix_file_backend.h"
#include "mmap_backend.h"

#include "temporary_file.h"

using backends = testing::Types<
        protostream::mmap_backend<protostream::file_mode_t::READ_APPEND>,
        protostream::posix_file_backend<protostream::file_mode_t::READ_APPEND>>;

template<class T>
struct file_backend : public testing::Test { };

TYPED_TEST_CASE(file_backend, backends);

TYPED_TEST(file_backend, open) {
    auto file = temporary_file{};
    TypeParam{file.c_str()};
}

TYPED_TEST(file_backend, read) {
    const auto payload = std::string{"hello world"};
    auto file = make_temporary(payload);

    auto backend = TypeParam{file.c_str()};
    const auto offset = 3;
    const auto length = 4;
    auto data = backend.read(offset, length);
    auto ptr = protostream::detail::as_ptr(data);

    EXPECT_EQ(payload.substr(offset, length), std::string(ptr, ptr + length));
}

TYPED_TEST(file_backend, write) {
    const auto payload = std::string{"hello world"};
    auto file = make_temporary(payload);

    auto backend = TypeParam{file.c_str()};
    const auto new_payload = "foobar";
    backend.write(3, strlen(new_payload), reinterpret_cast<const std::uint8_t*>(new_payload));

    EXPECT_EQ("helfoobarld", file.contents());
}

TYPED_TEST(file_backend, write_far) {
    auto file = temporary_file{};

    const auto payload = "foobar";
    const auto offset = 8192;

    const auto expected = std::string(offset, '\0') + payload;

    {
        auto backend = TypeParam{file.c_str()};
        backend.write(offset, strlen(payload), reinterpret_cast<const std::uint8_t*>(payload));

        auto prefix_of = [](const std::string &fst, const std::string &snd) {
            if(fst.size() > snd.size())
                return false;

            return fst == snd.substr(0, fst.size());
        };

        EXPECT_PRED2(prefix_of, expected, file.contents());
    }

    EXPECT_EQ(expected.size(), file.size());
}

TYPED_TEST(file_backend, write_small) {
    auto file = temporary_file{};
    const auto payload = std::uint64_t{0xdeadbeef12345678llu};
    const auto big_endian = protostream::detail::htobe(payload);

    {
        auto backend = TypeParam{file.c_str()};
        backend.write_small(8, &big_endian);
    }

    auto expected = std::string{'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
                                '\xde', '\xad', '\xbe', '\xef', '\x12', '\x34', '\x56', '\x78'};

    EXPECT_EQ(expected, file.contents());
}

TYPED_TEST(file_backend, read_small) {
    auto file = make_temporary({'\x12', '\x34', '\x56', '\x78'});
    auto big_endian = std::uint16_t{};

    auto backend = TypeParam{file.c_str()};
    backend.read_small(2, &big_endian);

    EXPECT_EQ(0x5678, protostream::detail::betoh(big_endian));
}