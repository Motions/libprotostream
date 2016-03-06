#include "cprotostream.h"

#include "stream.h"
#include "mmap_backend.h"
#include "cache.h"


using namespace protostream;

using hstream = stream<
        with_backend<mmap_backend<file_mode_t::READ_APPEND>>,
        with_cache<offsets_only_cache>,
        with_keyframe_factory<default_factory<std::uint8_t*>>,
        with_delta_factory<default_factory<std::uint8_t*>>,
        with_proto_header_factory<default_factory<std::uint8_t*>>>;

HStream* protostream_open_existing(const char* path) {
    auto ptr = new hstream(path);
    return reinterpret_cast<HStream*>(ptr);
}

HStream* protostream_open_new(const char* path, uint32_t frames_per_kf,
                              const void* proto_header, size_t proto_header_size) {
    auto ptr = new hstream(path, frames_per_kf, proto_header, proto_header_size);
    return reinterpret_cast<HStream*>(ptr);
}

void protostream_close(HStream* stream) {
    auto ptr = reinterpret_cast<hstream*>(stream);
    delete ptr;
}

void protostream_append_keyframe(HStream* stream, const void* keyframe, size_t keyframe_size) {
    auto ptr = reinterpret_cast<hstream*>(stream);
    ptr->append_keyframe(static_cast<const std::uint8_t*>(keyframe), keyframe_size);
}

void protostream_append_delta(HStream* stream, const void* delta, size_t delta_size) {
    auto ptr = reinterpret_cast<hstream*>(stream);
    ptr->append_delta(static_cast<const std::uint8_t*>(delta), delta_size);
}

void protostream_get_header(HStream* stream, const void** into, size_t* size) {
    auto ptr = reinterpret_cast<hstream*>(stream);
    std::tie(*into, *size) = ptr->get_proto_header();
}

void protostream_free_header(const void*) { }

HKeyframeIterator* protostream_iter_keyframes(HStream* stream) {
    auto ptr = reinterpret_cast<hstream*>(stream);
    auto iter = new hstream::keyframe_iterator(ptr->begin());
    return reinterpret_cast<HKeyframeIterator*>(iter);
}

void protostream_get_keyframe(HKeyframeIterator* iterator, const void** into, size_t* size) {
    auto ptr = reinterpret_cast<hstream::keyframe_iterator*>(iterator);
    std::tie(*into, *size) = (*ptr)->get();
}

void protostream_free_keyframe(const void*) { }

int protostream_valid_keyframe_iterator(HStream* stream, HKeyframeIterator* iterator) {
    auto stream_ptr = reinterpret_cast<hstream*>(stream);
    auto it = reinterpret_cast<hstream::keyframe_iterator*>(iterator);
    return *it == stream_ptr->end();
}

void protostream_advance_keyframe_iterator(HKeyframeIterator* iterator) {
    ++*reinterpret_cast<hstream::keyframe_iterator*>(iterator);
}

void protostream_free_keyframe_iterator(HKeyframeIterator* iterator) {
    delete reinterpret_cast<hstream::keyframe_iterator*>(iterator);
}

HDeltaIterator* protostream_iter_deltas(HKeyframeIterator* keyframe) {
    auto ptr = reinterpret_cast<hstream::keyframe_iterator*>(keyframe);
    auto iter = new hstream::delta_iterator((*ptr)->begin());
    return reinterpret_cast<HDeltaIterator*>(iter);
}

void protostream_get_delta(HDeltaIterator* iterator, const void** into, size_t* size) {
    auto ptr = reinterpret_cast<hstream::delta_iterator*>(iterator);
    std::tie(*into, *size) = (*ptr)->get();
}

void protostream_free_delta(const void*) { }

int protostream_valid_delta_iterator(HKeyframeIterator* keyframe, HDeltaIterator* iterator) {
    auto keyframe_ptr = reinterpret_cast<hstream::keyframe_iterator*>(keyframe);
    auto it_ptr = reinterpret_cast<hstream::delta_iterator*>(iterator);
    return *it_ptr == (*keyframe_ptr)->end();
}

void protostream_advance_delta_iterator(HDeltaIterator* iterator) {
    ++*reinterpret_cast<hstream::delta_iterator*>(iterator);
}

void protostream_free_delta_iterator(HDeltaIterator* iterator) {
    delete reinterpret_cast<hstream::delta_iterator*>(iterator);
}