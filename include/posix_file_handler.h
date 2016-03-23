#pragma once

#include "common.h"
#include "config.h"

#include <cstdint>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <system_error>

namespace protostream {

enum class file_mode_t { READ_ONLY, READ_APPEND };

namespace detail {
constexpr int open_flags(file_mode_t mode) {
    return mode == file_mode_t::READ_ONLY ? O_RDONLY : O_RDWR | O_CREAT;
}
}

/** A RAII wrapper around open files */
template <file_mode_t mode>
class posix_file_handler {
public:
    using buffer_type =
        std::conditional_t<mode == file_mode_t::READ_ONLY, const std::uint8_t*, std::uint8_t*>;

    const int fd;

    posix_file_handler(const char* path) : fd(open(path, detail::open_flags(mode), 0666)) {
        if (fd == -1) {
            throw std::system_error{errno, std::system_category(), "open"};
        }
    }

    ~posix_file_handler() {
        close(fd);
    }

    posix_file_handler(const posix_file_handler&) = delete;

    posix_file_handler(posix_file_handler&&) = default;

    posix_file_handler& operator=(const posix_file_handler&) = delete;

    posix_file_handler& operator=(posix_file_handler&&) = default;

    void read(offset_t offset, std::size_t length, std::uint8_t* into) const;

    void write(offset_t offset, size_t length, const std::uint8_t* from);

    buffer_type mmap() {
        return mmap(size());
    }

    buffer_type mmap(std::size_t size);

    void munmap(const buffer_type buffer) const {
        munmap(buffer, size());
    }

    void munmap(const buffer_type buffer, std::size_t size) const;

    buffer_type mremap(const buffer_type buffer, std::size_t old_size, std::size_t new_size);

    void expand(std::size_t expand_by) {
        expand(size(), expand_by);
    }

    void expand(std::size_t current_size, std::size_t expand_by);

    void truncate(std::size_t new_size);

    std::size_t size() const {
        struct stat st;

        if (fstat(fd, &st) != 0) {
            throw std::system_error{errno, std::system_category(), "fstat"};
        }

        return st.st_size;
    }
};

template <file_mode_t mode>
void posix_file_handler<mode>::read(offset_t offset, size_t length, std::uint8_t* into) const {
    size_t count = 0;
    while (count < length) {
        auto ret = pread(fd, into + count, length - count, offset + count);

        if (ret == 0) {
            throw std::logic_error{"Premature end of file"};
        } else if (ret < 0) {
            throw std::system_error{errno, std::system_category(), "pread"};
        }

        count += ret;
    }
}

template <>
inline void posix_file_handler<file_mode_t::READ_APPEND>::write(offset_t offset,
                                                                size_t length,
                                                                const std::uint8_t* from) {
    size_t count = 0;
    while (count < length) {
        ssize_t ret;
        ret = pwrite(fd, from + count, length - count, offset + count);
        if (ret < 0) {
            throw std::system_error{errno, std::system_category(), "pwrite"};
        }

        count += ret;
    }
}

template <file_mode_t mode>
auto posix_file_handler<mode>::mmap(std::size_t size) -> buffer_type {
    if (size == 0) {
        return nullptr;
    }

    const auto mmap_prot = mode == file_mode_t::READ_ONLY ? PROT_READ : (PROT_WRITE | PROT_READ);
    const auto mmap_flags = mode == file_mode_t::READ_ONLY ? MAP_PRIVATE : MAP_SHARED;

    auto buf = ::mmap(nullptr, size, mmap_prot, mmap_flags, fd, 0);
    if (buf == MAP_FAILED) {
        throw std::system_error{errno, std::system_category(), "mmap"};
    }
    return static_cast<buffer_type>(buf);
}

template <file_mode_t mode>
void posix_file_handler<mode>::munmap(const buffer_type buffer, std::size_t size) const {
    if (!buffer) {
        return;
    }

    if (::munmap(const_cast<std::uint8_t*>(buffer), size)) {
        throw std::system_error{errno, std::system_category(), "munmap"};
    }
}

template <file_mode_t mode>
auto posix_file_handler<mode>::mremap(const buffer_type buffer,
                                      std::size_t old_size,
                                      std::size_t new_size) -> buffer_type {
#ifdef HAVE_MREMAP
    if (!buffer) {
        assert(old_size == 0);
        return mmap(new_size);
    } else {
        auto buf = ::mremap(buffer, old_size, new_size, MREMAP_MAYMOVE);
        if (buf == MAP_FAILED) {
            throw std::system_error{errno, std::system_category(), "mremap"};
        }
        return static_cast<buffer_type>(buf);
    }
#else
    munmap(buffer, old_size);
    return mmap(new_size);
#endif
}

template <>
inline void posix_file_handler<file_mode_t::READ_APPEND>::expand(std::size_t current_size,
                                                                 std::size_t expand_by) {
#if defined(HAVE_FALLOCATE)
    if (fallocate(fd, 0, current_size, expand_by) != 0) {
        throw std::system_error{errno, std::system_category(), "fallocate"};
    }
#elif defined(HAVE_POSIX_FALLOCATE)
    if (auto ret = posix_fallocate(fd, current_size, expand_by)) {
        throw std::system_error{ret, std::system_category(), "posix_fallocate"};
    }
#elif defined(HAVE_F_PREALLOCATE)
    fstore_t store{F_ALLOCATECONTIG, F_PEOFPOSMODE, 0, static_cast<off_t>(expand_by), 0};
    if (fcntl(fd, F_PREALLOCATE, &store) == -1) {
        throw std::system_error{errno, std::system_category(), "fcntl"};
    }
#else
    char buf[] = {0};
    switch (auto ret = pwrite(fd, buf, 1, current_size + expand_by)) {
    case 1:
        return;
    case -1:
        throw std::system_error{errno, std::system_category(), "pwrite"};
    default:
        throw std::runtime_error{"Write returned " + std::to_string(ret)};
    }
#endif
}

template <>
inline void posix_file_handler<file_mode_t::READ_APPEND>::truncate(std::size_t new_size) {
    if (ftruncate(fd, new_size) != 0) {
        throw std::system_error{errno, std::system_category(), "ftruncate"};
    }
}
}
