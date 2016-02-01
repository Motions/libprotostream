#ifndef _BASIC_STREAM_H
#define _BASIC_STREAM_H

#include "stream_base.h"
#include <cstdlib>
#include <array>
#include <vector>

class stream_reader : public stream_base {
    public:
    stream_reader(const char *path, stream_base::open_mode mode = stream_base::READ_ONLY):
        stream_base(path, mode),
        header_proto(nullptr),
        header_proto_size(0)
    {
    }
    ~stream_reader() {
        free(header_proto);
    }
    protected:
    void *header_proto;
    struct keyframe_header {
        std::array<offset_t, skiplist_height> skiplist;
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
    size_t header_proto_size;
    public:
    int load();
    // The returned pointer is managed by this class, do not free it
    void *load_header();
    void *get_header(size_t *s) {
        if(!header_proto)
            load_header();
        *s = header_proto_size;
        return header_proto;
    }
    void *get_keyframe_data(offset_t n, size_t *size);
    std::vector<std::pair<void*, size_t>> get_deltaframes_for_kf(offset_t keyframe);
    static void free_deltaframe_vector(std::vector<std::pair<void*, size_t>> v) {
        for(auto p: v)
            free(p.first);
    }
};
#endif
