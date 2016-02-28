#ifndef _HEADER_H
#define _HEADER_H

#include "common.h"
#include "utils.h"

#include <stdexcept>

namespace protostream {

namespace detail {

/** A wrapper around the type `T`, enriching it with offset information. */
template<class T, offset_t Offset>
struct with_offset {
    using type = T;
    static constexpr offset_t offset = Offset;

    type value;

    constexpr with_offset(type value = 0) : value{value} { }

    constexpr operator type() const {
        return value;
    }

    static with_offset read(const std::uint8_t *buffer) {
        return readbuf_aligned<type>(buffer + offset);
    }

    template<class Backend>
    static with_offset read(const Backend &backend, offset_t file_offset) {
        return backend.template read_num<type>(file_offset + offset);
    }

    template<class... Args>
    void read_self(Args &&... args) {
        value = with_offset::read(std::forward<Args>(args)...);
    }

    void write(uint8_t *buffer) const {
        writebuf(buffer + offset, value);
    }

    template<class Backend>
    void write(Backend &backend, offset_t file_offset) const {
        backend.write_num(file_offset + offset, value);
    }
};

/** A simple magic value checker following the API of `with_offset` */
template<const char *Magic, std::size_t MagicSize, offset_t Offset>
struct magic_value {
    static constexpr offset_t offset = Offset;
    static constexpr const char *magic = Magic;
    static constexpr std::size_t magic_size = MagicSize;

    static magic_value read(const std::uint8_t *buffer) {
        if (0 != memcmp(buffer + offset, magic, magic_size)) {
            throw std::logic_error("Invalid magic value");
        }
        return {};
    }

    template<class Backend>
    static magic_value read(const Backend &backend, offset_t file_offset) {
        auto buffer = backend.read(file_offset + offset, magic_size);
        return read(as_ptr(buffer));
    }

    template<class... Args>
    static magic_value read_self(Args &&... args) {
        return read(std::forward<Args>(args)...);
    }

    void write(uint8_t *buffer) const {
        memcpy(buffer + offset, magic, magic_size);
    }

    template<class Backend>
    void write(Backend &backend, offset_t file_offset) const {
        backend.write(file_offset + offset, magic_size, reinterpret_cast<const std::uint8_t *>(magic));
    }
};

}

struct file_header {
    static constexpr const char magic_value[] = "PROTOSTR";
    static constexpr std::size_t magic_size = sizeof(magic_value) - 1;
    static_assert(magic_size == 8, "Invalid magic size");

    static constexpr std::size_t size = magic_size + sizeof(offset_t) * 5 + sizeof(uint32_t);

    detail::magic_value<magic_value, magic_size, 0> magic_field;
    detail::with_offset<offset_t, 8> file_size;
    detail::with_offset<offset_t, 8 * 2> proto_header_offset;
    detail::with_offset<offset_t, 8 * 3> kf0_offset;
    detail::with_offset<offset_t, 8 * 4> keyframe_count;
    detail::with_offset<offset_t, 8 * 5> frame_count;
    detail::with_offset<uint32_t, 8 * 6> frames_per_kf;

    template<class... Args>
    static file_header read(Args &&... args);

    template<class... Args>
    void write(Args &&... args) const;
};

constexpr const char file_header::magic_value[];

template<class... Args>
inline file_header file_header::read(Args &&... args) {
    file_header result;

    result.magic_field.read_self(args...);
    result.file_size.read_self(args...);
    result.proto_header_offset.read_self(args...);
    result.kf0_offset.read_self(args...);
    result.keyframe_count.read_self(args...);
    result.frame_count.read_self(args...);
    result.frames_per_kf.read_self(args...);

    return result;
}

template<class... Args>
inline void file_header::write(Args &&... args) const {
    magic_field.write(args...);
    file_size.write(args...);
    proto_header_offset.write(args...);
    kf0_offset.write(args...);
    keyframe_count.write(args...);
    frame_count.write(args...);
    frames_per_kf.write(args...);
}

struct reduced_keyframe_header {
    static constexpr unsigned skiplist_height = 10;
    static constexpr std::size_t size = sizeof(offset_t) * (2 + skiplist_height) + sizeof(uint32_t);

    detail::with_offset<offset_t, 0> kf_num;
    detail::with_offset<offset_t, 8> delta_offset;
    detail::with_offset<uint32_t, 8 * 2 + 8 * skiplist_height> kf_size;

    static constexpr offset_t skiplist_offset(unsigned level = 0) {
        return sizeof(offset_t) * 2 + sizeof(offset_t) * level;
    }

    template<class... Args>
    static reduced_keyframe_header read(Args &&... args);

    template<class... Args>
    void write(Args &&... args) const;
};

template<class... Args>
inline reduced_keyframe_header reduced_keyframe_header::read(Args &&... args) {
    reduced_keyframe_header result;

    result.kf_num.read_self(args...);
    result.delta_offset.read_self(args...);
    result.kf_size.read_self(args...);

    return result;
}

template<class... Args>
inline void reduced_keyframe_header::write(Args &&... args) const {
    kf_num.write(args...);
    delta_offset.write(args...);
    kf_size.write(args...);
}

}

#endif // _HEADER_H
