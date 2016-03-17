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
        close();
    }

    void close() {
        released = true;
        if (boost::filesystem::exists(path)) {
            boost::filesystem::remove(path);
        }
    }

    boost::filesystem::path path;
    bool released;
};