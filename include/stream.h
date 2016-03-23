#pragma once

#include "common.h"
#include "header.h"
#include "utils.h"

#include <cassert>

#include <iterator>
#include <memory>
#include <type_traits>

namespace protostream {

namespace detail {
/** A tag used to annotate `with_*` options */
struct constraint {};

/** Merges `with_*` options together */
template <class... Args>
struct options_handler : public Args... {
    static_assert(conjunction<std::is_base_of<constraint, Args>...>::value,
                  "Some arguments are illegal");
};
}

template <class Backend>
struct with_backend : detail::constraint {
    using backend_type = Backend;
};

template <template <class> class Cache>
struct with_cache : detail::constraint {
    template <class Backend>
    using cache_type = Cache<Backend>;
};

/** Sets the proto header factory.
 *
 * The factory must provide the following members:
 *   * using type = ...
 *       representing the return type of the factory
 *   * static type build(pointer_type ptr, std::size_t len)
 *       where pointer_type is defined as backend_type::pointer_type
 */
template <class ProtoHeaderFactory>
struct with_proto_header_factory : detail::constraint {
    using proto_header_factory_type = ProtoHeaderFactory;
};

/** Sets the keyframe factory.
 *  See `with_proto_header_factory` for the requirements to be met.
 */
template <class KeyframeFactory>
struct with_keyframe_factory : detail::constraint {
    using keyframe_factory_type = KeyframeFactory;
};

/** Sets the delta factory.
 *  See `with_proto_header_factory` for the requirements to be met.
 */
template <class DeltaFactory>
struct with_delta_factory : detail::constraint {
    using delta_factory_type = DeltaFactory;
};

template <class Pointer>
struct default_factory {
    using type = std::pair<Pointer, std::size_t>;

    static type build(Pointer ptr, std::size_t length) {
        return std::make_pair(std::move(ptr), length);
    }
};

template <class... Args>
class stream : public detail::options_handler<Args...> {
public:
    using typename detail::options_handler<Args...>::backend_type;
    using cache_type = typename detail::options_handler<Args...>::template cache_type<backend_type>;
    using typename detail::options_handler<Args...>::proto_header_factory_type;
    using typename detail::options_handler<Args...>::keyframe_factory_type;
    using typename detail::options_handler<Args...>::delta_factory_type;
    using pointer_type = typename backend_type::pointer_type;
    using proto_header_type = typename proto_header_factory_type::type;
    using keyframe_type = typename keyframe_factory_type::type;
    using delta_type = typename delta_factory_type::type;

    /** Opens the file and reads the header from it */
    stream(const char* path);

    /** Opens the file and writes a new header to it */
    stream(const char* path,
           std::uint32_t frames_per_kf,
           const void* proto_header,
           std::size_t proto_header_size);

    stream(const stream&) = delete;

    stream(stream&&) = default;

    stream& operator=(const stream&) = delete;

    stream& operator=(stream&&) = default;

    proto_header_type get_proto_header() const {
        const auto size =
            header_field<fields::kf0_offset>() - header_field<fields::proto_header_offset>();
        return proto_header_factory_type::build(
            backend.read(header_field<fields::proto_header_offset>(), size), size);
    }

    class keyframe_iterator;

    class delta_iterator;

    class delta_data {
    public:
        using size_type = delta_size_t;

        delta_type get() const {
            return delta_factory_type::build(raw(), size());
        }

        bool operator==(const delta_data& that) const {
            return &str == &that.str && frame_id == that.frame_id;
        }

        bool operator!=(const delta_data& that) const {
            return !(*this == that);
        }

        size_type size() const {
            return str.backend.template read_num<size_type>(offset);
        }

        pointer_type raw() const {
            return str.backend.read(offset + sizeof(size_type), size());
        }

    private:
        const stream& str;
        offset_t offset;
        keyframe_id_t frame_id;

        /* Represents a past-the-end "delta". Its offset is undefined, only
     * `frame_id`s are used in comparison operators.
     */
        delta_data(const stream& str, keyframe_id_t frame_id)
            : str{str}, offset{std::numeric_limits<offset_t>::max()}, frame_id{frame_id} {
        }

