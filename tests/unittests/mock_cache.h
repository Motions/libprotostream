#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cache.h"
#include "header.h"

template<class Backend>
struct mock_cache : public protostream::cache_base<mock_cache<Backend>, Backend> {
    using base = protostream::cache_base<mock_cache<Backend>, Backend>;
public:
    mock_cache(Backend& backend) : base{backend} { }

    MOCK_CONST_METHOD1(header_at, protostream::reduced_keyframe_header(offset_t offset));
};