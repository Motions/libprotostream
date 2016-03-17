#include <gtest/gtest.h>
#include "temporary_file.h"

#include "posix_file_handler.h"

using read_only_handler = protostream::posix_file_handler<protostream::file_mode_t::READ_ONLY>;
using read_append_handler = protostream::posix_file_handler<protostream::file_mode_t::READ_APPEND>;

TEST(posix_file_handler, open_read) {
    const auto file = temporary_file{};
    EXPECT_THROW(read_only_handler{file.c_str()};, std::system_error);
}

TEST(posix_file_handler, open_write) {
    const auto file = temporary_file{};
    read_append_handler{file.c_str()};
}

TEST(posix_file_handler, write) {
    const auto file = temporary_file{};
    auto handler = read_append_handler{file.c_str()};

    constexpr auto offset = 42;

    const char bytes[] = "proto";
    handler.write(offset, strlen(bytes), reinterpret_cast<const std::uint8_t*>(bytes));

    auto stream = std::ifstream{file.c_str(), std::ifstream::binary};
    stream.seekg(0, stream.end);
    ASSERT_EQ(offset + strlen(bytes), stream.tellg());
    stream.seekg(0, stream.beg);

    auto buffer = file.contents();

    EXPECT_EQ(std::string(offset, '\0'), std::string(std::cbegin(buffer), std::cbegin(buffer) + offset));
    EXPECT_EQ(std::string{bytes}, std::string(std::cbegin(buffer) + offset, std::cend(buffer)));
}

TEST(posix_file_handler, size_write) {
    const auto file = temporary_file{};

    const auto handler = read_append_handler{file.c_str()};
    ASSERT_EQ(0, handler.size());
    EXPECT_EQ(0, boost::filesystem::file_size(file.path));
}

TEST(posix_file_handler, size_read) {
    const auto payload = std::string{"Hello world"};
    const auto file = make_temporary(payload);

    const auto handler = read_only_handler{file.c_str()};
    ASSERT_EQ(payload.length(), handler.size());
    EXPECT_EQ(payload.length(), boost::filesystem::file_size(file.path));
}

TEST(posix_file_handler, read) {
    const auto payload = std::string{"Hello world"};
    const auto file = make_temporary(payload);

    const auto handler = read_only_handler{file.c_str()};

    auto buffer = std::array<std::uint8_t, 6>{};
    constexpr auto offset = 3;
    handler.read(offset, buffer.size(), buffer.data());

    EXPECT_EQ(payload.substr(offset, buffer.size()), std::string(std::begin(buffer), std::end(buffer)));
}

TEST(posix_file_handler, mmap_read) {
    const auto payload = std::string{"Hello world"};
    const auto file = make_temporary(payload);

    auto handler = read_only_handler{file.c_str()};
    ASSERT_EQ(payload.length(), handler.size());

    auto buffer = handler.mmap();
    try {
        EXPECT_EQ(payload, std::string(buffer, buffer + payload.length()));
    } catch(...) {
        handler.munmap(buffer);
        throw;
    }
}

TEST(posix_file_handler, mmap_write) {
    constexpr const char payload[] = "Hello world";
    constexpr auto payload_length = sizeof(payload) - 1;
    const auto file = make_temporary(payload);

    auto handler = read_append_handler{file.c_str()};
    ASSERT_EQ(payload_length, handler.size());

    {
        auto buffer = handler.mmap();
        buffer[3] = 'p';
        handler.munmap(buffer);
        ASSERT_EQ(payload_length, handler.size());
    }

    EXPECT_EQ("Helpo world", file.contents());
}

TEST(posix_file_handler, mremap) {
    constexpr auto add = "far, far away";
    constexpr auto offset = 64 * 1024 * 1024;

    const auto file = make_temporary("some random string");
    auto handler = read_append_handler{file.c_str()};

    const auto old_length = handler.size();
    auto buffer = handler.mmap();

    {
        std::ofstream stream{file.c_str(), std::ofstream::binary};
        stream.seekp(offset, std::ios_base::beg);
        stream << add;
    }

    ASSERT_EQ(offset + strlen(add), handler.size());

    buffer = handler.mremap(buffer, old_length, handler.size());
    EXPECT_EQ(std::string(add), std::string(buffer + offset, buffer + offset + strlen(add)));

    handler.munmap(buffer);
}

TEST(posix_file_handler, expand) {
    constexpr auto expand_by = 64 * 1024 * 1024;
    const auto payload = std::string{"foobar"};
    const auto file = make_temporary(payload);

    auto handler = read_append_handler{file.c_str()};
    ASSERT_EQ(payload.size(), handler.size());

    handler.expand(expand_by);
    EXPECT_EQ(payload.size() + expand_by, handler.size());
    EXPECT_EQ(payload.size() + expand_by, boost::filesystem::file_size(file.path));
}

TEST(posix_file_handler, truncate) {
    const auto file = make_temporary("some random string");
    auto handler = read_append_handler{file.c_str()};

    handler.truncate(4);
    EXPECT_EQ(4, handler.size());
    EXPECT_EQ(4, boost::filesystem::file_size(file.path));
}