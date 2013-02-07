// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_message_reader
#define included_ares_message_reader

#include "ares/buffer.hpp"
#include "ares/buffer_formatter.hpp"
#include "ares/sink.hpp"
#include <cstring>
#include <string>

namespace ares {

// A buffer formatter that parses messages from an input buffer. The messages
// must be in the same format as those created by the Message_writer class.
// See that class's documentation for more details.
class Message_reader : public Buffer_formatter {
  public:
    // Constructs a message reader given a Sink to which messages can be sent.
    Message_reader(Sink& sink);

    // Parses messages from `input` and sends them to the sink passed to this
    // object's constructor. The messages are removed from `input` as they are
    // sent. Returns the number of complete messages sent to the sink.
    //
    // If this function raises a Message_size_exceeded_error exception, the
    // communication channel can be kept open; subsequent calls will discard
    // packets that are part of the problematic message. If this function
    // raises any other exception type, however, the communication channel
    // will enter an invalid state and further use of it will have undefined
    // results.
    //
    // Throws a Packet_size_exceeded_error if an input packet exceeds the
    // maximum size (see Message_reader::max_packet_size).
    //
    // Throws a Message_size_exceeded_error if an input message exceeds the
    // maximum size (see (see Message_reader::max_message_size).
    //
    // Throws a Packet_sequence_error if an input packet is received out of
    // sequence.
    int read_messages(Buffer& input);

    // Sets the maximum packet size allowed by this object. The default packet
    // size limit is 1KB. Any input packets that exceed n bytes in length will
    // cause Message_reader::read_messages to raise an exception. Note that if
    // n is less than one, the size limit will be set to one.
    void set_max_packet_size(int n);

    // Sets the maximum message size allowed by this object. The default
    // message size limit is 1MB. Any input messages that exceed n bytes in
    // length will cause Message_reader::read_messages to raise an exception.
    // Note that if n is less than one, the size limit will be set to one.
    void set_max_message_size(int n);

    // Sets the optimal capacity of this object's internal memory buffer. This
    // function allows the user to prevent a message reader from holding onto
    // excessive amounts of memory. By default, the retained size is 1MB.
    //
    // During a call to Message_reader::read_messages, this object may
    // allocate a large contiguous block of memory in order to store unusually
    // long messages. By specifying a retained size lower than the maximum
    // allowed message size, the user can force a reader to release some
    // memory after excessive growth.
    //
    // It is generally advisable to set the retained size to a value slightly
    // greater than the average expected message size.
    void set_retained_size(int n);

    // Returns the maximum packet size allowed by this object. This value can
    // be modified by calling Message_reader::set_max_packet_size.
    int max_packet_size() const { return m_max_packet_size; }

    // Returns the maximum message size allowed by this object. This value can
    // be modified by calling Message_reader::set_max_message_size.
    int max_message_size() const { return m_max_message_size; }

    // Returns the number of messages successfully read by the most recent
    // call to Message_reader::read_messages. This function is provided in
    // case that function raises an exception after successfully reading one
    // or more messages.
    int num_messages() const { return m_num_messages; }

  private:
    Sink& m_sink;           // where to send message buffers
    Buffer m_buffer;        // for storing partial messages
    int m_max_packet_size;  // maximum size of a packet
    int m_max_message_size; // maximum size of a message
    int m_retained_size;    // target size of m_buffer after heavy growth
    int m_seq_num;          // for ordering packets in a sequence
    int m_num_messages;     // # successfully read by last read_messages call
    bool m_overflow;        // set when an input message was too big
};

} // namespace ares

#endif
