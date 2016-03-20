#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock_backend.h"
#include "mock_cache.h"

#include "header.h"

using ::testing::Return;


struct cache_base : public ::testing::Test {
    static offset_t link(unsigned level) {
        return 500000 + 100 * level;
    }

    static void SetUpTestCase() {
        using namespace protostream::fields;
        header.get<kf_num>() = 42;
        header.get<delta_offset>() = 0xdeadbeef;
        header.get<kf_size>() = 0xdeadbeef;

        for (auto idx = 0u; idx < skiplist.size(); ++idx) {
            skiplist[idx] = protostream::detail::htobe(link(idx));
        }
    }

    virtual void SetUp() override {
        backend = std::make_unique<mock_backend>();
        cache = std::make_unique<mock_cache<mock_backend>>(*backend);
    }

    virtual void TearDown() override {
        cache.reset();
        backend.reset();
    }

    std::unique_ptr<mock_backend> backend;
    std::unique_ptr<mock_cache<mock_backend>> cache;

    static protostream::reduced_keyframe_header header;
    static std::array<offset_t, protostream::fields::skiplist_height> skiplist;
};

decltype(cache_base::header) cache_base::header;
decltype(cache_base::skiplist) cache_base::skiplist;

TEST_F(cache_base, offset_of_unknown_keyframe) {
    EXPECT_EQ(std::experimental::optional<offset_t>{}, cache->offset_of(42));
}

TEST_F(cache_base, links) {
    constexpr offset_t keyframe_offset = 404;

    EXPECT_CALL(*cache, header_at(keyframe_offset))
                .WillRepeatedly(Return(header));

    EXPECT_CALL(*backend, read(keyframe_offset + protostream::fields::skiplist_offset(), sizeof(skiplist)))
                .WillRepeatedly(Return(reinterpret_cast<const std::uint8_t*>(skiplist.data())));

    for (auto idx = 0u; idx < skiplist.size(); ++idx) {
        EXPECT_EQ(link(idx), cache->link_at(keyframe_offset, idx));
    }
}

TEST_F(cache_base, offset_caching) {
    constexpr offset_t keyframe_offset = 302;

    EXPECT_CALL(*cache, header_at(keyframe_offset))
            .WillOnce(Return(header));

    EXPECT_CALL(*backend, read(keyframe_offset + protostream::fields::skiplist_offset(), sizeof(skiplist)))
            .WillOnce(Return(reinterpret_cast<const std::uint8_t*>(skiplist.data())));

    /** Retrieve the keyframe metadata */
    cache->link_at(keyframe_offset, 0);

    EXPECT_EQ(keyframe_offset, cache->offset_of(header.get<protostream::fields::kf_num>()));

    for (auto idx = 0u; idx < skiplist.size(); ++idx) {
        const auto keyframe_id = header.get<protostream::fields::kf_num>() + (1 << idx);
        EXPECT_EQ(link(idx), cache->offset_of(keyframe_id));
    }
}