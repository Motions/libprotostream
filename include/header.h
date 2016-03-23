#pragma once

#include "common.h"
#include "header/generic_header.h"
#include "header/magic_value.h"
#include "header/placeholder.h"
#include "header/with_offset.h"

namespace protostream {

namespace fields {
inline namespace file_header {
struct magic_field : public detail::magic_value<magic_field, 0> {
    static constexpr const char* magic() {
        return "PROTOSTR";
    }

    static constexpr std::size_t size = 8;
};
struct file_size : public detail::with_offset<offset_t, 8> {};
struct proto_header_offset : public detail::with_offset<offset_t, 8 * 2> {};
struct kf0_offset : public detail::with_offset<offset_t, 8 * 3> {};
struct keyframe_count : public detail::with_offset<offset_t, 8 * 4> {};
struct frame_count : public detail::with_offset<offset_t, 8 * 5> {};
struct frames_per_kf : public detail::with_offset<uint32_t, 8 * 6> {};
struct reserved : public detail::placeholder<4, 8 * 6 + 4> {};
}
}

struct file_header : public detail::generic_header<file_header,
                                                   fields::file_header::magic_field,
                                                   fields::file_header::file_size,
                                                   fields::file_header::proto_header_offset,
                                                   fields::file_header::kf0_offset,
                                                   fields::file_header::keyframe_count,
                                                   fields::file_header::frame_count,
                                                   fields::file_header::frames_per_kf,
                                                   fields::file_header::reserved> {};

namespace fields {
inline namespace reduced_keyframe_header {
static constexpr unsigned skiplist_height = 10;

static constexpr offset_t skiplist_offset(unsigned level = 0) {
    return 8 * 2 + 8 * level;
}

struct kf_num : public detail::with_offset<offset_t, 0> {};
struct delta_offset : public detail::with_offset<offset_t, 8> {};
struct skiplist_placeholder : public detail::placeholder<8 * skiplist_height, 8 * 2> {};
struct kf_size : public detail::with_offset<uint32_t, 8 * 2 + 8 * skiplist_height> {};
}
}

struct reduced_keyframe_header
    : public detail::generic_header<reduced_keyframe_header,
                                    fields::reduced_keyframe_header::kf_num,
                                    fields::reduced_keyframe_header::delta_offset,
                                    fields::reduced_keyframe_header::skiplist_placeholder,
                                    fields::reduced_keyframe_header::kf_size> {};
}
