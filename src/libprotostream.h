#ifndef _LIBPROTOSTREAM_H
#define _LIBPROTOSTREAM_H

#include <stdint.h>


#ifdef _cplusplus
extern "C" {
#endif

typedef uint64_t offset_t;

struct HStream;
typedef struct HStream HStream;

/* open_mode: 0 = stream_base::open_mode::READ_ONLY
              1 = stream_base::open_mode__READ_WRITE */
HStream* new_stream_reader(int open_mode);
HStream* new_stream_writer();
HStream* new_mmap_reader();
void delete_stream(HStream* stream);

int stream_open(HStream* stream, const char* path);
void stream_find_keyframe(HStream* stream, uint64_t n);
offset_t stream_offset_for_kf(HStream* stream, uint64_t n);
void* stream_get_header(HStream* stream, size_t* size);
void* stream_load_header(HStream* stream);
void* stream_get_keyframe_data(HStream* stream, uint64_t n, size_t* size);
//TODO: get_deltaframes_for_kf czeka na lepsze czasy

#ifdef _cplusplus
}
#endif

#endif
