// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_bin_util
#define included_ares_bin_util

// Collection of portable operations on binary data.
//
// Note: Multiple versions of the pack and unpack methods are required because
// bit-manipulation on unsigned and signed values do not precisely correspond.
// Specifically, right-shift on signed values is not portable (without
// explicit bit masking), and sign-bits must be removed from signed byte
// values before they can be combined into a target integer value.

#include "ares/types.hpp"

namespace ares {

// Stores a signed 16-bit integer in a byte buffer. The integer is packed in
// most to least significant byte order (also called network byte order).
inline void pack_int16(Byte* buf, Int16 n) {
    buf[0] = (n >> 8) & 0xFF;
    buf[1] = n;
}

// Stores a signed 32-bit integer in a byte buffer. The integer is packed in
// most to least significant byte order (also called network byte order).
inline void pack_int32(Byte* buf, Int32 n) {
    buf[0] = (n >> 24) & 0xFF;
    buf[1] = (n >> 16) & 0xFF;
    buf[2] = (n >> 8)  & 0xFF;
    buf[3] = n;
}

// Reverses the operation performed by pack_int16.
inline Int16 unpack_int16(Byte const* buf) {
    Int16 n = buf[0] << 8;
    return n | (buf[1] & 0xFF);
}

// Reverses the operation performed by pack_int32.
inline Int32 unpack_int32(Byte const* buf) {
    Int32 n = (buf[0] << 24);
    n |= (buf[1] << 16) & 0xFF0000;
    n |= (buf[2] << 8)  & 0xFF00;
    return n | (buf[3] & 0xFF);
}

// Stores an unsigned 16-bit integer in a byte buffer. The integer is packed
// in most to least significant byte order (also called network byte order).
inline void pack_uint16(Byte* buf, Uint16 n)
{
    buf[0] = n >> 8;
    buf[1] = n;
}

// Stores an unsigned 32-bit integer in a byte buffer. The integer is packed
// in most to least significant byte order (also called network byte order).
inline void pack_uint32(Byte* buf, Uint32 n) {
    buf[0] = n >> 24;
    buf[1] = n >> 16;
    buf[2] = n >> 8;
    buf[3] = n;
}

// Reverses the operation performed by pack_uint16.
inline Uint16 unpack_uint16(Byte const* buf) {
    return unpack_int16(buf);
}

// Reverses the operation performed by pack_uint32.
inline Uint32 unpack_uint32(Byte const* buf) {
    return unpack_int32(buf);
}

// Packs n into buf, using as little space as possible. Specifically, n will
// consume one byte in buf if it is less than 2^8-1, three bytes if less than
// 2^16-1, and five bytes otherwise. Therefore, buf must point to at least
// five bytes of contiguous memory. Use decompress_uint32 to reverse this
// operation. Returns the number of bytes used.
int compress_uint32(Byte* buf, Uint32 n);

// Reverses the operation performed by compress_uint32.
int decompress_uint32(Byte const* buf, Uint32& n);

// Similar to decompress_uint32(Byte const*,Uint32&), except this function
// does not assume that up to five bytes are available in buf. Instead, the
// caller must specify the number of bytes available in the buffer via the len
// parameter. Returns the number of bytes used in buf, or 0 if len was too
// small to read the entire integer value.
int decompress_uint32(Byte const* buf, int len, Uint32& n);

// Reverses the order of bytes in a signed 16-bit integer, returning the
// resulting integer value. Note that the bytes are reversed in logical high
// to low order regardless of the endianess of the machine.
inline Int16 reverse_int16(Int16 n) {
    return ((n >> 8) & 0xFF) | ((n << 8) & 0xFF00);
}

// Similar to reverse_int16, this function operates on unsigned values.
inline Uint16 reverse_uint16(Uint16 n) {
    return reverse_int16(n);
}

// Similar to reverse_int16, this function operates on 32-bit values.
inline Int32 reverse_int32(Int32 n) {
    return ((n >> 24) & 0xFF)      |
            ((n >>  8) & 0xFF00)    |
            ((n <<  8) & 0xFF0000)  |
            ((n << 24) & 0xFF000000);
}

// Similar to reverse_int32, this function operates on unsigned values.
inline Uint32 reverse_uint32(Uint32 n) {
    return reverse_int32(n);
}

} // namespace ares

#endif
