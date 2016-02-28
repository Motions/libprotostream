#ifndef _FILE_HANDLER_H
#define _FILE_HANDLER_H

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
class file_handler {
public:
    const int fd;

    file_handler(const char *path)
            : fd(open(path, detail::open_flags(mode), 0666)) {
        if (fd == -1) {
            throw std::system_error(errno, std::system_category(), "open");
        }

    }

    ~file_handler() {
        close(fd);
    }

    file_handler(const file_handler &) = delete;

    file_handler(file_handler &&) = default;

    file_handler &operator=(const file_handler &) = delete;

    file_handler &operator=(file_handler &&) = default;

    std::size_t size() const {
        struct stat st;

        if (0 != fstat(fd, &st)) {
            throw std::system_error(errno, std::system_category(), "open");
        }

        return st.st_size;
    }
};

}

#endif // _FILE_HANDLER_H