        delta_data(const stream& str, offset_t offset, keyframe_id_t frame_id)
            : str{str}, offset{offset}, frame_id{frame_id} {
        }

        friend class delta_iterator;
    };

    class delta_iterator : public std::iterator<std::forward_iterator_tag, delta_data> {
    public:
        const delta_data& operator*() const {
            return data;
        }

        const delta_data* operator->() const {
            return &data;
        }

        delta_iterator& operator++() {
            data.offset += sizeof(typename delta_data::size_type) + data.size();
            data.frame_id++;
            return *this;
        }

        delta_iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator==(const delta_iterator& that) const {
            return data == that.data;
        }

        bool operator!=(const delta_iterator& that) const {
            return !(*this == that);
        }

    private:
        /* Creates a past-the-end iterator. */
        delta_iterator(const stream& str, keyframe_id_t frame_id) : data{str, frame_id} {
        }

        delta_iterator(const stream& str, offset_t offset, keyframe_id_t frame_id)
            : data{str, offset, frame_id} {
        }

        delta_data data;

        friend class stream::keyframe_data;
    };

    class keyframe_data {
    public:
        keyframe_type get() const {
            return keyframe_factory_type::build(raw(), size());
        }

        delta_iterator begin() const {
            return {str, field<fields::delta_offset>(),
                    id() * str.header_field<fields::frames_per_kf>() + 1};
        }

        delta_iterator end() const {
            return {str, std::min(str.header_field<fields::frame_count>(),
                                  (id() + 1) * str.header_field<fields::frames_per_kf>())};
        }

        bool operator==(const keyframe_data& that) const {
            return &str == &that.str && offset == that.offset;
        }

        bool operator!=(const keyframe_data& that) const {
            return !(*this == that);
        }

        pointer_type raw() const {
            return str.backend.read(offset + reduced_keyframe_header::size, size());
        }

        std::size_t size() const {
            return field<fields::kf_size>();
        }

        keyframe_id_t id() const {
            return field<fields::kf_num>();
        }

    private:
        keyframe_data(const stream& str, offset_t offset) : str{str}, offset{offset} {
        }

        const stream& str;
        offset_t offset;

        auto header() const {
            return str.cache.header_at(offset);
        }

        template <class Field>
        auto field() const {
            return header().template get<Field>();
        }

        friend class stream;

        friend class keyframe_iterator;
    };

    class keyframe_iterator : public std::iterator<std::forward_iterator_tag, keyframe_data> {
    public:
        const keyframe_data& operator*() const {
            assert(data.offset != no_keyframe);
            return data;
        }

        const keyframe_data* operator->() const {
            assert(data.offset != no_keyframe);
            return &data;
        }

        keyframe_iterator& operator++() {
            data.offset = link(0);
            return *this;
        }

        keyframe_iterator operator++(int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        keyframe_iterator& operator+=(keyframe_id_t diff) {
            static_assert(std::is_unsigned<decltype(diff)>{}, "Keyframe diff is not unsigned");

            if (auto offset_opt = data.str.cache.offset_of(data.id() + diff)) {
                data.offset = *offset_opt;
            } else {
                for (auto level = fields::skiplist_height - 1; diff > 0; level--) {
                    while (diff >= (1u << level)) {
                        diff -= 1u << level;
                        data.offset = link(level);
                    }
                }

                assert(diff == 0);
            }

            return *this;
        }

        keyframe_iterator operator+(keyframe_id_t diff) const {
            auto tmp = *this;
            return tmp += diff;
        }

        bool operator==(const keyframe_iterator& that) const {
            return data == that.data;
        }

        bool operator!=(const keyframe_iterator& that) const {
            return !(*this == that);
        }

    private:
        keyframe_iterator(const stream& str, offset_t offset) : data{str, offset} {
        }

        offset_t link(unsigned level) const {
            return data.str.cache.link_at(data.offset, level);
        }

        keyframe_data data;

        friend class stream;
    };

    keyframe_iterator begin() const {
        return {*this, header_field<fields::kf0_offset>()};
    }

