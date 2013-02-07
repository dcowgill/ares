// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/buffer_formatter.hpp"

ares::Buffer_formatter::~Buffer_formatter()
{}

void ares::Buffer_formatter::fail()
{
    m_fail = true;
    if (m_throw) {
        throw Buffer_formatter_error();
    }
}
