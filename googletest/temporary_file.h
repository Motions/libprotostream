#pragma once

#include <boost/filesystem.hpp>

struct temporary_file {
    temporary_file() : path{boost::filesystem::unique_path()}, released{false} { }

    const char* c_str() const noexcept {
        return path.native().c_str();
    }

    temporary_file(const temporary_file&) = delete;

    temporary_file(temporary_file&& that) noexcept : path{std::move(that.path)}, released{false} { }

    temporary_file& operator=(const temporary_file&) = delete;

    temporary_file& operator=(temporary_file&& that) noexcept {
        path = std::move(that.path);
        released = false;
        that.released = true;
        return *this;
    }

    ~temporary_file() {
        if (!released) {
            close();
        }
    }

    void close() {
        if (released) {
            throw std::logic_error{"Closing a released temporary file"};
        }

        released = true;
        if (boost::filesystem::exists(path)) {
            boost::filesystem::remove(path);
        }
    }

    std::string contents() const {
        std::ifstream stream{c_str(), std::ifstream::binary};
        return {std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
    }

    std::size_t size() const {
        return boost::filesystem::file_size(path);
    }

    boost::filesystem::path path;
    bool released;
};

static temporary_file make_temporary(const std::string &payload) {
    auto file = temporary_file{};

    auto stream = std::ofstream{file.c_str(), std::ofstream::binary};
    stream << payload;

    return file;
}