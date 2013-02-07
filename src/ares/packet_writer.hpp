// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_packet_writer
#define included_ares_packet_writer

#include "ares/buffer_formatter.hpp"

namespace ares {

// A buffer formatter that writes packets to a buffer, where a "packet" is
// defined as a sequence of zero or more bytes preceeded by a 4-byte header
// containing the length of the byte sequence. The packet data refers only to
// the byte sequence and does not include the header data.
class Packet_writer : public Buffer_formatter {
  public:
    // Constructs a packet writer.
    Packet_writer(Buffer& b);

    // Destructor.
    virtual ~Packet_writer();

    // Begins a new packet. This should eventually be followed by a call to
    // Packet_writer::end_packet before any further calls are made to this
    // function. Between this call and the call to Packet_writer::end_packet,
    // there are no restrictions on writes to the wrapped buffer. Reads must
    // _not_ occur because they may overwrite the packet header.
    bool begin_packet();

    // Ends the packet begun by a prior call to Packet_writer::begin_packet.
    // Any intervening writes to the wrapped buffer constitute the packet
    // data. Undefined if called out of sequence (that is, if it does not
    // follow a prior call to Packet_writer::begin_packet).
    bool end_packet();

  private:
    int m_original_size;
};

} // namespace ares

#endif
