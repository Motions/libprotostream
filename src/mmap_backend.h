#pragma once

#include "file_backend.h"
#include "posix_file_handler.h"

namespace protostream {

/** A backend using memory-mapped files */
class mmap_backend : public file_backend<mmap_backend> {
public:
    /** Raw pointers are returned directly -- no deallocation is needed,
     *  because the whole file is memory-mapped
     */
    using pointer_type = std::uint8_t*;

    mmap_backend(const char* path);

    ~mmap_backend();

    mmap_backend(const mmap_backend&) = delete;

    mmap_backend(mmap_backend&&) = default;

    mmap_backend& operator=(const mmap_backend&) = delete;

    mmap_backend& operator=(mmap_backend&&) = default;

    template<class T>
    void read_small(offset_t offset, T* into) const {
        // TODO: should we care about data alignment?
        *into = *reinterpret_cast<T*>(buffer + offset);
    }

    pointer_type read(offset_t offset, size_t length) const {
#ifdef DEBUG
        assert(offset + length <= buffer_size);
#else
        (void) length;
#endif
        return buffer + offset;
    }

    std::size_t size() const {
        return buffer_size;
    }

private:
    posix_file_handler <file_mode_t::READ_ONLY> file;
    std::size_t buffer_size;
    std::uint8_t* buffer;
};

}