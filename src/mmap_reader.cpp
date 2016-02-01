#include "mmap_reader.h"
#include <cstring>

int mmap_reader::open(const char *path) {
    if(stream_base::open(path) < 0)
        return -1;
    //TODO those three are copied from stream_reader, dedup?
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
    //end todo
    mmap_buffer_size = st.st_size;
    //TODO might want to madvise(MADV_WILLNEED) somewhere
    mmap_buffer = (uint8_t*) mmap(nullptr, mmap_buffer_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(mmap_buffer == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    //load_file_header requires alignment, but this header itself is designed
    //to be aligned properly, and mmap_buffer is aligned to page size.
    if(load_file_header(mmap_buffer, st.st_size))
        return keyframe_count * frames_per_kf;

    return keyframe_count * frames_per_kf;
}

void *mmap_reader::load_header(){
    header_proto_size = kf0_off - hdr_data_offset;
    header_proto = mmap_buffer + hdr_data_offset;
    return header_proto;
}

void *mmap_reader::get_keyframe_data(offset_t n, size_t *size) {
    if(!keyframe_offsets.count(n))
        find_keyframe(n);   //TODO
    uint8_t* kf_ptr = mmap_buffer + keyframe_offsets[n];
    /// !!!!!! TODO THIS MAY BE *NOT* ALIGNED
    *size = kfhdr_read_data_size(kf_ptr);
    return kf_ptr + kf_header_size;
}

std::vector<std::pair<void*, size_t>> mmap_reader::get_deltaframes_for_kf(offset_t keyframe) {
    if(!keyframe_offsets.count(keyframe))
        find_keyframe(keyframe);
    std::vector<std::pair<void*, size_t>> ret;
    uint8_t *ptr = mmap_buffer + kfhdr_read_deltas(mmap_buffer + keyframe_offsets[keyframe]);
    for(unsigned i = 0; i < frames_per_kf; i++) {
        delta_size_t cur_size = readbuf_unaligned<delta_size_t>(ptr);
        ptr += sizeof(delta_size_t);
        void *data = ptr;
        ptr += cur_size;
        assert(ptr <= mmap_buffer + mmap_buffer_size);
        ret.push_back({data, cur_size});
    }
    return ret;
}

void mmap_reader::find_keyframe(offset_t n){
    for(unsigned i = 0; i < skiplist_height; i++) {
        offset_t x = n - (1<<i);
        auto it = keyframe_offsets.find(x);
        if(it != keyframe_offsets.end()) {
            fill_offsets_from_skiplist(mmap_buffer + it->second, x);
            return;
            //TODO czy tu na pewno nie ma buga, magic
        }
    }
    offset_t curr = 0;
    void* curr_ptr = mmap_buffer + kf0_off;
    while(curr < n) {
        fill_offsets_from_skiplist(curr_ptr, curr);
        for(int i = skiplist_height - 1; i >= 0; i--) {
            if(curr + (1 << i) <= n) {
                curr += 1 << i;
                curr_ptr = mmap_buffer + kfhdr_read_skiplist(curr_ptr, i);
                assert(curr_ptr <= mmap_buffer + mmap_buffer_size);
                break;
            }
        }
    }
    assert(curr == n);
}