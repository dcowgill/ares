// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/data_reader.hpp"
#include "ares/buffer.hpp"
#include "ares/bin_util.hpp"

using namespace std;

ares::Data_reader::Data_reader(Buffer& b)
        : Basic_reader(b)
{}

ares::Data_reader::~Data_reader()
{}

ares::Int8 ares::Data_reader::get_int8()
{
    Int8 n = peek_int8();
    if (*this) buffer().consume(sizeof(n));
    return n;
}

ares::Int16 ares::Data_reader::get_int16()
{
    Int16 n = peek_int16();
    if (*this) buffer().consume(sizeof(n));
    return n;
}

ares::Int32 ares::Data_reader::get_int32()
{
    Int32 n = peek_int32();
    if (*this) buffer().consume(sizeof(n));
    return n;
}

ares::Int8 ares::Data_reader::peek_int8()
{
    return can_read(sizeof(Int8))
            ? static_cast<Int8>(buffer().begin()[0])
            : 255;
}

ares::Int16 ares::Data_reader::peek_int16()
{
    return can_read(sizeof(Int16))
            ? unpack_int16(buffer().begin())
            : 32767;
}

ares::Int32 ares::Data_reader::peek_int32()
{
    return can_read(sizeof(Int32))
            ? unpack_int32(buffer().begin())
            : 2147483647;
}

bool ares::Data_reader::get_string(string& s, int)
{
    if (can_read(sizeof(Int32))) {
        Int32 len = unpack_int32(buffer().begin());
        if (can_read(sizeof(len) + len)) {
            s.assign((char const*) buffer().begin() + sizeof(len), len);
            buffer().consume(len + sizeof(len));
            return true;
        }
    }
    return false;
}

string ares::Data_reader::get_string()
{
    string s;
    get_string(s);
    return s;
}

bool ares::Data_reader::get_bytes(Bytes& bytes)
{
    if (can_read(sizeof(Int32))) {
        Int32 len = unpack_int32(buffer().begin());
        if (can_read(sizeof(len) + len)) {
            bytes.assign(buffer().begin() + sizeof(len), len);
            buffer().consume(len + sizeof(len));
            return true;
        }
    }
    return false;
}

bool ares::Data_reader::can_read(int n)
{
    if (!*this) {
        return false;
    }
    if (buffer().size() < n) {
        fail();
        return false;
    }
    return true;
}
