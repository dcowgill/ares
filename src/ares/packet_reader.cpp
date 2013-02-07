// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/packet_reader.hpp"
#include "ares/buffer.hpp"
#include "ares/types.hpp"
#include "ares/bin_util.hpp"

using namespace std;

ares::Packet_reader::Packet_reader(Buffer& b)
        : Buffer_formatter(b)
{}

ares::Packet_reader::~Packet_reader()
{}

ares::Buffer* ares::Packet_reader::get_packet()
{
    // Abort the operation if we're in the failed state.
    if (!*this)
        return 0;

    Int32 packet_size;

    // Insure that there is enough data for the packet header.
    if (buffer().size() < int(sizeof(packet_size))) {
        fail();
        return 0;
    }

    // Extract the packet header.
    packet_size = unpack_int32(buffer().begin());

    // Insure that there is enough data for the packet data.
    if (buffer().size() < int(sizeof(packet_size)) + packet_size) {
        fail();
        return 0;
    }

    // Extract the packet from the buffer.
    buffer().consume(sizeof(packet_size));
    Buffer* packet = new Buffer(buffer().begin(), packet_size);
    buffer().consume(packet_size);
    return packet;
}
