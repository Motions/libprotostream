#ifndef _UTIL_H
#define _UTIL_H
#include <sys/stat.h>
#include <endian.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cassert>

using delta_size_t = uint16_t;
using offset_t = uint64_t;
const unsigned skiplist_height = 10;


const char magic[] = "PROTOSTR";
const size_t magic_size = 8;
static_assert(magic_size == sizeof(magic) - 1, "magic size");     //without null byte

const size_t header_size = 8 /* magic */ + sizeof(offset_t) * 4 /* pointers */ +
                                sizeof(uint32_t) /* frames/kfr */;
const size_t kf_header_size = sizeof(offset_t) * (2 + skiplist_height) + sizeof(uint32_t);


//TODO cleanup

inline uint64_t readbuf64(const void *addr) {
#ifdef DEBUG
    assert((uintptr_t) addr % sizeof(uint64_t) == 0);
#endif
    return be64toh(*(uint64_t*)addr);
}

inline uint32_t readbuf32(const void *addr) {
#ifdef DEBUG
    assert((uintptr_t) addr % sizeof(uint32_t) == 0);
#endif
    return be32toh(*(uint32_t*)addr);
}

inline uint16_t readbuf16(const void *addr) {
#ifdef DEBUG
    assert((uintptr_t) addr % sizeof(uint16_t) == 0);
#endif
    return be16toh(*(uint16_t*)addr);
}

template<typename T>
T readbuf_aligned(const void *ptr);
template<>
inline uint16_t readbuf_aligned<uint16_t>(const void *a) {
    return readbuf16(a);
}
template<>
inline uint32_t readbuf_aligned<uint32_t>(const void *a) {
    return readbuf32(a);
}
template<>
inline uint64_t readbuf_aligned<uint64_t>(const void *a) {
    return readbuf64(a);
}

template<typename T>
void writebuf(void *ptr, const T);
template<>
inline void writebuf<uint16_t>(void *ptr, const uint16_t x) {
    *(uint16_t*)ptr = htobe16(x);
}
template<>
inline void writebuf<uint32_t>(void *ptr, const uint32_t x) {
    *(uint32_t*)ptr = htobe32(x);
}
template<>
inline void writebuf<uint64_t>(void *ptr, const uint64_t x) {
    *(uint64_t*)ptr = htobe64(x);
}

template<typename T>
T readbuf_unaligned(const void *a) {
    T val;
    memcpy(&val, a, sizeof(T));
    val = readbuf_aligned<T>(&val);
    return val;
}

bool read_loop(int fd, void *buf, size_t count, off_t offset=-1);

bool write_loop(int fd, const void *buf, size_t count, off_t offset=-1);

#endif
