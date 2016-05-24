#include <gtest/gtest.h>

#include "../common/file_operations.h"
#include "../common/temporary_file.h"
#include "simple_tests.h"
#include "streams.h"
#include "type_list.h"

template <class Stream>
struct integration_write_error : public testing::Test {};

TYPED_TEST_CASE(integration_write_error,
                type_list::unapply<streams::write_streams>::type<testing::Types>);

TYPED_TEST(integration_write_error, existing_file_as_new) {
    using simple_tests::small_test;
    constexpr auto header = small_test::header;

    auto file = temporary_file{file_contents(small_test::file)};
    EXPECT_THROW(
        TypeParam(file.filepath(), small_test::frames_per_keyframe, header, strlen(header)),
        std::runtime_error);
}
