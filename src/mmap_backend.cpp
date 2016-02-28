#include "mmap_backend.h"

#include <sys/mman.h>

#include <system_error>

namespace protostream {

mmap_backend::mmap_backend(const char *path)
        : file{path}, buffer_size{file.size()},
          buffer{static_cast<uint8_t *>(mmap(nullptr, buffer_size, PROT_READ, MAP_PRIVATE, file.fd, 0))} {
    if (buffer == MAP_FAILED) {
        throw std::system_error(errno, std::system_category(), "mmap");
    }
}

mmap_backend::~mmap_backend() {
    munmap(buffer, buffer_size);
}

}
