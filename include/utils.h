#pragma once

#include <endian.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>

#include <type_traits>

namespace protostream {

namespace detail {

/** Logical AND of conditions
 *
 * To be replaced with std::conjunction when it is finally available
 */
template<class...>
struct conjunction;

template<>
struct conjunction<> : std::true_type {
};

template<class Head, class... Tail>
struct conjunction<Head, Tail...> : std::conditional_t<Head::value, conjunction<Tail...>, Head> {
};

template<typename T>
T betoh(const T);

template<>
inline std::uint64_t betoh<std::uint64_t>(const std::uint64_t x) {
    return be64toh(x);
}

template<>
inline std::uint32_t betoh<std::uint32_t>(const std::uint32_t x) {
    return be32toh(x);
}

template<>
inline std::uint16_t betoh<std::uint16_t>(const std::uint16_t x) {
    return be16toh(x);
}

template<typename T>
T htobe(const T);

template<>
inline std::uint64_t htobe<std::uint64_t>(const std::uint64_t x) {
    return htobe64(x);
}

template<>
inline std::uint32_t htobe<std::uint32_t>(const std::uint32_t x) {
    return htobe32(x);
}

template<>
inline std::uint16_t htobe<std::uint16_t>(const std::uint16_t x) {
    return htobe16(x);
}

template<typename T>
inline T readbuf_aligned(const void* ptr) {
#ifdef DEBUG
    assert((uintptr_t) ptr % sizeof(T) == 0);
#endif
    return betoh<T>(*(T*) ptr);

}

template<typename T>
inline void writebuf(void* ptr, const T x) {
#ifdef DEBUG
    assert((uintptr_t) ptr % sizeof(T) == 0);
#endif
    *(T*) ptr = htobe<T>(x);
}

template<typename T>
T readbuf_unaligned(const void* a) {
    T val;
    memcpy(&val, a, sizeof(T));
    val = betoh(val);
    return val;
}

template<class Ptr>
auto as_ptr(Ptr& ptr) {
    return ptr.get();
}

template<class T>
auto as_ptr(T* ptr) {
    return ptr;
}

}

}