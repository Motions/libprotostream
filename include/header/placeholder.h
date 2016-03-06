#pragma once

#include "common.h"

namespace protostream {
namespace detail {

/** Represents a "hole" in a header, i.e. bytes which are neither read from nor written to
 *  by the `generic_header` class.
 */
template<std::size_t Size, offset_t Offset>
struct placeholder {
    static constexpr std::size_t size = Size;
    static constexpr offset_t offset = Offset;

    template<class... Args>
    static placeholder read(Args&& ...) {
        return {};
    }

    template<class... Args>
    void read_self(Args&& ...) const { }

    template<class... Args>
    void write(Args&& ...) const { }
};

}
}
