#pragma once

#include "config.h"

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif /* HAVE_ENDIAN_H */

#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif /* HAVE_MACHINE_ENDIAN_H */

#ifdef HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#if defined(HAVE_BETOH64) and not defined(HAVE_BE64TOH)
#define be64toh(x) betoh64(x)
#define be32toh(x) betoh32(x)
#define be16toh(x) betoh16(x)
#endif /* defined(HAVE_BETOH64) and not defined(HAVE_BE64TOH) */
#endif /* HAVE_SYS_ENDIAN_H */

#ifdef HAVE_KERN_OSBYTEORDER_H
#include <libkern/OSByteOrder.h>
#define be64toh(x) OSSwapBigToHostInt64(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htobe16(x) OSSwapHostToBigInt16(x)
#endif /* HAVE_KERN_OSBYTEORDER_H */

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
template <class...>
struct conjunction;

template <>
struct conjunction<> : std::true_type {};

template <class Head, class... Tail>
struct conjunction<Head, Tail...> : std::conditional_t<Head::value, conjunction<Tail...>, Head> {};

template <typename T>
T betoh(const T);

template <>
inline std::uint64_t betoh<std::uint64_t>(const std::uint64_t x) {
    return be64toh(x);
}

template <>
inline std::uint32_t betoh<std::uint32_t>(const std::uint32_t x) {
    return be32toh(x);
}

template <>
inline std::uint16_t betoh<std::uint16_t>(const std::uint16_t x) {
    return be16toh(x);
}

template <typename T>
T htobe(const T);

template <>
inline std::uint64_t htobe<std::uint64_t>(const std::uint64_t x) {
    return htobe64(x);
}

template <>
inline std::uint32_t htobe<std::uint32_t>(const std::uint32_t x) {
    return htobe32(x);
}

template <>
inline std::uint16_t htobe<std::uint16_t>(const std::uint16_t x) {
    return htobe16(x);
}

template <typename T>
inline T readbuf_aligned(const void* ptr) {
    assert(reinterpret_cast<uintptr_t>(ptr) % sizeof(T) == 0);
    return betoh<T>(*static_cast<const T*>(ptr));
}

template <typename T>
inline void writebuf(void* ptr, const T x) {
    assert(reinterpret_cast<uintptr_t>(ptr) % sizeof(T) == 0);
    *static_cast<T*>(ptr) = htobe<T>(x);
}

template <typename T>
T readbuf_unaligned(const void* a) {
    T val;
    memcpy(&val, a, sizeof(T));
    val = betoh(val);
    return val;
}

template <class Ptr>
auto as_ptr(Ptr& ptr) {
    return ptr.get();
}

template <class T>
auto as_ptr(T* ptr) {
    return ptr;
}
}
}
