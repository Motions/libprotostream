#pragma once

#include <cstdint>

namespace protostream {

template <class Handler>
class mmap_guard {
public:
    using buffer_type = typename Handler::buffer_type;

    mmap_guard(Handler& handler) : mmap_guard{handler, handler.size()} {
    }

    mmap_guard(Handler& handler, std::size_t size)
        : handler{handler}, bufsize{size}, buffer{handler.mmap(size)} {
    }

    mmap_guard(const mmap_guard&) = delete;

    mmap_guard(mmap_guard&&) = default;

    mmap_guard& operator=(const mmap_guard&) = delete;

    mmap_guard& operator=(mmap_guard&&) = default;

    ~mmap_guard() {
        handler.munmap(buffer, bufsize);
    }

    void mremap(std::size_t new_size) {
        buffer = handler.mremap(buffer, bufsize, new_size);
        bufsize = new_size;
    }

    std::size_t size() const {
        return bufsize;
    }

    buffer_type get() const {
        return buffer;
    }

private:
    Handler& handler;
    std::size_t bufsize;
    buffer_type buffer;
};
}