#include <gtest/gtest.h>

#include "common.h"
#include "simple_tests.h"
#include "streams.h"
#include "type_list.h"

template<class Param>
struct integration_read_simple : public testing::Test {
    virtual void SetUp() override {
        stream = std::make_unique<typename Param::stream>(Param::test::file);
    }

    virtual void TearDown() override {
        stream.reset();
    }
protected:
    std::unique_ptr<typename Param::stream> stream;
};

using pairs = type_list::product<streams::read_streams, simple_tests::tests>::type<testing::Types>;

TYPED_TEST_CASE(integration_read_simple, pairs);

TYPED_TEST(integration_read_simple, proto_header) {
    EXPECT_EQ(TypeParam::test::header, this->stream->get_proto_header());
}

TYPED_TEST(integration_read_simple, keyframe_count) {
    EXPECT_EQ(TypeParam::test::keyframe_count, this->stream->keyframe_count());
    EXPECT_EQ(TypeParam::test::keyframe_count,
              std::distance(this->stream->begin(), this->stream->end()));
}

TYPED_TEST(integration_read_simple, frame_count) {
    EXPECT_EQ(TypeParam::test::frame_count, this->stream->frame_count());
}

TYPED_TEST(integration_read_simple, keyframes) {
    auto cnt = std::size_t{0};
    for(const auto& keyframe : *this->stream) {
        const auto expected = TypeParam::test::frame(cnt);
        EXPECT_EQ(expected, keyframe.get());
        EXPECT_EQ(expected, streams::types::string_factory::build(keyframe.raw(), keyframe.size()));
        cnt += TypeParam::test::frames_per_keyframe;
    }
}

TYPED_TEST(integration_read_simple, deltas) {
    auto cnt = std::size_t{0};
    for(const auto& keyframe : *this->stream) {
        if (cnt + TypeParam::test::frames_per_keyframe <= TypeParam::test::frame_count) {
            ASSERT_EQ(TypeParam::test::frames_per_keyframe - 1, std::distance(keyframe.begin(), keyframe.end()));
        }
        else {
            ASSERT_EQ(TypeParam::test::frame_count - cnt - 1, std::distance(keyframe.begin(), keyframe.end()));
        }

        cnt++;
        for(const auto& delta : keyframe) {
            const auto expected = TypeParam::test::frame(cnt);
            EXPECT_EQ(expected, delta.get());
            EXPECT_EQ(expected, streams::types::string_factory::build(delta.raw(), delta.size()));
            cnt++;
        }
    }
}
