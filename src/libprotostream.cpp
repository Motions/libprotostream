#include "stream_base.h"
#include "stream_reader.h"
#include "stream_writer.h"
#include "mmap_reader.h"

#include "libprotostream.h"


HStream* new_stream_reader(int open_mode)
{
    auto mode = static_cast<stream_base::open_mode>(open_mode);
    return reinterpret_cast<HStream*>(new stream_reader(mode));
}

HStream* new_stream_writer()
{
    return reinterpret_cast<HStream*>(new stream_writer());
}

HStream* new_mmap_reader()
{
    return reinterpret_cast<HStream*>(new mmap_reader());
}

void delete_stream(HStream* stream)
{
    delete reinterpret_cast<stream_base*>(stream);
}

int stream_open(HStream* stream, const char* path)
{
    return reinterpret_cast<stream_base*>(stream)->open(path);
}

void stream_find_keyframe(HStream* stream, uint64_t n)
{
    reinterpret_cast<stream_base*>(stream)->find_keyframe(n);
}

offset_t stream_offset_for_kf(HStream* stream, uint64_t n)
{
    return reinterpret_cast<stream_base*>(stream)->offset_for_kf(n);
}

void* stream_get_header(HStream* stream, size_t* size)
{
    return reinterpret_cast<stream_base*>(stream)->get_header(size);
}

void* stream_load_header(HStream* stream)
{
    return reinterpret_cast<stream_base*>(stream)->load_header();
}

void* stream_get_keyframe_data(HStream* stream, uint64_t n, size_t* size)
{
    return reinterpret_cast<stream_base*>(stream)->get_keyframe_data(n, size);
}