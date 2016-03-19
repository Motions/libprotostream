#pragma once

#include "common.h"
#include "utils.h"

#include <cstring>

#include <stdexcept>

namespace protostream {
namespace detail {

/** A simple magic value checker following the API of `with_offset` */
template<class Derived, offset_t Offset>
struct magic_value {
    static constexpr offset_t offset = Offset;

    /** The magic value itself is passed as the return value of a static constexpr method
     *  of the Derived subclass. It is required to avoid clashes with the One Definition Rule
     *  which severely restricts the other possibilities of passing a constexpr string literal
     *  in a header-only library such as protostream:
     *      * a static data member must be also defined/declared out-of-class
     *      * passing the literal as a const char* template parameter technically means that
     *        the deriving class may have a different base (magic_value<...>) in every translation
     *        unit, because the address of the string literal may vary. Although this code
     *        will probably behave correctly nevertheless, the standard declares the behaviour
     *        undefined.
     */

    static magic_value read(const std::uint8_t* buffer) {
        if (0 != memcmp(buffer + offset, Derived::magic(), Derived::size)) {
            throw std::logic_error("Invalid magic value");
        }
        return {};
    }

    template<class Backend>
    static magic_value read(const Backend& backend, offset_t file_offset) {
        auto buffer = backend.read(file_offset + offset, Derived::size);
        return read(as_ptr(buffer));
    }

    template<class... Args>
    void read_self(Args&& ... args) {
        read(std::forward<Args>(args)...);
    }

    void write(uint8_t* buffer) const {
        memcpy(buffer + offset, Derived::magic(), Derived::size);
    }

    template<class Backend>
    void write(Backend& backend, offset_t file_offset) const {
        backend.write(file_offset + offset, Derived::size,
                      reinterpret_cast<const std::uint8_t*>(Derived::magic()));
    }
};

}
}