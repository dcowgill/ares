// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_basic_writer
#define included_ares_basic_writer

#include "ares/buffer_formatter.hpp"
#include "ares/types.hpp"

namespace ares {

class Buffer;

// The most basic writing buffer formatter class. Objects of this class can
// only write sequences of bytes to their managed buffers.
class Basic_writer : public Buffer_formatter {
  public:
    // Constructs a basic writer.
    Basic_writer(Buffer& b);

    // Destructor.
    virtual ~Basic_writer();

    // Writes count bytes from data to the wrapped buffer. If the buffer has
    // sufficient capacity to store all count bytes, this function returns
    // true and the wrapped buffer is modified. Otherwise, this function
    // returns false and the buffer is not modified.
    virtual bool write(Byte const* data, int count);
};

} // namespace ares

#endif
