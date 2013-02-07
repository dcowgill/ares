// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_sink
#define included_ares_sink

#include "ares/types.hpp"

namespace ares {

class Buffer;

// An interface for objects that can receive Buffer objects, sending them to
// some destination.
class Sink {
  public:
    virtual ~Sink();

    // Sends the contents of a buffer to the sink. Note that this function may
    // arbitrarily modify the input buffer.
    virtual void send(Buffer const& b) = 0;

    // Sends the first `count` bytes in `data` to the sink. By default, this
    // function constructs a temporary Buffer object and calls
    // Sink::send(Buffer&).
    virtual void send(Byte const* data, int count);
};

} // namespace ares

#endif
