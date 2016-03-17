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
    auto new_payload = "foobar";
    backend.write(3, strlen(new_payload), reinterpret_cast<const std::uint8_t*>(new_payload));

    EXPECT_EQ("helfoobarld", file.contents());
}