#pragma once

#include "common.h"

namespace protostream {
namespace detail {

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