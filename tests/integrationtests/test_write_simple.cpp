#include <gtest/gtest.h>

#include "common.h"
#include "simple_tests.h"
#include "streams.h"
#include "type_list.h"
#include "../common/temporary_file.h"
#include "../common/file_operations.h"

template<class Param>
struct integration_write_simple : public testing::Test { };

using pairs = type_list::product<streams::write_streams, simple_tests::tests>::type<testing::Types>;

TYPED_TEST_CASE(integration_write_simple, pairs);

TYPED_TEST(integration_write_simple, write) {
    auto file = temporary_file{};

    {
        constexpr auto header = TypeParam::test::header;
        auto stream = typename TypeParam::stream{file.filepath(), TypeParam::test::frames_per_keyframe, header, strlen(header)};

        for (auto i = 0; i < TypeParam::test::frame_count; ++i) {
            const auto data = TypeParam::test::frame(i);
            if (i % TypeParam::test::frames_per_keyframe == 0) {
                stream.append_keyframe(reinterpret_cast<const std::uint8_t*>(data.c_str()), data.length());
            }
            else {
                stream.append_delta(reinterpret_cast<const std::uint8_t*>(data.c_str()), data.length());
            }
        }

        EXPECT_EQ(TypeParam::test::keyframe_count, stream.keyframe_count());
        EXPECT_EQ(TypeParam::test::frame_count, stream.frame_count());

    }

    EXPECT_EQ(file_contents(TypeParam::test::file), file.contents());
}