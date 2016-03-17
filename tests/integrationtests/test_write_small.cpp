#include <gtest/gtest.h>

#include "common.h"
#include "../common/temporary_file.h"
#include "../common/file_operations.h"

template<class Stream>
struct integration_write_small : public testing::Test { };

TYPED_TEST_CASE(integration_write_small, types::write_streams);

TYPED_TEST(integration_write_small, write) {
    auto file = temporary_file{};

    {
        constexpr auto header = "protostream header";
        auto stream = TypeParam{file.filepath(), 2, header, strlen(header)};

        for (auto i = 1; i < 20; i += 2) {
            const auto keyframe = std::to_string(i);
            const auto delta = std::to_string(i + 1);
            stream.append_keyframe(reinterpret_cast<const std::uint8_t*>(keyframe.c_str()), keyframe.length());
            stream.append_delta(reinterpret_cast<const std::uint8_t*>(delta.c_str()), delta.length());
        }
    }

    EXPECT_EQ(file_contents("data/small.data"), file.contents());
}
