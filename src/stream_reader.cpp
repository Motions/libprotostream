#include <cstring>
#include <cassert>

#include "stream_reader.h"
#include "common.h"

int stream_reader::load() {
    struct stat st;
    if(0 != fstat(fd, &st)) {
        perror("fstat");
        return -1;
    }
    if(0 != lseek(fd, 0, SEEK_SET)) {
        perror("lseek");
        return -1;
    }
    if(st.st_size < header_size) {
        fprintf(stderr, "Input file too short.\n");
        return -1;
    }
    uint8_t hdr_buf[header_size];
    if(!read_loop(fd, hdr_buf, header_size))
        return -1;
    if(0 != memcmp(hdr_buf, magic, magic_size)) {
        fprintf(stderr, "Invalid magic at beginning of file.\n");
        return -1;
    }

    file_size = readbuf64(hdr_buf + 8);
    if(st.st_size != file_size) {
        fprintf(stderr, "File size not consistent with data in header.\n");
        return -1;
    }
    hdr_data_offset = readbuf_aligned<uint64_t>(hdr_buf + 8*2);
    kf0_off = readbuf_aligned<uint64_t>(hdr_buf + 8*3);
    keyframe_count = readbuf_aligned<uint64_t>(hdr_buf + 8*4);
    frames_per_kf = readbuf32(hdr_buf + 8*5);
    if(hdr_data_offset > file_size - header_size ||
            kf0_off > file_size - header_size ||
            hdr_data_offset > kf0_off) {
        fprintf(stderr, "Invalid offsets in header.\n");
        return -1;
    }
#ifdef DEBUG
    fprintf(stderr, "_HDR: hdr_d_off %ld, kf0 %ld, kfc %ld, fpkf %d\n", hdr_data_offset, kf0_off, keyframe_count, frames_per_kf);
#endif
    keyframe_offsets[0] = kf0_off;

    return keyframe_count * frames_per_kf;
}

void *stream_reader::load_header(){
    header_proto_size = kf0_off - hdr_data_offset;
    void *ret = malloc(header_proto_size);
    if(ret == nullptr)
        return nullptr;
    if(!read_loop(fd, ret, header_proto_size, hdr_data_offset)) {
        fprintf(stderr, "Error reading header protobuf from file.\n");
        free(ret);
        return nullptr;
    }
    header_proto = ret;
    return ret;
}

void stream_reader::load_kf_header(offset_t n) {
    if(kf_cache.count(n))
        return;
    offset_t where = offset_for_kf(n);
    unsigned char *buf;
    if(0 != posix_memalign((void**)&buf, std::min(sizeof(void*), sizeof(offset_t)), kf_header_size)) {
        fprintf(stderr, "Allocation failed.\n");
        exit(1);
    }
    if(!read_loop(fd, buf, kf_header_size, where)) {
        fprintf(stderr, "Read failed.\n");
        //TODO exit/return
    }
    keyframe_header &kf = (*kf_cache.insert({n, keyframe_header()}).first).second;
    offset_t kf_num = readbuf_aligned<offset_t>(buf);
    if(kf_num != n) {
        fprintf(stderr, "Inconsistent frame numbers in file.\n");
        //exit/ret
    }
    kf.deltas = readbuf_aligned<offset_t>(buf + sizeof(offset_t));
    kf.data_off = where + kf_header_size;
    kf.data_size = readbuf_aligned<uint32_t>(buf + sizeof(offset_t) * (skiplist_height + 2));
    for(unsigned i = 0; i < skiplist_height; i++){
        kf.skiplist[i] = readbuf_aligned<offset_t>(buf + sizeof(offset_t) * (2 + i));
        if(kf.skiplist[i] != 0) {
            offset_t num = n + (1 << i);
            assert(num < keyframe_count);
            keyframe_offsets.insert({num, kf.skiplist[i]});
        }
    }
    free(buf);
}

void stream_reader::find_keyframe(offset_t n) {
    //Instead of always starting from the beginning, check if keyframe_offsets
    //already contains values for n - 2^x for some x.
    for(unsigned i = 0; i < skiplist_height; i++) {
        offset_t x = n - (1<<i);
        if(keyframe_offsets.count(x)) {
            load_kf_header(x);
            return;
            //TODO czy tu na pewno nie ma buga, magic
        }
    }
    offset_t curr = 0;
    keyframe_header curr_h = get_kf_header(0);
    while(curr < n) {
        for(int i = skiplist_height - 1; i >= 0; i--) {
            if(curr + (1 << i) <= n) {
                curr += 1 << i;
                curr_h = get_kf_header(curr);
                break;
            }
        }
    }
    assert(curr == n);
}

void *stream_reader::get_keyframe_data(offset_t n, size_t *size) {
    if(!kf_cache.count(n))
        load_kf_header(n);
    *size = kf_cache[n].data_size;
    void *ret = malloc(*size);
    if(ret == nullptr)
        return nullptr;
    if(!read_loop(fd, ret, *size, kf_cache[n].data_off)) {
        free(ret);
        return nullptr;
    }
    return ret;
}


//This approach is fairly inefficient and loads the whole delta stream into memory at once.
std::vector<std::pair<void*, size_t>> stream_reader::get_deltaframes_for_kf(offset_t keyframe) {
    if(!kf_cache.count(keyframe))
        load_kf_header(keyframe);
    std::vector<std::pair<void*, size_t>> ret;
    offset_t current_off = kf_cache[keyframe].deltas;
    for(unsigned i = 0; i < frames_per_kf; i++) {
        uint8_t sz_buf[sizeof(delta_size_t)];
        if(!read_loop(fd, sz_buf, sizeof(delta_size_t), current_off)) {
            //TODO error
        }
        current_off += sizeof(delta_size_t);
        delta_size_t cur_size = readbuf_unaligned<delta_size_t>(sz_buf);
        void *data = malloc(cur_size);
        if(!read_loop(fd, data, cur_size, current_off)) {
            //TODO error
        }
        current_off += cur_size;
        ret.push_back({data, cur_size});
    }
    return ret;
}
