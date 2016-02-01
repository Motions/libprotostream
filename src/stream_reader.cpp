#include <cstring>
#include <cassert>

#include "stream_reader.h"
#include "common.h"

//std::max will be constexpr in c++14
template<typename T>
static inline constexpr T const_max(const T a, const T b) {
    return a > b ? a : b;
}

int stream_reader::open(const char *path) {
    if(stream_base::open(path) < 0)
        return -1;
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
    alignas(const_max(8ul, alignof(offset_t))) uint8_t hdr_buf[header_size];
    if(!read_loop(fd, hdr_buf, header_size))
        return -1;
    if(load_file_header(hdr_buf, st.st_size))
        return keyframe_count * frames_per_kf;
    else return -1;
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
    alignas(offset_t) uint8_t buf[kf_header_size];
    //if(0 != posix_memalign((void**)&buf, std::min(sizeof(void*), sizeof(offset_t)), kf_header_size)) {
        //fprintf(stderr, "Allocation failed.\n");
        //exit(1);
    //}
    if(!read_loop(fd, buf, kf_header_size, where)) {
        fprintf(stderr, "Read failed.\n");
        //TODO exit/return
    }
    keyframe_header &kf = kf_cache.emplace(n, keyframe_header()).first->second;
    offset_t kf_num = readbuf_aligned<offset_t>(buf);
    if(kf_num != n) {
        fprintf(stderr, "Inconsistent frame numbers in file.\n");
        //exit/ret
    }
    kf.deltas = kfhdr_read_deltas(buf);
    kf.data_off = where + kf_header_size;
    kf.data_size = kfhdr_read_data_size(buf);
    fill_offsets_from_skiplist(buf, n);
    //free(buf);
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
        ret.emplace_back(data, cur_size);
    }
    return ret;
}
