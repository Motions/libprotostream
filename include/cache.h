#pragma once

#include "header.h"

#include <experimental/optional>
#include <unordered_map>

namespace protostream {

/** A CRTP base class for caches
 *
 * `Derived` is the CRTP derived class
 * `Backend` is the backend this cache will operate on
 *
 *  The derived class must provide the following member:
 *    * reduced_keyframe_header header_at(offset_t offset)
 *        returns the header at offset `offset`, possibly a cached one
 */
template <class Derived, class Backend>
class cache_base {
public:
    /** Returns the offset of the given keyframe, if known */
    std::experimental::optional<offset_t> offset_of(keyframe_id_t keyframe_id) const {
        const auto it = offsets.find(keyframe_id);
        if (it == offsets.end()) {
            return {};
        } else {
            return {it->second};
        }
    }

    /** Returns the `level`th link in the skiplist associated with the keyframe
   * with offset `offset */
    offset_t link_at(offset_t offset, unsigned level) {
        assert(level < fields::skiplist_height);
        assert(offset != no_keyframe);

        const auto& header = self()->header_at(offset);
        const auto kf_num = header.template get<fields::kf_num>();

        const auto it = offsets.find(kf_num + (1 << level));
        if (it != offsets.end()) {
            return it->second;
        }

        offsets.emplace(kf_num, offset);

        const auto ptr = backend.read(offset + fields::skiplist_offset(),
                                      sizeof(offset_t) * fields::skiplist_height);

        const auto skiplist = reinterpret_cast<const offset_t*>(detail::as_ptr(ptr));

        for (auto i = 0u; i < fields::skiplist_height; ++i) {
            const auto link_offset = detail::betoh<offset_t>(skiplist[i]);

            if (link_offset != no_keyframe) {
                if (link_offset <= offset) {
                    throw std::runtime_error{"Back link found"};
                }

                offsets.emplace(kf_num + (1 << i), link_offset);
            }
        }

        return detail::betoh<offset_t>(skiplist[level]);
    }

protected:
    explicit cache_base(Backend& backend) : backend{backend} {
    }

    /** Returns the keyframe header read from the backend */
    reduced_keyframe_header retrieve(offset_t offset) const {
        return reduced_keyframe_header::read(backend, offset);
    }

private:
    Backend& backend;
    std::unordered_map<keyframe_id_t, offset_t> offsets;

    Derived* self() {
        return static_cast<Derived*>(this);
    }
};

/** A simple cache, caching only the offsets (i.e. using only the features
 * provided by cache_base) */
template <class Backend>
class offsets_only_cache : public cache_base<offsets_only_cache<Backend>, Backend> {
    using base = cache_base<offsets_only_cache<Backend>, Backend>;

protected:
    using base::retrieve;

public:
    explicit offsets_only_cache(Backend& backend) : base{backend} {
    }

    reduced_keyframe_header header_at(offset_t offset) const {
        return retrieve(offset);
    }
};

/** Caches not only the keyframe offsets, but also the keyframe headers. */
template <class Backend>
class full_cache : public cache_base<full_cache<Backend>, Backend> {
    using base = cache_base<full_cache<Backend>, Backend>;

protected:
    using base::retrieve;

public:
    explicit full_cache(Backend& backend) : base{backend} {
    }

    reduced_keyframe_header header_at(offset_t offset) {
        auto it = headers.find(offset);
        if (it != headers.end()) {
            return it->second;
        } else {
            auto header = retrieve(offset);
            headers.emplace(offset, header);
            return header;
        }
    }

private:
    std::unordered_map<offset_t, reduced_keyframe_header> headers;
};
}
