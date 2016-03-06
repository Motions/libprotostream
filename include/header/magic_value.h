#pragma once

#include "common.h"
#include "utils.h"

#include <cstring>

#include <stdexcept>

namespace protostream {
namespace detail {

/** A simple magic value checker following the API of `with_offset` */
template<const char* Magic, std::size_t MagicSize, offset_t Offset>
struct magic_value {
    static constexpr offset_t offset = Offset;
    static constexpr const char* magic = Magic;
    static constexpr std::size_t size = MagicSize;

    static magic_value read(const std::uint8_t* buffer) {
        if (0 != memcmp(buffer + offset, magic, size)) {
            throw std::logic_error("Invalid magic value");
        }
        return {};
    }

    template<class Backend>
    static magic_value read(const Backend& backend, offset_t file_offset) {
        auto buffer = backend.read(file_offset + offset, size);
        return read(as_ptr(buffer));
    }

    template<class... Args>
    void read_self(Args&& ... args) {
        read(std::forward<Args>(args)...);
    }

    void write(uint8_t* buffer) const {
        memcpy(buffer + offset, magic, size);
    }

    template<class Backend>
    void write(Backend& backend, offset_t file_offset) const {
        backend.write(file_offset + offset, size, reinterpret_cast<const std::uint8_t*>(magic));
    }
};

}
}