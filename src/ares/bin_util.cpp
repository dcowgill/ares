// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/bin_util.hpp"

int ares::compress_uint32(Byte* buf, Uint32 n) {
    if (n < 254) {
        buf[0] = static_cast<Byte>(n);
        return sizeof(Byte);
    }
    else if (n < 65535) {
        buf[0] = static_cast<Byte>(254);
        pack_uint16(buf+1, n);
        return sizeof(Uint16)+1;
    }
    else {
        buf[0] = static_cast<Byte>(255);
        pack_uint32(buf+1, n);
        return sizeof(Uint32)+1;
    }
}

int ares::decompress_uint32(Byte const* buf, Uint32& n) {
    if (buf[0] < 254) {
        n = static_cast<Uint32>(buf[0]);
        return sizeof(Byte);
    }
    else if (buf[0] == 254) {
        n = unpack_uint16(buf+1);
        return sizeof(Uint16)+1;
    }
    else {
        n = unpack_uint32(buf+1);
        return sizeof(Uint32)+1;
    }
}

int ares::decompress_uint32(Byte const* buf, int len, Uint32& n)
{
    if (len <= 0) return 0;     // can't do anything when len<1

    if (buf[0] < 254) {
        n = static_cast<Uint32>(buf[0]);
        return sizeof(Byte);
    }
    else if (buf[0] == 254) {
        if (len < int(sizeof(Uint16)+1))
            return 0;
        n = unpack_uint16(buf+1);
        return sizeof(Uint16)+1;
    }
    else {
        if (len < int(sizeof(Uint32)+1))
            return 0;
        n = unpack_uint32(buf+1);
        return sizeof(Uint32)+1;
    }
}
