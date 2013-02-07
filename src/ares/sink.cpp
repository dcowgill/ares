// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/sink.hpp"
#include "ares/buffer.hpp"

using ares::Sink;

Sink::~Sink()
{}

void Sink::send(Byte const* data, int count)
{
    Buffer b(data, count);
    send(b);
}
