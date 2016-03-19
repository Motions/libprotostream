#include <gtest/gtest.h>

#include "type_list.h"
#include "streams.h"
#include "../common/temporary_file.h"

template<class Stream>
struct integration_read_error : public testing::Test { };

TYPED_TEST_CASE(integration_read_error, unapply<streams::read_streams>::type<testing::Types>);

TYPED_TEST(integration_read_error, empty_file) {
    auto file = temporary_file{};
    EXPECT_THROW(TypeParam{file.filepath()}, std::runtime_error);
}

TYPED_TEST(integration_read_error, wrong_file_size) {
    EXPECT_THROW(TypeParam{"data/error/wrong_file_size.data"}, std::runtime_error);
}

TYPED_TEST(integration_read_error, wrong_proto_header_offset) {
    EXPECT_THROW(TypeParam{"data/error/wrong_proto_header_offset.data"}, std::runtime_error);
}

TYPED_TEST(integration_read_error, wrong_kf0_offset) {
    EXPECT_THROW(TypeParam{"data/error/wrong_kf0_offset.data"}, std::runtime_error);
}

TYPED_TEST(integration_read_error, proto_header_after_kf0) {
    EXPECT_THROW(TypeParam{"data/error/proto_header_after_kf0.data"}, std::runtime_error);
}