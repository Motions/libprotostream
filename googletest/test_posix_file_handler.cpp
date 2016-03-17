#include <gtest/gtest.h>
#include "temporary_file.h"

#include "posix_file_handler.h"

using namespace protostream;
using namespace protostream::detail;

TEST(posix_file_handler, open_read) {
    const auto file = temporary_file{};
    EXPECT_THROW(posix_file_handler<file_mode_t::READ_ONLY>{file.c_str()};, std::system_error);
}

TEST(posix_file_handler, open_write) {
    const auto file = temporary_file{};
    posix_file_handler<file_mode_t::READ_APPEND>{file.c_str()};
}

TEST(posix_file_handler, write) {
    const auto file = temporary_file{};
    auto handler = posix_file_handler<file_mode_t::READ_APPEND>{file.c_str()};

    constexpr auto offset = 42;

    const char bytes[] = {'p', 'r', 'o' , 't', 'o'};
    handler.write(offset, sizeof(bytes), reinterpret_cast<const std::uint8_t*>(bytes));

    auto stream = std::ifstream{file.c_str(), std::ifstream::binary};
    stream.seekg(0, stream.end);
    ASSERT_EQ(offset + sizeof(bytes), stream.tellg());
    stream.seekg(0, stream.beg);

    auto buffer = std::array<char, offset + sizeof(bytes)>{};
    buffer.fill(0);
    stream.read(buffer.data(), offset + sizeof(bytes));

    EXPECT_EQ(std::string(offset, '\0'), std::string(buffer.cbegin(), buffer.cbegin() + offset));
    EXPECT_EQ(std::string{bytes}, std::string(buffer.cbegin() + offset, buffer.cend()));
}

temporary_file make_temporary(const std::string &payload) {
    auto file = temporary_file{};

    auto stream = std::ofstream{file.c_str(), std::ofstream::binary};
    stream << payload;

    return std::move(file);
}

TEST(posix_file_handler, size) {
    const auto payload = std::string{"Hello world"};
    const auto file = make_temporary(payload);

    const auto handler = posix_file_handler<file_mode_t::READ_ONLY>(file.c_str());
    ASSERT_EQ(payload.length(), handler.size());
}

TEST(posix_file_handler, read) {
    const auto payload = std::string{"Hello world"};
    const auto file = make_temporary(payload);

    const auto handler = posix_file_handler<file_mode_t::READ_ONLY>(file.c_str());

    auto buffer = std::array<std::uint8_t, 6>{};
    constexpr auto offset = 3;
    handler.read(offset, buffer.size(), buffer.data());

    EXPECT_EQ(payload.substr(offset, buffer.size()), std::string(buffer.begin(), buffer.end()));
}

TEST(posix_file_handler, mmap_read) {
    const auto payload = std::string{"Hello world"};
    const auto file = make_temporary(payload);

    auto handler = posix_file_handler<file_mode_t::READ_ONLY>(file.c_str());
    ASSERT_EQ(payload.length(), handler.size());

    auto buffer = handler.mmap();
    try {
        EXPECT_EQ(payload, std::string(buffer, buffer + payload.length()));
    } catch(...) {
        handler.munmap(buffer);
        throw;
    }
}