#pragma once

#include "config.h"
#include "file_backend.h"
#include "posix_file_handler.h"

#include <sys/mman.h>

namespace protostream {

/** A backend using memory-mapped files */
template<file_mode_t mode, size_t ExpansionGranularity = 1024 * 1024 /* bytes */>
class mmap_backend : public file_backend<mmap_backend<mode, ExpansionGranularity>> {
    static constexpr size_t expansion_granularity = ExpansionGranularity;

public:
    /** Raw pointers are returned directly -- no deallocation is needed,
     *  because the whole file is memory-mapped
     */
    using pointer_type = std::uint8_t*;

    mmap_backend(const char* path)
            : file{path}, used_size{file.size()}, file_size{used_size} {
        do_mmap();
    }

    ~mmap_backend() {
        munmap(buffer, file_size);

        if (used_size != file_size) {
            ftruncate(file.fd, used_size);
        }
    }

    mmap_backend(const mmap_backend&) = delete;

    mmap_backend(mmap_backend&&) = default;

    mmap_backend& operator=(const mmap_backend&) = delete;

    mmap_backend& operator=(mmap_backend&&) = default;

    template<class T>
    void read_small(offset_t offset, T* into) const {
        // TODO: should we care about data alignment?
        *into = *reinterpret_cast<T*>(read(offset, sizeof(T)));
    }

    pointer_type read(offset_t offset, size_t length) const {
        assert(offset + length <= used_size);
        (void) length;
        return buffer + offset;
    }

    std::size_t size() const {
        return used_size;
    }

    template<class T>
    void write_small(offset_t offset, T* from) {
        check_expand(offset + sizeof(*from));
        *reinterpret_cast<T*>(buffer + offset) = *from;
    }

    void write(offset_t offset, size_t length, const std::uint8_t* from) {
        check_expand(offset + length);
        memcpy(buffer + offset, from, length);
    }

private:
    posix_file_handler<mode> file;
    std::size_t used_size, file_size;
    std::uint8_t* buffer;

    void check_expand(offset_t new_end);

    void do_mmap() {
        if (file_size == 0) {
            buffer = nullptr;
            return;
        }

        const auto mmap_prot = mode == file_mode_t::READ_ONLY
                               ? PROT_READ : (PROT_WRITE | PROT_READ);
        const auto mmap_flags = mode == file_mode_t::READ_ONLY
                                ? MAP_PRIVATE : MAP_SHARED;

        auto buf = mmap(nullptr, file_size, mmap_prot, mmap_flags, file.fd, 0);
        if (buf == MAP_FAILED) {
            throw std::system_error(errno, std::system_category(), "mmap");
        }
        buffer = static_cast<std::uint8_t*>(buf);
    }

    void do_munmap() {
        if (munmap(buffer, file_size)) {
            throw std::system_error(errno, std::system_category(), "munmap");
        }
        buffer = nullptr;
    }

#ifdef HAVE_MREMAP

    /** Expands the mapping to new_size, sets file_size to new_size */
    void do_mremap(std::size_t new_size) {
        if (file_size == 0) {
            file_size = new_size;
            do_mmap();
        }
        else {
            auto buf = mremap(buffer, file_size, new_size, MREMAP_MAYMOVE);
            if (buf == MAP_FAILED) {
                throw std::system_error(errno, std::system_category(), "mremap");
            }
            buffer = static_cast<std::uint8_t*>(buf);
            file_size = new_size;
        }
    }

#endif
};

template<>
inline void mmap_backend<file_mode_t::READ_APPEND>::check_expand(offset_t new_end) {
    if (new_end < file_size) {
        used_size = std::max(new_end, used_size);
        return;
    }

    auto expand_by = new_end - file_size;

    /* Round up to the nearest multiple of expansion_granularity */
    expand_by += expansion_granularity;
    expand_by /= expansion_granularity;
    expand_by *= expansion_granularity;

    if (auto ret = posix_fallocate(file.fd, file_size, expand_by)) {
        throw std::system_error(ret, std::system_category(), "posix_fallocate");
    }

#ifdef HAVE_MREMAP
    do_mremap(file_size + expand_by);
#else
    do_munmap();
    file_size += expand_by;
    do_mmap();
#endif

    used_size = new_end;
}

}
