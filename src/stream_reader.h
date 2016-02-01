#ifndef _STREAM_READER_H
#define _STREAM_READER_H

#include "stream_base.h"
#include <cstdlib>
#include <array>

class stream_reader : public stream_base {
    public:
    stream_reader(stream_base::open_mode mode = open_mode::READ_ONLY):
        stream_base(mode)
    {
    }
    virtual ~stream_reader() {
        free(header_proto);
    }
    protected:
    struct keyframe_header {
        //std::array<offset_t, skiplist_height> skiplist;
        offset_t deltas;
        offset_t data_off;
        uint32_t data_size;
        keyframe_header& operator=(const keyframe_header&) = default;
    };
    std::unordered_map<offset_t, keyframe_header> kf_cache;
    void load_kf_header(offset_t num);
    virtual void find_keyframe(offset_t n) override;
    inline keyframe_header get_kf_header(offset_t n) {
        if(!kf_cache.count(n))
            load_kf_header(n);
        return kf_cache[n];
    }
    public:
    virtual int open(const char *path) override;
    virtual void *load_header() override;
    virtual void *get_keyframe_data(offset_t n, size_t *size) override;
    virtual std::vector<std::pair<void*, size_t>> get_deltaframes_for_kf(offset_t keyframe) override;
    static void free_deltaframe_vector(std::vector<std::pair<void*, size_t>> v) {
        for(auto p: v)
            free(p.first);
    }
};
#endif
