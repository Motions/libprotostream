#ifndef _HEADER_H
#define _HEADER_H

#include "common.h"
#include "utils.h"

#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace protostream {

namespace detail {

/** A wrapper around the type `T`, enriching it with offset information. */
template<class T, offset_t Offset>
struct with_offset {
    using type = T;
    static constexpr offset_t offset = Offset;
    static constexpr size_t size = sizeof(type);

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
    static constexpr std::size_t size = MagicSize;

    static magic_value read(const std::uint8_t *buffer) {
        if (0 != memcmp(buffer + offset, magic, size)) {
            throw std::logic_error("Invalid magic value");
        }
        return {};
    }

    template<class Backend>
    static magic_value read(const Backend &backend, offset_t file_offset) {
        auto buffer = backend.read(file_offset + offset, size);
        return read(as_ptr(buffer));
    }

    template<class... Args>
    void read_self(Args &&... args) {
        read(std::forward<Args>(args)...);
    }

    void write(uint8_t *buffer) const {
        memcpy(buffer + offset, magic, size);
    }

    template<class Backend>
    void write(Backend &backend, offset_t file_offset) const {
        backend.write(file_offset + offset, size, reinterpret_cast<const std::uint8_t *>(magic));
    }
};

template<std::size_t Size, offset_t Offset>
struct placeholder {
    static constexpr std::size_t size = Size;
    static constexpr offset_t offset = Offset;

    template<class... Args>
    static placeholder read(Args &&...) {
        return {};
    }

    template<class... Args>
    void read_self(Args &&...) const {
    }

    template<class... Args>
    void write(Args &&...) const {
    }
};

namespace overlap_check {

template<class Field1, class Field2>
struct check_pair {
    static_assert(!(Field1::offset <= Field2::offset &&
                    Field1::offset + Field1::size > Field2::offset), "fields overlap");
};

template<class Field>
struct check_pair<Field, Field> {
};

template<class Field, class... Fields>
struct check_one_to_many : check_pair<Field, Fields> ... {
};

template<class... Fields>
struct check_all_pairs : check_one_to_many<Fields, Fields...> ... {
};

}

template<class Derived, class... Fields>
class generic_header : overlap_check::check_all_pairs<Fields...> {
public:
    static constexpr size_t size = std::max({(Fields::offset + Fields::size)...});

    template<class... Args>
    void read_self(Args &&... args) {
        int _[] = {(std::get<Fields>(fields).read_self(args...), 0)...};
        (void) _;
    }

    template<class... Args>
    static Derived read(Args &&... args) {
        Derived result;
        result.read_self(std::forward<Args>(args)...);
        return result;
    }

    template<class... Args>
    void write(Args &&... args) const {
        int _[] = {(std::get<Fields>(fields).write(args...), 0)...};
        (void) _;
    }

    template<class Field>
    auto get() const {
        return std::get<Field>(fields).value;
    }

    template<class Field>
    auto &get() {
        return std::get<Field>(fields).value;
    }

private:
    std::tuple<Fields...> fields;
};

}

namespace fields {

inline namespace file_header {

static constexpr const char magic_value[] = "PROTOSTR";
static constexpr std::size_t magic_size = sizeof(magic_value) - 1;
static_assert(magic_size == 8, "Invalid magic size");

struct magic_field : public detail::magic_value<magic_value, magic_size, 0> {
};
struct file_size : public detail::with_offset<offset_t, 8> {
};
struct proto_header_offset : public detail::with_offset<offset_t, 8 * 2> {
};
struct kf0_offset : public detail::with_offset<offset_t, 8 * 3> {
};
struct keyframe_count : public detail::with_offset<offset_t, 8 * 4> {
};
struct frame_count : public detail::with_offset<offset_t, 8 * 5> {
};
struct frames_per_kf : public detail::with_offset<uint32_t, 8 * 6> {
};

}

inline namespace reduced_keyframe_header {

static constexpr unsigned skiplist_height = 10;

static constexpr offset_t skiplist_offset(unsigned level = 0) {
    return sizeof(offset_t) * 2 + sizeof(offset_t) * level;
}

struct kf_num : public detail::with_offset<offset_t, 0> {
};
struct delta_offset : public detail::with_offset<offset_t, 8> {
};
struct skiplist_placeholder : public detail::placeholder<8 * skiplist_height, 8 * 2> {
};
struct kf_size : public detail::with_offset<uint32_t, 8 * 2 + 8 * skiplist_height> {
};

}

}

struct file_header : public detail::generic_header<file_header,
        fields::file_header::magic_field,
        fields::file_header::file_size,
        fields::file_header::proto_header_offset,
        fields::file_header::kf0_offset,
        fields::file_header::keyframe_count,
        fields::file_header::frame_count,
        fields::file_header::frames_per_kf
> {
};

struct reduced_keyframe_header : public detail::generic_header<reduced_keyframe_header,
        fields::reduced_keyframe_header::kf_num,
        fields::reduced_keyframe_header::delta_offset,
        fields::reduced_keyframe_header::skiplist_placeholder,
        fields::reduced_keyframe_header::kf_size
> {
};

}

#endif // _HEADER_H
