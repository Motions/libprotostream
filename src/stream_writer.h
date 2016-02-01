#ifndef _STREAM_WRITER_H
#define _STREAM_WRITER_H

#include "stream_reader.h"

class stream_writer : public stream_reader {
    public:
    stream_writer():
        stream_reader(stream_base::READ_WRITE)
    {
    }
    virtual ~stream_writer(){
    }
    
    int init(uint32_t frames_per_kf, const void *data, size_t data_size);
    int append_keyframe(const void *data, size_t data_size);
    int append_delta(const void *data, size_t data_size);
    virtual int open(const char *path) override {
        //call open from base only
        //currently we don't support appending
        return stream_base::open(path);
    }

    private:
    uint32_t deltas_for_current;
    //A cache of skiplist slots we need to fill when appending a frame.
    //If forward_pointers[n] = [(k1, s1), (k2, s2), ..], and we want
    //to add a keyframe number n, we must set skiplist slots
    //s1 of keyframe k1, s2 of keyframe k2 (and so on) to offset of n.
    std::unordered_multimap<offset_t, std::pair<offset_t, uint8_t>>
        forward_pointers;
    bool fixup_skiplist(offset_t new_kf_num, offset_t where);


};


#endif
