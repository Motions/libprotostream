#pragma once

#include <cstdint>

namespace protostream {

using delta_size_t = std::uint16_t;
using offset_t = std::uint64_t;
using keyframe_id_t = std::uint64_t;
constexpr offset_t no_keyframe = 0;
}
