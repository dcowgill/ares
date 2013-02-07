// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/message_reader.hpp"
#include "ares/data_reader.hpp"
#include "ares/error.hpp"

using namespace std;
using ares::Message_reader;

namespace
{
inline bool read_packet_headers(int& packet_size,
                                int& seq_num,
                                bool& chained,
                                ares::Data_reader& reader,
                                ares::Buffer& b,
                                int max_packet_size)
{
    using namespace ares;

    // Insure that there is a whole packet in the input buffer.
    packet_size = reader.peek_int32();
    if (!reader)
        return false;
    if (packet_size < 0 || packet_size > max_packet_size)
        throw Packet_size_exceeded_error(packet_size, max_packet_size);
    if (b.size() < int(sizeof(Int32)) + packet_size)
        return false;

    // Read the packet header.
    b.consume(sizeof(packet_size));
    seq_num = reader.get_int16();
    chained = reader.get_int8();
    packet_size -= sizeof(Int16) + sizeof(Int8);
    return true;
}
}

enum {
    DEFAULT_MAX_PACKET_SIZE = 1*1024,
    DEFAULT_MAX_MESSAGE_SIZE = 1024*1024,
    DEFAULT_RETAINED_SIZE = 1024*1024,
};

Message_reader::Message_reader(Sink& sink)
        : Buffer_formatter(m_buffer)
        , m_sink(sink)
        , m_max_packet_size(DEFAULT_MAX_PACKET_SIZE)
        , m_max_message_size(DEFAULT_MAX_MESSAGE_SIZE)
        , m_retained_size(DEFAULT_RETAINED_SIZE)
        , m_num_messages(0)
        , m_overflow(false)
{}

int Message_reader::read_messages(Buffer& input)
{
    Data_reader reader(input);
    int packet_size;
    int seq_num;
    bool chained;

    m_num_messages = 0;     // return value of this function

    if (m_overflow) {
        // A previous message caused a Message_size_exceeded_error exception
        // to be raised. We must now discard packets until we find one whose
        // chained-flag is zero, which indicates that the subsequent packet
        // will begin a new logical message.
        for (;;) {
            if (!read_packet_headers(packet_size, seq_num, chained, reader,
                                     input, m_max_packet_size))
                return 0;

            input.consume(packet_size);
            if (!chained) {
                m_overflow = false;
                break;
            }
        }
    }

    for (;;) {
        if (!read_packet_headers(packet_size, seq_num, chained, reader,
                                 input, m_max_packet_size))
            break;

        int msg_size = m_buffer.size() + packet_size;
        if (msg_size > m_max_message_size) {
            if (chained) m_overflow = true;
            input.consume(packet_size);
            m_buffer.clear();
            throw Message_size_exceeded_error(msg_size, m_max_message_size);
        }

        if (m_buffer.size() > 0) {  // we previously read a partial message
            if (seq_num != ++m_seq_num)
                throw Packet_sequence_error();

            m_buffer.set_min_capacity(msg_size);
            m_buffer.put(input.begin(), packet_size);

            if (!chained) {
                m_sink.send(m_buffer);
                m_buffer.clear();
                m_num_messages++;
            }
        }
        else if (!chained) {        // the whole message is available
            if (input.size() == packet_size)
                m_sink.send(input);
            else
                m_sink.send(input.begin(), packet_size);

            m_num_messages++;
        }
        else {                      // this message is chained
            m_buffer.assign(input.begin(), packet_size);
            m_seq_num = seq_num;
        }

        input.consume(packet_size);
    }

    // If our internal buffer is over our retained size, compact it.
    if (m_buffer.capacity() > m_retained_size &&
        m_buffer.size() < m_retained_size)
    {
        Buffer temp(m_retained_size);
        temp.put(m_buffer);
        temp.swap(m_buffer);
    }

    return m_num_messages;
}

void Message_reader::set_max_packet_size(int n)
{
    m_max_packet_size = n > 0 ? n : 1;
}

void Message_reader::set_max_message_size(int n)
{
    m_max_message_size = n > 0 ? n : 1;
}

void Message_reader::set_retained_size(int n)
{
    m_retained_size = n > 0 ? n : 0;
}
