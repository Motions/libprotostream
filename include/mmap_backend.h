#pragma once

#include "file_backend.h"
#include "mmap_guard.h"
#include "posix_file_handler.h"

namespace protostream {

/** A backend using memory-mapped files */
template <file_mode_t mode, size_t ExpansionGranularity = 1024 * 1024 /* bytes */>
class mmap_backend : public file_backend<mmap_backend<mode, ExpansionGranularity>> {
    static constexpr size_t expansion_granularity = ExpansionGranularity;

public:
    /** Raw pointers are returned directly -- no deallocation is needed,
   *  because the whole file is memory-mapped
   */
    using pointer_type = const std::uint8_t*;

    mmap_backend(const char* path) : file{path}, used_size{file.size()}, buffer{file} {
    }

    /** If an unrecoverable system error occurs this destructor WILL throw an
   * exception */
    ~mmap_backend() noexcept(false);

    mmap_backend(const mmap_backend&) = delete;

    mmap_backend(mmap_backend&&) = default;

    mmap_backend& operator=(const mmap_backend&) = delete;

    mmap_backend& operator=(mmap_backend&&) = default;

    template <class T>
    void read_small(offset_t offset, T* into) const {
        // TODO: should we care about data alignment?
        *into = *reinterpret_cast<const T*>(read(offset, sizeof(T)));
    }

    pointer_type read(offset_t offset, size_t length) const {
        assert(offset + length <= used_size);
        (void)length;
        return buffer.get() + offset;
    }

    std::size_t size() const {
        return used_size;
    }

    template <class T>
    void write_small(offset_t offset, const T* from) {
        check_expand(offset + sizeof(*from));
        *reinterpret_cast<T*>(buffer.get() + offset) = *from;
    }

    void write(offset_t offset, size_t length, const std::uint8_t* from) {
        check_expand(offset + length);
        memcpy(buffer.get() + offset, from, length);
    }

private:
    posix_file_handler<mode> file;
    std::size_t used_size;
    mmap_guard<posix_file_handler<mode>> buffer;

    void check_expand(std::size_t new_end);
};

template <>
inline mmap_backend<file_mode_t::READ_ONLY>::~mmap_backend() noexcept(false) {
}

template <>
inline mmap_backend<file_mode_t::READ_APPEND>::~mmap_backend() noexcept(false) {
    if (used_size != buffer.size()) {
        file.truncate(used_size);
    }
}

template <>
inline void mmap_backend<file_mode_t::READ_APPEND>::check_expand(std::size_t new_end) {
    if (new_end <= buffer.size()) {
        used_size = std::max(new_end, used_size);
        return;
    }

    auto expand_by = new_end - buffer.size();

    /* Round up to the nearest multiple of expansion_granularity */
    expand_by += expansion_granularity - 1;
    expand_by /= expansion_granularity;
    expand_by *= expansion_granularity;

    file.expand(buffer.size(), expand_by);
    buffer.mremap(buffer.size() + expand_by);
    used_size = new_end;
}
}
