#include "simple_tests.h"

namespace simple_tests {

#define DECLARE_FIELD(cls, name) constexpr decltype(cls::name) cls::name
#define DECLARE_TEST(cls)                                                                          \
    DECLARE_FIELD(cls, file);                                                                      \
    DECLARE_FIELD(cls, header);                                                                    \
    DECLARE_FIELD(cls, keyframe_count);                                                            \
    DECLARE_FIELD(cls, frame_count);                                                               \
    DECLARE_FIELD(cls, frames_per_keyframe);

DECLARE_TEST(small_test);
DECLARE_TEST(medium_test);
}