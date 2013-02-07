// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_packet_reader
#define included_ares_packet_reader

#include "ares/buffer_formatter.hpp"

namespace ares {

// A buffer formatter that reads packets from a buffer. See the documentation
// for Packet_writer for more information about packets.
class Packet_reader : public Buffer_formatter {
  public:
    // Constructs a packet reader.
    Packet_reader(Buffer& b);

    // Destructor.
    virtual ~Packet_reader();

    // Reads a packet from the wrapped buffer, returning it as a dynamically
    // allocated Buffer object. If a packet could not be read, returns null.
    // The caller assumes ownership of the returned Buffer object.
    Buffer* get_packet();
};

} // namespace ares

#endif
