#pragma once

#include "file_backend.h"
#include "common.h"
#include "posix_file_handler.h"

#include <unistd.h>

#include <memory>
#include <stdexcept>
#include <system_error>

namespace protostream {

/** A backend using pread/pwrite system calls. */
template <file_mode_t mode>
class posix_file_backend : public file_backend<posix_file_backend<mode>> {
public:
    using pointer_type = std::unique_ptr<const std::uint8_t[]>;

    posix_file_backend(const char* path) : file{path} {
    }

    posix_file_backend(const posix_file_backend&) = delete;

    posix_file_backend(posix_file_backend&&) = default;

    posix_file_backend& operator=(const posix_file_backend&) = delete;

    posix_file_backend& operator=(posix_file_backend&&) = default;

    template <class T>
    void read_small(offset_t offset, T* into) const {
        file.read(offset, sizeof(T), reinterpret_cast<std::uint8_t*>(into));
    }

    pointer_type read(offset_t offset, size_t length) const {
        auto result = std::make_unique<std::uint8_t[]>(length);
        file.read(offset, length, result.get());
        return {std::move(result)};
    }

    template <class T>
    void write_small(offset_t offset, const T* from) {
        static_assert(mode == file_mode_t::READ_APPEND, "writing into a read-only file");
        file.write(offset, sizeof(T), reinterpret_cast<const std::uint8_t*>(from));
    }

    template <bool /* dummy */ = true>
    void write(offset_t offset, size_t length, const std::uint8_t* from) {
        static_assert(mode == file_mode_t::READ_APPEND, "writing into a read-only file");
        file.write(offset, length, from);
    }

    std::size_t size() const {
        return file.size();
    }

private:
    posix_file_handler<mode> file;
};
}
