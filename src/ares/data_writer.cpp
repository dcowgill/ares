// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/data_writer.hpp"
#include "ares/buffer.hpp"
#include "ares/bin_util.hpp"

using namespace std;
using ares::Data_writer;

Data_writer::Data_writer(Buffer& b)
        : Buffer_formatter(b)
{}

Data_writer::~Data_writer()
{}

bool Data_writer::put_int8(Int8 n)
{
    if (!*this)
        return false;
    Byte b = static_cast<Byte>(n);
    return write(&b, 1);
}

bool Data_writer::put_int16(Int16 n)
{
    if (!*this)
        return false;
    Byte buf[sizeof(n)];
    pack_int16(buf, n);
    return write(buf, sizeof(buf));
}

bool Data_writer::put_int32(Int32 n)
{
    if (!*this)
        return false;
    Byte buf[sizeof(n)];
    pack_int32(buf, n);
    return write(buf, sizeof(buf));
}

bool Data_writer::put_bytes(Byte const* data, Int32 len)
{
    if (!*this)
        return false;

    if (buffer().free() < int(sizeof(len)) + len) {
        fail();
        return false;
    }

    put_int32(len);
    return write(data, len);
}

bool Data_writer::write(Byte const* data, int count)
{
    if (!buffer().put(data, count)) {
        fail();
        return false;
    }
    return true;
}
