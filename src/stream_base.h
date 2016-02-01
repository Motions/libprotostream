#ifndef _STREAM_BASE_H
#define _STREAM_BASE_H
#include <type_traits>
#include <unordered_map>
#include <fcntl.h>

#include "common.h"

class stream_base {
    //static_assert(std::is_integral<offset_t>::value,
            //"offset type must be integral");
    public:
    enum open_mode { READ_ONLY, READ_WRITE };

    protected:
    int fd;
    uint32_t frames_per_kf;
    offset_t file_size,
             hdr_data_offset,
             kf0_off,
             keyframe_count;
    std::unordered_map<offset_t, offset_t> keyframe_offsets;
    stream_base(const char *path, open_mode mode):
        frames_per_kf(0),
        file_size(0), kf0_off(0), keyframe_count(0)
    {
        if(mode == READ_WRITE) {
            fd = open(path, O_RDWR | O_CREAT | O_EXCL, 0666);
        } else 
            fd = open(path, O_RDONLY);
    }
    virtual void find_keyframe(offset_t n) = 0;
    inline offset_t offset_for_kf(offset_t n) {
        if(!keyframe_offsets.count(n))
            find_keyframe(n);
        return keyframe_offsets[n];
    }

};


#endif
