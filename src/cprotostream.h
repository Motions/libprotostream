#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {

/** Almost every function declared in this file wraps a protostream function which may throw
 * an exception if a system error occurs. In this case cprotostream aborts execution.
 */
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

/** An opaque handle to a stream */
typedef struct HStream HStream;

/** An opaque handler to a keyframe iterator */
typedef struct HKeyframeIterator HKeyframeIterator;

/** An opaque handler to a delta iterator */
typedef struct HDeltaIterator HDeltaIterator;

/** Opens a protostream file, reading its header */
HStream* protostream_open_existing(const char* path) NOEXCEPT;

/** Opens a protostream file and writes a new header to it */
HStream* protostream_open_new(const char* path, uint32_t frames_per_kf,
                              const void* proto_header, size_t proto_header_size) NOEXCEPT;

/** Closes a protostream file */
void protostream_close(HStream* stream) NOEXCEPT;

/** Appends a keyframe to a protostream */
void protostream_append_keyframe(HStream* stream, const void* keyframe, size_t keyframe_size) NOEXCEPT;

/** Appends a delta to a protostream */
void protostream_append_delta(HStream* stream, const void* delta, size_t delta_size) NOEXCEPT;

/** Retrieves the header of a protostream.
 *
 * The address of the header is written into *into and its size into *size
 */
void protostream_get_header(HStream* stream, const void** into, size_t* size) NOEXCEPT;

/** Frees a header returned by protostream_get_header */
void protostream_free_header(const void* header) NOEXCEPT;

/** Returns an iterator over keyframes in a stream */
HKeyframeIterator* protostream_iter_keyframes(HStream* stream) NOEXCEPT;

/** Retrieves the contents of a keyframe.
 *
 * Precondition: protostream_valid_keyframe_iterator(stream, iterator),
 *     where stream is the appropriate HStream
 * The address of the keyframe is written into *into and its size into *size
 */
void protostream_get_keyframe(HKeyframeIterator* iterator, const void** into, size_t* size) NOEXCEPT;

/** Frees the contents of a keyframe returned by protostream_get_keyframe */
void protostream_free_keyframe(const void* keyframe) NOEXCEPT;

/** Checks if a keyframe iterator is valid.
 *
 *  Return value:
 *     - 1 if the iterator points to a valid keyframe
 *     - 0 if the iterator is a past-the-end iterator
 */
int protostream_valid_keyframe_iterator(HStream* stream, HKeyframeIterator* iterator) NOEXCEPT;

/** Advances a keyframe iterator.
 *
 *  Precondition: protostream_valid_keyframe_iterator(stream, iterator),
 *     where stream is the appropriate HStream
 */
void protostream_advance_keyframe_iterator(HKeyframeIterator* iterator) NOEXCEPT;

/** Frees a keyframe iterator */
void protostream_free_keyframe_iterator(HKeyframeIterator* iterator) NOEXCEPT;

/** Returns an iterator over keyframes associated with the given keyframe */
HDeltaIterator* protostream_iter_deltas(HKeyframeIterator* keyframe) NOEXCEPT;

/** Retrieves the contents of a delta.
 *
 *  Precondition: protostream_valid_delta_iterator(keyframe, iterator),
 *     where keyframe is the appropriate HKeyframeIterator
 *  The address of the delta is written into *into and its size into *size
 */
void protostream_get_delta(HDeltaIterator* iterator, const void** into, size_t* size) NOEXCEPT;

/** Frees the contents of a delta returned by protostream_get_delta */
void protostream_free_delta(const void* delta) NOEXCEPT;

/** Checks if a delta iterator is valid.
 *
 *  Return value:
 *     - 1 if the iterator points to a valid delta
 *     - 0 if the iterator is a past-the-end iterator
 */
int protostream_valid_delta_iterator(HKeyframeIterator* keyframe, HDeltaIterator* iterator) NOEXCEPT;

/** Advances a delta iterator.
 *
 *  Precondition: protostream_valid_delta_iterator(keyframe, iterator),
 *     where keyframe is the appropriate HKeyframeIterator
 */
void protostream_advance_delta_iterator(HDeltaIterator* iterator) NOEXCEPT;

/** Frees a delta iterator */
void protostream_free_delta_iterator(HDeltaIterator* iterator) NOEXCEPT;

#ifdef __cplusplus
} // extern "C"
#endif