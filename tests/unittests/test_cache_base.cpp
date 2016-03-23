#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_cache.h"
#include "cache_test_base.h"

#include "header.h"

using ::testing::Return;

struct cache_base : public cache_test_base<mock_cache> {};

TEST_F(cache_base, offset_of_unknown_keyframe) {
    EXPECT_EQ(std::experimental::optional<offset_t>{}, cache->offset_of(42));
}

TEST_F(cache_base, links) {
    constexpr offset_t keyframe_offset = 404;

    EXPECT_CALL(*cache, header_at(keyframe_offset)).WillRepeatedly(Return(header));

    expect_skiplist_read(keyframe_offset);

    for (auto idx = 0u; idx < skiplist.size(); ++idx) {
        EXPECT_EQ(link(idx), cache->link_at(keyframe_offset, idx));
    }
}

TEST_F(cache_base, offset_caching) {
    constexpr offset_t keyframe_offset = 302;

    EXPECT_CALL(*cache, header_at(keyframe_offset)).WillOnce(Return(header));

    expect_skiplist_read(keyframe_offset);

    /** Retrieve the keyframe metadata */
    cache->link_at(keyframe_offset, 0);

    EXPECT_EQ(keyframe_offset, cache->offset_of(header.get<protostream::fields::kf_num>()));

    for (auto idx = 0u; idx < skiplist.size(); ++idx) {
        const auto keyframe_id = header.get<protostream::fields::kf_num>() + (1 << idx);
        EXPECT_EQ(link(idx), cache->offset_of(keyframe_id));
    }
}