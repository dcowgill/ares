// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/basic_reader.hpp"
#include "ares/buffer.hpp"
#include <cstring>

using namespace std;

ares::Basic_reader::Basic_reader(Buffer& b)
        : Buffer_formatter(b)
{}

ares::Basic_reader::~Basic_reader()
{}

bool ares::Basic_reader::read(Byte* dest, int count)
{
    // Abort the operation if we're in the failed state.
    if (!*this) {
        return 0;
    }
    if (!buffer().get(dest, count)) {
        fail();
        return false;
    }
    return true;
}

int ares::Basic_reader::read_partial(Byte* dest, int count)
{
    // Abort the operation if we're in the failed state.
    if (!*this) {
        return 0;
    }
    count = buffer().size() < count ? buffer().size() : count;
    memcpy(dest, buffer().begin(), count);
    buffer().consume(count);
    return count;
}
