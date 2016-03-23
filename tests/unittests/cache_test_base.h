#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mock_backend.h"

#include "common.h"
#include "header.h"

template <template <class> class Cache>
struct cache_test_base : public testing::Test {
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
        cache = std::make_unique<Cache<mock_backend>>(*backend);
    }

    virtual void TearDown() override {
        cache.reset();
        backend.reset();
    }

    void expect_skiplist_read(protostream::offset_t keyframe_offset) {
        EXPECT_CALL(*backend, read(keyframe_offset + protostream::fields::skiplist_offset(),
                                   sizeof(skiplist)))
            .WillOnce(testing::Return(reinterpret_cast<const std::uint8_t*>(skiplist.data())));
    }

    template <class Field>
    void expect_field_reads(protostream::offset_t keyframe_offset) {
        read_helper<Field>{}(*backend, keyframe_offset)
            .WillRepeatedly(testing::Return(header.get<Field>()));
    }

    template <class Field>
    void expect_field_read(protostream::offset_t keyframe_offset) {
        read_helper<Field>{}(*backend, keyframe_offset)
            .WillOnce(testing::Return(header.get<Field>()));
    }

    std::unique_ptr<mock_backend> backend;
    std::unique_ptr<Cache<mock_backend>> cache;

    static protostream::reduced_keyframe_header header;
    static std::array<offset_t, protostream::fields::skiplist_height> skiplist;

private:
    template <class Field, unsigned bits = 8 * Field::size>
    struct read_helper;

#define CACHE_TEST_BASE_DEFINE_HELPER(bits)                                                        \
    template <class Field>                                                                         \
    struct read_helper<Field, bits> {                                                              \
        auto&& operator()(mock_backend & backend, protostream::offset_t keyframe_offset) {         \
            return EXPECT_CALL(backend, read##bits(keyframe_offset + Field::offset));              \
        }                                                                                          \
    };

    MOCK_BACKEND_FOR_ALL_NUMERICS(CACHE_TEST_BASE_DEFINE_HELPER);
};

template <template <class> class Cache>
protostream::reduced_keyframe_header cache_test_base<Cache>::header;

template <template <class> class Cache>
std::array<offset_t, protostream::fields::skiplist_height> cache_test_base<Cache>::skiplist;