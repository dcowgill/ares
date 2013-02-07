// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_message_writer
#define included_ares_message_writer

#include "ares/buffer.hpp"
#include "ares/bytes.hpp"
#include "ares/packet_writer.hpp"
#include "ares/sink.hpp"
#include "ares/types.hpp"
#include <cstring>
#include <string>

namespace ares {

// A buffer formatter that gracefully handles overflow of its internal Buffer
// object, and is otherwise similar to Data_writer and Packet_writer in its
// interface. Message_writer works by requiring a Sink object to which it can
// send the contents of its internal buffer whenever overflow occurs. We refer
// to the overflow buffers as physical packets, or more simply _packets_.
// While the _message_ comprises the data the user wishes to send to the sink,
// the packets are the chunks of data into which the message is divided when
// overflow occurs. Each packet has the following structure:
//
//  +------+---------+------------+---------+
//  | size | seq_num | is_chained | data... |
//  +------+---------+------------+---------+
//
// size is a 4-byte integer that indicates the number of bytes in the packet,
// excluding size itself (in other words, the combined size of seq_num,
// is_chained, and data).
//
// seq_num is a 2-byte integer containing the packet's sequence number, which
// begins at zero and increments by one for each subsequent packet describing
// the same message (which is delimited by calls to
// Message_writer::begin_message and Message_writer::end_message). This value
// is present only as a sanity-check, since packets for different messages
// should never be interleaved (see the warning below).
//
// is_chained is a single byte. It is zero when the current packet is the
// final one in a sequence that comprises a single message. Otherwise, the
// message is incomplete and another packet follows.
//
// data is a sequence of bytes containing the message data. When the packet
// headers have been discarded, their data sections can be concatenated to
// form the message.
//
// The maximum size of the individual packets can be changed using the
// Message_writer::set_max_packet_size function, although the size must be
// within the range described by Message_writer::MIN_PACKET_SIZE and
// Message_writer::MAX_PACKET_SIZE.
//
// The rationale for Message_writer is that some programs need to share a
// limited pool of memory among a large number of clients, while also sending
// large messages to those clients. This class simplifies the task of sending
// such large messages using a single, much smaller memory buffer. The sink
// object then becomes responsible for accepting, buffering, and sending those
// packets.
//
// Note: multi-byte integers are transmitted in network byte order. See the
// Data_writer documentation for more information.
//
// Warning: sending to the sink pointed to by a Message_writer object between
// calls to Message_writer::begin_message and Message_writer::end_message must
// be avoided. Otherwise, the sequence of packets sent to the sink by the
// Message_writer will be hopelessly confused, making it impossible to
// recombine them into a valid message.
class Message_writer : public Buffer_formatter {
  public:
    // Constructs a stream packet writer. Until the Message_writer::set_sink
    // function is called, output packets will be discarded when overflow
    // occurs or the Message_writer::end_message function is called.
    Message_writer();

    // Constructs a stream packet writer given an initial output sink.
    Message_writer(Sink& sink);

    // Destructor.
    virtual ~Message_writer();

    // Specifies the destination for output packets.
    void set_sink(Sink& sink);

    // Sets the capacity of the internal buffer, and therefore the maximum
    // size of output packets. If n is less than
    // Message_writer::MIN_PACKET_SIZE or greater than
    // Message_writer::MAX_PACKET_SIZE, it will be brought into range as
    // expected.
    void set_max_packet_size(int n);

    // Begins a new logical packet. This call must eventually be followed by a
    // call to Message_writer::end_message, which ends the logical packet.
    void begin_message();

    // Ends the logical packet begun by a prior call to
    // Message_writer::begin_message. Any intervening writes operations
    // constitute the actual packet data. Undefined if called out of sequence
    // (that is, if it does not follow a prior call to
    // Message_writer::begin_message).
    void end_message();

    // Adds an 8-bit integer to the message.
    void put_int8(Int8 n);

    // Adds an unsigned 8-bit integer to the message.
    void put_uint8(Uint8 n);

    // Adds a 16-bit integer to the message.
    void put_int16(Int16 n);

    // Adds an unsigned 16-bit integer to the message.
    void put_uint16(Uint16 n);

    // Adds a 32-bit integer to the message.
    void put_int32(Int32 n);

    // Adds an unsigned 32-bit integer to the message.
    void put_uint32(Uint32 n);

    // Adds a null-terminated string to the message. This function is
    // semantically equivalent to put_string(s,strlen(s)).
    void put_string(char const* s);

    // Adds a string to the message. This function is semantically equivalent
    // to put_string(s.data(),s.length()).
    void put_string(std::string const& s);

    // Writes the value of `len` to the message exactly as
    // Message_writer::put_int32 would, then writes `len` bytes from `data` to
    // the message.
    void put_bytes(Byte const* data, int len);

    // Adds the contents of a byte array to the message. This function is
    // semantically equivalent to put_string(bytes.begin(),bytes.length()).
    void put_bytes(Bytes const& bytes);

    // Returns the current capacity of the internal buffer, and therefore the
    // maximum size of output packets.
    int max_packet_size() const { return m_buffer.capacity(); }

    enum {
        DEFAULT_PACKET_SIZE = 4*1024, // default output packet size
        MIN_PACKET_SIZE = 8,          // minimum output packet size
        MAX_PACKET_SIZE = 512*1024,   // maximum output packet size
    };

  private:
    void put(Byte const* data, int count);
    void write_header();
    void clear_buffer();

  private:
    Sink* m_sink;                   // destination for full buffers
    Buffer m_buffer;                // the output buffer
    Packet_writer m_packet_writer;  // for writing packet headers
    int m_seq_num;                  // for ordering packets in a sequence
    int m_mark;                     // marks start of packet header
};

// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

inline void Message_writer::put_string(char const* s)
{
    put_bytes((Byte const*) s, std::strlen(s));
}

inline void Message_writer::put_string(std::string const& s)
{
    put_bytes((Byte const*) s.data(), s.length());
}

inline void Message_writer::put_bytes(Bytes const& bytes)
{
    put_bytes(bytes.begin(), bytes.size());
}

} // namespace ares

#endif
