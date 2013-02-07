// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/packet_writer.hpp"
#include "ares/buffer.hpp"
#include "ares/data_writer.hpp"
#include "ares/bin_util.hpp"
#include <cstring>

using namespace std;

ares::Packet_writer::Packet_writer(Buffer& b)
        : Buffer_formatter(b)
{}

ares::Packet_writer::~Packet_writer()
{}

bool ares::Packet_writer::begin_packet()
{
    if (!*this)
        return false;

    m_original_size = buffer().size();  // don't count existing data
    if (buffer().free() < int(sizeof(Int32))) {
        fail();
        return false;
    }
    pack_int32(buffer().begin(), 0);
    buffer().advance(sizeof(Int32));
    return true;
}

bool ares::Packet_writer::end_packet()
{
    if (!*this)
        return false;

    int size = buffer().size() - m_original_size - sizeof(Int32);
    buffer().reverse(size + sizeof(size));
    pack_int32(buffer().begin(), size);
    buffer().advance(size + sizeof(size));
    return true;
}
