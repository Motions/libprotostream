#ifndef _LIBPROTOSTREAM_C_H
#define _LIBPROTOSTREAM_C_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t offset_t;

typedef struct HStream HStream;

HStream* new_stream_reader();
HStream* new_stream_writer();
HStream* new_mmap_reader();
void delete_stream(HStream* stream);

int stream_open(HStream* stream, const char* path);
offset_t stream_offset_for_kf(HStream* stream, uint64_t n);
void* stream_get_header(HStream* stream, size_t* size);
void* stream_load_header(HStream* stream);
void* stream_get_keyframe_data(HStream* stream, uint64_t n, size_t* size);
//TODO: get_deltaframes_for_kf czeka na lepsze czasy

#ifdef __cplusplus
}
#endif

#endif
