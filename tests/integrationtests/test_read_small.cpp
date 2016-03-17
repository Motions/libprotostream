#include <gtest/gtest.h>

#include "common.h"

template<class Stream>
struct integration_read_small : public testing::Test {
    virtual void SetUp() override {
        stream = std::make_unique<Stream>("data/small.data");
    }

    virtual void TearDown() override {
        stream.reset();
    }
protected:
    std::unique_ptr<Stream> stream;
};

TYPED_TEST_CASE(integration_read_small, types::read_streams);

TYPED_TEST(integration_read_small, proto_header) {
    EXPECT_EQ("protostream header", this->stream->get_proto_header());
}

TYPED_TEST(integration_read_small, keyframe_count) {
    EXPECT_EQ(10, std::distance(this->stream->begin(), this->stream->end()));
}

TYPED_TEST(integration_read_small, keyframes) {
    int cnt = 1;
    for(const auto& keyframe : *this->stream) {
        const auto expected = std::to_string(cnt);
        EXPECT_EQ(expected, keyframe.get());
        EXPECT_EQ(expected, types::string_factory::build(keyframe.raw(), keyframe.size()));
        cnt += 2;
    }
}

TYPED_TEST(integration_read_small, deltas) {
    int cnt = 2;
    for(const auto& keyframe : *this->stream) {
        ASSERT_EQ(1, std::distance(keyframe.begin(), keyframe.end()));

        const auto expected = std::to_string(cnt);
        EXPECT_EQ(expected, keyframe.begin()->get());
        EXPECT_EQ(expected, types::string_factory::build(keyframe.begin()->raw(), keyframe.begin()->size()));
        cnt += 2;
    }
}
