#ifndef _FILE_BACKEND_H
#define _FILE_BACKEND_H

#include "backend.h"
#include "common.h"
#include "file_handler.h"

#include <unistd.h>

#include <memory>
#include <stdexcept>
#include <system_error>

namespace protostream {

/** A backend using pread/pwrite system calls. */
template<file_mode_t mode>
class file_backend : public backend<file_backend<mode>> {
public:
    using pointer_type = std::unique_ptr<std::uint8_t[]>;

    file_backend(const char *path) : file{path} { }

    file_backend(const file_backend &) = delete;

    file_backend(file_backend &&) = default;

    file_backend &operator=(const file_backend &) = delete;

    file_backend &operator=(file_backend &&) = default;

    template<class T>
    void read_small(offset_t offset, T *into) const {
        read_impl(offset, sizeof(T), reinterpret_cast<std::uint8_t *>(into));
    }

    pointer_type read(offset_t offset, size_t length) const {
        pointer_type result{new std::uint8_t[length]};
        read_impl(offset, length, result.get());
        return result;
    }

    template<class T>
    void write_small(offset_t offset, T *from) {
        static_assert(mode == file_mode_t::READ_APPEND, "writing into a read-only file");
        write_impl(offset, sizeof(T), reinterpret_cast<std::uint8_t *>(from));
    }

    template<bool /* dummy */ = true>
    void write(offset_t offset, size_t length, const std::uint8_t *from) {
        static_assert(mode == file_mode_t::READ_APPEND, "writing into a read-only file");
        write_impl(offset, length, from);
    }

    std::size_t size() const {
        return file.size();
    }

private:
    file_handler<mode> file;

    void read_impl(offset_t offset, size_t length, std::uint8_t *into) const;

    void write_impl(offset_t offset, size_t length, const std::uint8_t *from);
};

template<file_mode_t mode>
void file_backend<mode>::read_impl(offset_t offset, size_t length, std::uint8_t *into) const {
    size_t count = 0;
    while (count < length) {
        ssize_t ret;
        ret = pread(file.fd, into + count, length - count, offset + count);

        if (ret == 0) {
            throw std::logic_error("Premature end of file");
        } else if (ret < 0) {
            throw std::system_error(errno, std::system_category(), "pread");
        }

        count += ret;
    }
}

template<>
void file_backend<file_mode_t::READ_APPEND>::write_impl(offset_t offset, size_t length, const std::uint8_t *from) {
    size_t count = 0;
    while (count < length) {
        ssize_t ret;
        ret = pwrite(file.fd, from + count, length - count, offset + count);

        if (ret == 0) {
            throw std::logic_error("Premature end of file");
        } else if (ret < 0) {
            throw std::system_error(errno, std::system_category(), "pwrite");
        }

        count += ret;
    }
}

}

#endif // _FILE_BACKEND_H
