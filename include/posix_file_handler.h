#pragma once

#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <system_error>

namespace protostream {

enum class file_mode_t {
    READ_ONLY, READ_APPEND
};

namespace detail {
constexpr int open_flags(file_mode_t mode) {
    return mode == file_mode_t::READ_ONLY
           ? O_RDONLY
           : O_RDWR | O_CREAT | O_EXCL;
}
}

/** A RAII wrapper around open files */
template<file_mode_t mode>
class posix_file_handler {
public:
    const int fd;

    posix_file_handler(const char* path)
            : fd(open(path, detail::open_flags(mode), 0666)) {
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

    std::size_t size() const {
        struct stat st;

        if (fstat(fd, &st) != 0) {
            throw std::system_error{errno, std::system_category(), "fstat"};
        }

        return st.st_size;
    }
};

}
