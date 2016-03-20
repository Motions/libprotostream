#pragma once

#include <gmock/gmock.h>

#include "file_backend.h"

/** googlemock does not support mocking template methods,
 * so, as a workaround, methods reading std::uint{16,32,64}_t
 * are mocked and the read_num method template is specialised
 * for those particular types. To reduce code duplication and
 * allow easy addition of new types, the relevant code is
 * generated during preprocessing
 */
#define MOCK_BACKEND_READ_NAME(bits) read ## bits
#define MOCK_BACKEND_READ_TYPE(bits) std::uint ## bits ## _t

#define MOCK_BACKEND_MOCK_READ(bits) \
    MOCK_CONST_METHOD1(MOCK_BACKEND_READ_NAME(bits), \
                       MOCK_BACKEND_READ_TYPE(bits)(offset_t offset));
#define MOCK_BACKEND_MOCK_SPECIALIZE(bits) \
    template<> \
    inline MOCK_BACKEND_READ_TYPE(bits) mock_backend::read_num<>(offset_t offset) const { \
        return MOCK_BACKEND_READ_NAME(bits)(offset); \
    }

#define MOCK_BACKEND_FOR_ALL_NUMERICS(MACRO) \
    MACRO(16); \
    MACRO(32); \
    MACRO(64);

using offset_t = std::uint64_t;
class mock_backend : public protostream::file_backend<mock_backend> {
public:
    using pointer_type = const std::uint8_t*;
    MOCK_CONST_METHOD2(read, pointer_type(offset_t offset, std::size_t length));

    template<class T>
    T read_num(offset_t offset) const;

    MOCK_BACKEND_FOR_ALL_NUMERICS(MOCK_BACKEND_MOCK_READ);
};

MOCK_BACKEND_FOR_ALL_NUMERICS(MOCK_BACKEND_MOCK_SPECIALIZE);