#include "stream.h"
#include "mmap_backend.h"
#include "posix_file_backend.h"
#include "cache.h"

namespace streams {

inline namespace types {

using namespace protostream;

struct string_factory {
    using type = std::string;

    static type build(const char* start, std::size_t len) {
        return std::string(start, start + len);
    }

    template<class Ptr>
    static type build(Ptr&& ptr, std::size_t len) {
        return build(reinterpret_cast<const char*>(detail::as_ptr(ptr)), len);
    }
};

using mmap_writer = stream<
        with_backend<mmap_backend<file_mode_t::READ_APPEND>>,
        with_cache<offsets_only_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

using stream_writer = stream<
        with_backend<posix_file_backend<file_mode_t::READ_APPEND>>,
        with_cache<full_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

using mmap_reader = stream<
        with_backend<mmap_backend<file_mode_t::READ_ONLY>>,
        with_cache<offsets_only_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

using stream_reader = stream<
        with_backend<posix_file_backend<file_mode_t::READ_ONLY>>,
        with_cache<full_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

}

using read_streams = std::tuple<
        types::mmap_reader, types::mmap_writer, types::stream_reader, types::stream_writer>;

using write_streams = std::tuple<
        types::mmap_writer, types::stream_writer>;

}