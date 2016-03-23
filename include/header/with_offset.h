#pragma once

#include "common.h"
#include "utils.h"

namespace protostream {
namespace detail {

/** A wrapper around the type `T`, enriching it with offset information. */
template <class T, offset_t Offset>
struct with_offset {
    using type = T;
    static constexpr offset_t offset = Offset;
    static constexpr size_t size = sizeof(type);

    type value;

    constexpr with_offset(type value = 0) : value{value} {
    }

    constexpr operator type() const {
        return value;
    }

    static with_offset read(const std::uint8_t* buffer) {
        return readbuf_aligned<type>(buffer + offset);
    }

    template <class Backend>
    static with_offset read(const Backend& backend, offset_t file_offset) {
        return backend.template read_num<type>(file_offset + offset);
    }

    template <class... Args>
    void read_self(Args&&... args) {
        value = with_offset::read(std::forward<Args>(args)...);
    }

    void write(uint8_t* buffer) const {
        writebuf(buffer + offset, value);
    }

    template <class Backend>
    void write(Backend& backend, offset_t file_offset) const {
        backend.write_num(file_offset + offset, value);
    }

    constexpr bool operator==(const with_offset& that) const {
        return value == that.value;
    }

    constexpr bool operator!=(const with_offset& that) const {
        return value == that.value;
    }
};
}
}