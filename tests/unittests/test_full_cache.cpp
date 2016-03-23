#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cache.h"
#include "header.h"

#include "cache_test_base.h"

struct full_cache : public cache_test_base<protostream::full_cache> {};

TEST_F(full_cache, read_header) {
    constexpr offset_t keyframe_offset = 404;

    expect_field_read<protostream::fields::kf_num>(keyframe_offset);
    expect_field_read<protostream::fields::delta_offset>(keyframe_offset);
    expect_field_read<protostream::fields::kf_size>(keyframe_offset);

    for (auto iter = 0; iter < 100; ++iter) {
        EXPECT_EQ(header, cache->header_at(keyframe_offset));
    }
}

TEST_F(full_cache, links) {
    constexpr offset_t keyframe_offset = 404;

    expect_field_read<protostream::fields::kf_num>(keyframe_offset);
    expect_field_read<protostream::fields::delta_offset>(keyframe_offset);
    expect_field_read<protostream::fields::kf_size>(keyframe_offset);
    expect_skiplist_read(keyframe_offset);

    for (auto idx = 0u; idx < skiplist.size(); ++idx) {
        EXPECT_EQ(link(idx), cache->link_at(keyframe_offset, idx));
    }
}