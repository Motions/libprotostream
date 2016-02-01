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


template <typename T>
T betoh(const T);
template<>
inline uint64_t betoh<uint64_t>(const uint64_t x) {
    return be64toh(x);
}
template<>
inline uint32_t betoh<uint32_t>(const uint32_t x) {
    return be32toh(x);
}
template<>
inline uint16_t betoh<uint16_t>(const uint16_t x) {
    return be16toh(x);
}

template <typename T>
T htobe(const T);
template<>
inline uint64_t htobe<uint64_t>(const uint64_t x) {
    return htobe64(x);
}
template<>
inline uint32_t htobe<uint32_t>(const uint32_t x) {
    return htobe32(x);
}
template<>
inline uint16_t htobe<uint16_t>(const uint16_t x) {
    return htobe16(x);
}

template<typename T>
inline T readbuf_aligned(const void *ptr) {
#ifdef DEBUG
    assert((uintptr_t) ptr % sizeof(T) == 0);
#endif
    return betoh<T>(*(T*)ptr);

}

template<typename T>
inline void writebuf(void *ptr, const T x) {
#ifdef DEBUG
    assert((uintptr_t) ptr % sizeof(T) == 0);
#endif
    *(T*)ptr = htobe<T>(x);
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
