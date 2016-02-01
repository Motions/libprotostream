#include "stream_writer.h"
#include <limits>
#include <cassert>

int stream_writer::init(uint32_t fpkf, const void* hdr_data, size_t data_size) {
    frames_per_kf = fpkf;
    hdr_data_offset = header_size;
    header_proto_size = data_size;
    assert(header_size + header_proto_size > hdr_data_offset);
    //TODO assuming file is empty and seeked to 0
    bool ok = true;
    offset_t tmp;
    ok &= write_loop(fd, magic, magic_size);
    // file size, no keyframes
    file_size = header_size + data_size;
    writebuf<offset_t>(&tmp, file_size);
    ok &= write_loop(fd, &tmp, sizeof(offset_t));
    //offset to header data, right after this header
    writebuf<offset_t>(&tmp, hdr_data_offset);
    ok &= write_loop(fd, &tmp, sizeof(offset_t));
    //first kf offset
    writebuf<offset_t>(&tmp, header_size + header_proto_size);
    ok &= write_loop(fd, &tmp, sizeof(offset_t));
    //kf count
    tmp = 0;
    ok &= write_loop(fd, &tmp, sizeof(offset_t));
    uint32_t tmp2;
    writebuf<uint32_t>(&tmp2, frames_per_kf);
    ok &= write_loop(fd, &tmp2, sizeof(uint32_t));
    if(!ok) {
        fprintf(stderr, "Failed writing header.\n");
        return -1;
    }
    if(!write_loop(fd, hdr_data, data_size)) {
        fprintf(stderr, "Failed writing header data.\n");
        return -1;
    }
#ifdef DEBUG
    fprintf(stderr, "fpos: %ld, fsz: %ld\n", lseek(fd, 0, SEEK_CUR), file_size);
#endif
    assert(lseek(fd, 0, SEEK_CUR) == file_size);
    return 0;
}

int stream_writer::append_keyframe(const void *data, size_t data_size) {
    bool ok = true;
    assert(data_size < std::numeric_limits<uint32_t>::max());
    //assert deltas
    off_t current_pos = lseek(fd, 0, SEEK_CUR);
    struct keyframe_header new_hdr = {
        //.skiplist = {},
        .deltas = current_pos + kf_header_size + data_size,
        .data_off = current_pos + kf_header_size,
        .data_size = static_cast<uint32_t>(data_size)};
    set_kf_offset(keyframe_count, current_pos);
    kf_cache.emplace(keyframe_count, new_hdr);
    deltas_for_current = 0;
    offset_t tmp;
    //kf_num
    tmp = htobe(keyframe_count);
    ok &= write_loop(fd, &tmp, sizeof(offset_t));
    //delta_start
    tmp = htobe(new_hdr.deltas);
    ok &= write_loop(fd, &tmp, sizeof(offset_t));
    //empty skiplist
    void *zerobuf = calloc(skiplist_height, sizeof(offset_t));
    if(zerobuf == nullptr) {
        //TODO err
        return -1;
    }
    ok &= write_loop(fd, zerobuf, skiplist_height * sizeof(offset_t));
    free(zerobuf);
    //kf_size, TODO remove?
    uint32_t tmp2 = htobe(new_hdr.data_size);
    ok &= write_loop(fd, &tmp2, sizeof(uint32_t));
    for(unsigned i = 0; i < skiplist_height; i++)
        forward_pointers.emplace(keyframe_count + (1 << i),
                std::make_pair(current_pos, i));

    if(!fixup_skiplist(keyframe_count, current_pos))
        return -1;
    ok &= write_loop(fd, data, data_size);

    file_size += kf_header_size + data_size;
#ifdef DEBUG
    fprintf(stderr, "fpos: %ld, fsz: %ld\n", lseek(fd, 0, SEEK_CUR), file_size);
#endif
    assert(lseek(fd, 0, SEEK_CUR) == file_size);
    tmp = htobe(file_size);
    ok &= write_loop(fd, &tmp, sizeof(offset_t), magic_size);

    keyframe_count++;
    tmp = htobe(keyframe_count);
    ok &= write_loop(fd, &tmp, sizeof(offset_t), magic_size + sizeof(offset_t) * 3);
    if(!ok)
        return -1;
    return 0;
}

bool stream_writer::fixup_skiplist(offset_t new_kf_num, offset_t where) {
    auto iterators = forward_pointers.equal_range(new_kf_num);
    //endian-correct version of where
    offset_t where_buf;
    where_buf = htobe(where);
    bool ok = true;
    for(auto it = iterators.first; it != iterators.second; it++) {
        offset_t kf_hdr_off = it->second.first;
        uint8_t slot = it->second.second;
        offset_t addr = kf_hdr_off + sizeof(offset_t) * (2 + slot);
#ifdef DEBUG
        fprintf(stderr, "Fixup for %ld: %ld(@%ld)[%d] -> %ld, WR %ld\n", new_kf_num, -1l, kf_hdr_off, slot, where, addr);
#endif
        ok &= write_loop(fd, &where_buf, sizeof(offset_t), addr);
    }
    forward_pointers.erase(iterators.first, iterators.second);
    return ok;
}

int stream_writer::append_delta(const void *data, size_t data_size) {
    assert(deltas_for_current < frames_per_kf);
    assert(! (lseek(fd, 0, SEEK_CUR) == kf_cache[keyframe_count].deltas && deltas_for_current == 0));
    assert(data_size < std::numeric_limits<delta_size_t>::max());
    delta_size_t size = htobe<delta_size_t>(data_size);
    write_loop(fd, &size, sizeof(delta_size_t));
    file_size += sizeof(delta_size_t);

    write_loop(fd, data, data_size);
    file_size += data_size;
    offset_t tmp;
    assert(lseek(fd, 0, SEEK_CUR) == file_size);
    tmp = htobe(file_size);
    write_loop(fd, &tmp, sizeof(offset_t), magic_size);
    return 0;
}
