// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/basic_writer.hpp"
#include "ares/buffer.hpp"

using namespace std;

ares::Basic_writer::Basic_writer(Buffer& b)
        : Buffer_formatter(b)
{}

ares::Basic_writer::~Basic_writer()
{}

bool ares::Basic_writer::write(Byte const* data, int count)
{
    // Abort the operation if we're in the failed state.
    if (!*this) {
        return 0;
    }
    if (!buffer().put(data, count)) {
        fail();
        return false;
    }
    return true;
}
