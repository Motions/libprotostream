#pragma once

#include "common.h"
#include "utils.h"

namespace protostream {

/** A low-level file backend (CRTP base for mmap_backend and
 * posix_file_backend).
 *
 * The CRTP derived class (the Self template parameter) must provide the
 * following members:
 *
 *   * using pointer_type = ...
 *       a pointer type (smart or not) which is returned by `read`.
 *   * template<class T> void read_small(offset_t offset, T* into) const
 *       reads sizeof(T) bytes from position `offset` into `into`
 *   * pointer_type read(offset_t offset, size_t length) const
 *       reads `length` bytes starting from `offset`
 *   * template<class T> void write_small(offset_t offset, T* from)
 *       writes sizeof(T) bytes from `from` into the file starting from `offset`
 *   * void write(offset_t offset, size_t length, const std::uint8_t* from)
 *       writes `length` bytes from `from` into the file starting from `offset`
 *
 *  Note: the latter two members are only required if the backend is not
 * read-only.
 */
template <class Derived>
struct file_backend {
    /** Reads an integer from position `offset` and converts it to the native
   * byte
   * order */
    template <class T>
    T read_num(offset_t offset) const {
        T value;
        self()->read_small(offset, &value);
        return detail::betoh(value);
    }

    /** Writes an integral `value` into the file starting from `offset`
   * (converting it previously to the big-endian byte order)
   */
    template <class T>
    void write_num(offset_t offset, T value) {
        value = detail::htobe(value);
        self()->write_small(offset, &value);
    }

private:
    constexpr Derived* self() {
        return static_cast<Derived*>(this);
    }

    constexpr const Derived* self() const {
        return static_cast<const Derived*>(this);
    }
};
}
