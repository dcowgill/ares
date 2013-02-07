// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_basic_reader
#define included_ares_basic_reader

#include "ares/buffer_formatter.hpp"
#include "ares/types.hpp"

namespace ares {

class Buffer;

// The most basic reading buffer formatter class. Objects of this class
// can only read sequences of bytes from their managed buffers.
class Basic_reader : public Buffer_formatter {
  public:
    // Constructs a basic reader.
    Basic_reader(Buffer& b);

    // Destructor.
    virtual ~Basic_reader();

    // Reads count bytes from the wrapped buffer, storing them in dest. If the
    // buffer contains enough data to satisfy the read operation, this
    // function returns true and the bytes are removed from the buffer.
    // Otherwise, this function returns false and the buffer is not modified.
    virtual bool read(Byte* dest, int count);

    // Works like Basic_reader::read, except this operation is not atomic.
    // That is, if fewer than count bytes are available, this function will
    // read as many bytes as possible and return the short count. It is not
    // considered an error if the buffer is empty and nothing could be read.
    virtual int read_partial(Byte* dest, int count);
};

} // namespace ares

#endif