    keyframe_iterator end() const {
        return {*this, 0};
    }

    void append_delta(const std::uint8_t* data, delta_size_t size) {
        assert(header_field<fields::frame_count>() % header_field<fields::frames_per_kf>() != 0);

        const auto offset = backend.size();
        backend.write_num(offset, size);
        backend.write(offset + sizeof(delta_size_t), size, data);

        header_field<fields::frame_count>()++;
        header_field<fields::file_size>() += sizeof(delta_size_t) + size;
        header.write(backend, 0);
    }

    void append_keyframe(const std::uint8_t* data, std::size_t size) {
        assert(header_field<fields::frame_count>() % header_field<fields::frames_per_kf>() == 0);

        const auto offset = backend.size();
        const auto id = header_field<fields::keyframe_count>();

        reduced_keyframe_header hdr;
        hdr.get<fields::kf_num>() = id;
        hdr.get<fields::delta_offset>() = offset + reduced_keyframe_header::size + size;
        hdr.get<fields::kf_size>() = size;
        hdr.write(backend, offset);

        backend.write(offset + reduced_keyframe_header::size, size, data);

        update_links_to(id, offset);

        header_field<fields::frame_count>()++;
        header_field<fields::keyframe_count>()++;
        header_field<fields::file_size>() += reduced_keyframe_header::size + size;

        header.write(backend, 0);
    }

    std::size_t frame_count() const {
        return header_field<fields::frame_count>();
    }

    std::size_t keyframe_count() const {
        return header_field<fields::keyframe_count>();
    }

private:
    backend_type backend;
    mutable cache_type cache;
    file_header header;

    template <class Field>
    auto header_field() const {
        return header.template get<Field>();
    }

    template <class Field>
    auto& header_field() {
        return header.template get<Field>();
    }

    void update_links_to(keyframe_id_t keyframe_id, offset_t offset) {
        auto level = static_cast<int>(fields::skiplist_height - 1);
        while (level >= 0 && keyframe_id < (1u << level)) {
            --level;
        }

        if (level < 0) {
            return;
        }

        auto it = begin() + (keyframe_id - (1u << level));
        while (level >= 0) {
            backend.write_num(it.data.offset + fields::skiplist_offset(level), offset);

            level--;
            if (level >= 0) {
                it += 1 << level;
            }
        }
    }
};

template <class... Args>
stream<Args...>::stream(const char* path)
    : backend{path}, cache{backend} {
    const auto file_size = backend.size();

    if (file_size < file_header::size) {
        throw std::runtime_error{"File too small"};
    }

    header = file_header::read(backend, 0);

    if (header_field<fields::file_size>() != file_size) {
        throw std::runtime_error{"File size not consistent with data in header"};
    }

    if (header_field<fields::proto_header_offset>() > file_size - file_header::size) {
        throw std::runtime_error{"Invalid proto header offset"};
    }

    if (header_field<fields::kf0_offset>() > file_size - file_header::size) {
        throw std::runtime_error{"Invalid keyframe 0 offset"};
    }

    if (header_field<fields::proto_header_offset>() > header_field<fields::kf0_offset>()) {
        throw std::runtime_error{"Proto header is placed after keyframe 0"};
    }
}

template <class... Args>
stream<Args...>::stream(const char* path,
                        std::uint32_t frames_per_kf,
                        const void* proto_header,
                        std::size_t proto_header_size)
    : backend{path}, cache{backend} {
    auto end = file_header::size + proto_header_size;

    header_field<fields::file_size>() = end;
    header_field<fields::kf0_offset>() = end;
    header_field<fields::proto_header_offset>() = file_header::size;
    header_field<fields::frames_per_kf>() = frames_per_kf;
    header.write(backend, 0);

    backend.write(file_header::size, proto_header_size,
                  static_cast<const std::uint8_t*>(proto_header));
}

template <class... Args>
auto begin(const stream<Args...>& stream) {
    return stream.begin();
}

template <class... Args>
auto end(const stream<Args...>& stream) {
    return stream.end();
}
}
