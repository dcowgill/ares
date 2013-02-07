// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/message_writer.hpp"
#include "ares/bin_util.hpp"

using namespace std;

ares::Message_writer::Message_writer()
        : Buffer_formatter(m_buffer)
        , m_sink(0)
        , m_buffer(DEFAULT_PACKET_SIZE)
        , m_packet_writer(m_buffer)
{}

ares::Message_writer::Message_writer(Sink& sink)
        : Buffer_formatter(m_buffer)
        , m_sink(&sink)
        , m_buffer(DEFAULT_PACKET_SIZE)
        , m_packet_writer(m_buffer)
{}

ares::Message_writer::~Message_writer()
{}

void ares::Message_writer::set_sink(Sink& sink)
{
    m_sink = &sink;
}

void ares::Message_writer::set_max_packet_size(int n)
{
    // Adjust new packet size if out of bounds.
    if (n < MIN_PACKET_SIZE) {
        n = MIN_PACKET_SIZE;
    }
    else if (n > MAX_PACKET_SIZE) {
        n = MAX_PACKET_SIZE;
    }

    // Change the buffer capacity, compacting if capacity was reduced.
    int old_capacity = m_buffer.capacity();
    m_buffer.set_capacity(n);
    if (old_capacity > n) {
        Buffer(n).swap(m_buffer);   // compacts buffer
    }
}

void ares::Message_writer::begin_message()
{
    m_seq_num = 0;
    clear_buffer();
    write_header();
}

void ares::Message_writer::end_message()
{
    m_packet_writer.end_packet();
    if (m_sink) m_sink->send(m_buffer);
}

void ares::Message_writer::put_int8(Int8 n)
{
    Byte b = static_cast<Byte>(n);
    put(&b, 1);
}

void ares::Message_writer::put_uint8(Uint8 n)
{
    Byte b = static_cast<Byte>(n);
    put(&b, 1);
}

void ares::Message_writer::put_int16(Int16 n)
{
    Byte buf[sizeof(n)];
    pack_int16(buf, n);
    put(buf, sizeof(buf));
}

void ares::Message_writer::put_uint16(Uint16 n)
{
    Byte buf[sizeof(n)];
    pack_uint16(buf, n);
    put(buf, sizeof(buf));
}

void ares::Message_writer::put_int32(Int32 n)
{
    Byte buf[sizeof(n)];
    pack_int32(buf, n);
    put(buf, sizeof(buf));
}

void ares::Message_writer::put_uint32(Uint32 n)
{
    Byte buf[sizeof(n)];
    pack_uint32(buf, n);
    put(buf, sizeof(buf));
}

void ares::Message_writer::put_bytes(Byte const* data, int len)
{
    /*
      Byte buf[sizeof(Uint32) + 1];
      put(buf, compress_uint32(but, len));
    */
    put_int32(len);
    put(data, len);
}

void ares::Message_writer::put(Byte const* data, int count)
{
    do {
        if (m_buffer.free() >= count) {
            m_buffer.put(data, count);
            break;
        }
        else {
            int n = m_buffer.free();
            m_buffer.put(data, n);
            data += n;
            count -= n;
            int pos = m_buffer.mark();
            m_buffer.reset(m_mark);
            m_buffer.end()[0] = 1;      // message is chained
            m_buffer.reset(pos);
            end_message();
            clear_buffer();
            write_header();
        }
    } while (count > 0);
}

void ares::Message_writer::write_header()
{
    put_int16(m_seq_num++);
    m_mark = m_buffer.mark();
    put_int8(0); // assume this will be final packet in seq.
}

void ares::Message_writer::clear_buffer()
{
    m_buffer.clear();
    m_packet_writer.begin_packet();
}
