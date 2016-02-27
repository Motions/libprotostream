#ifndef _STREAM_BASE_H
#define _STREAM_BASE_H
#include <type_traits>
#include <unordered_map>
#include <fcntl.h>
#include <vector>

#include "common.h"

class stream_base {
    public:
    enum class open_mode { READ_ONLY, READ_WRITE };
    private:
    const open_mode mode;
    //this is private to ensure nobody reads from it directly
    //always use offset_for_kf
    std::unordered_map<offset_t, offset_t> keyframe_offsets;

    protected:
    int fd;
    uint32_t frames_per_kf;
    offset_t file_size,
             hdr_data_offset,
             kf0_off,
             keyframe_count;
    //This pointer is *NOT* freed by stream_base.
    //Child classes must handle this in their destructors.
    void *header_proto;
    size_t header_proto_size;

    stream_base(open_mode mode):
        mode(mode), fd(-1),
        frames_per_kf(0),
        file_size(0), kf0_off(0), keyframe_count(0),
        header_proto(nullptr),
        header_proto_size(0)
    {
    }
    static inline offset_t kfhdr_read_skiplist(const void *buf, uint8_t i){
        //return readbuf_aligned<offset_t>((uint8_t*)buf + sizeof(offset_t) * (2 + i));
        return readbuf_unaligned<offset_t>((uint8_t*)buf + sizeof(offset_t) * (2 + i));
    }
    static inline uint32_t kfhdr_read_data_size(const void *buf) {
        //return readbuf_aligned<uint32_t>((uint8_t*)buf + sizeof(offset_t) * (skiplist_height + 2));
        return readbuf_unaligned<uint32_t>((uint8_t*)buf + sizeof(offset_t) * (skiplist_height + 2));
    }
    static inline offset_t kfhdr_read_deltas(const void *buf) {
        //return readbuf_aligned<offset_t>((uint8_t*)buf + sizeof(offset_t));
        return readbuf_unaligned<offset_t>((uint8_t*)buf + sizeof(offset_t));
    }
    bool load_file_header(const uint8_t *hdr_buf, offset_t fsize) {
        if(0 != memcmp(hdr_buf, magic, magic_size)) {
            fprintf(stderr, "Invalid magic at beginning of file.\n");
            return false;
        }
        //potential extra padding would be here
        static_assert(sizeof(offset_t) <= 8, "TODO padding after magic");

        file_size = readbuf_aligned<uint64_t>(hdr_buf + 8);
        if(fsize != file_size) {
            fprintf(stderr, "File size not consistent with data in header.\n");
            return false;
        }
        hdr_data_offset = readbuf_aligned<uint64_t>(hdr_buf + 8*2);
        kf0_off = readbuf_aligned<uint64_t>(hdr_buf + 8*3);
        keyframe_count = readbuf_aligned<uint64_t>(hdr_buf + 8*4);
        frames_per_kf = readbuf_aligned<uint32_t>(hdr_buf + 8*5);
        if(hdr_data_offset > file_size - header_size ||
                kf0_off > file_size - header_size ||
                hdr_data_offset > kf0_off) {
            fprintf(stderr, "Invalid offsets in header.\n");
            return false;
        }
#ifdef DEBUG
        fprintf(stderr, "_HDR: hdr_d_off %ld, kf0 %ld, kfc %ld, fpkf %d\n", hdr_data_offset, kf0_off, keyframe_count, frames_per_kf);
#endif
        keyframe_offsets[0] = kf0_off;
        return true;
    }
    void fill_offsets_from_skiplist(const void *kfhdr, offset_t n) {
        for(unsigned i = 0; i < skiplist_height; i++){
            offset_t tmp = kfhdr_read_skiplist(kfhdr, i);
            if(tmp != 0) {
                offset_t num = n + (1 << i);
                assert(num < keyframe_count);
                keyframe_offsets.emplace(num, tmp);
            }
        }
    }
    inline void set_kf_offset(offset_t k, offset_t v) {
#ifdef DEBUG
        assert(keyframe_offsets.count(k) == 0);
#endif
        keyframe_offsets.emplace(k, v);
    }
    //required for implementations of find_keyframe
    inline bool has_kf_offset(offset_t n) {
        return keyframe_offsets.count(n) == 1;
    }
    public:
    virtual ~stream_base() {
        if(fd != -1)
            close(fd);
    }
    virtual int open(const char *path) {
        if(mode == open_mode::READ_WRITE)
            fd = ::open(path, O_RDWR | O_CREAT | O_EXCL, 0666);
        else
            fd = ::open(path, O_RDONLY);
        if(fd)
            return 0;
        else
            return -1;
    }
    virtual void find_keyframe(offset_t n) = 0;
    inline offset_t offset_for_kf(offset_t n) {
        if(!keyframe_offsets.count(n))
            find_keyframe(n);
        return keyframe_offsets[n];
    }
    void *get_header(size_t *s) {
        if(!header_proto)
            load_header();
        *s = header_proto_size;
        return header_proto;
    }
    // The returned pointer is managed by this class, do not free it
    virtual void *load_header() = 0;
    virtual void *get_keyframe_data(offset_t n, size_t *size) = 0;
    virtual std::vector<std::pair<void*, size_t>> get_deltaframes_for_kf(offset_t keyframe) = 0;

};


#endif
