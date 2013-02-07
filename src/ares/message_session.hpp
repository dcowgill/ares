// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_message_session
#define included_ares_message_session

#include "ares/buffer.hpp"
#include "ares/message_reader.hpp"
#include "ares/mutex.hpp"
#include "ares/session.hpp"
#include "ares/sync_queue.hpp"

namespace ares {

// This class implements a message-based session in which messages must be
// processed in the order in which they are received. Note that it is an
// abstract implementation of Session_rep, i.e. it introduces new pure virtual
// functions and implements some virtual functions in Session_rep while
// leaving others unimplemented.
//
// A session implementation should derive from this class instead of from
// Session_rep if its protocol is based on "messages" (see
// ares::Message_writer for a technical description of "message") and the
// messages must be processed in the same order in which they were received.
class Message_session : public Session_rep {
  public:
    // Constructs a session.
    Message_session(Server_interface& server, Socket* socket);

  protected:
    // Sets the maximum allowable input packet size. This function allows
    // derived classes to set policy for the session.
    void set_max_packet_size(int n);

    // Sets the maximum allowable input message size. This function allows
    // derived classes to set policy for the session.
    void set_max_message_size(int n);

    // Sets the optimal capacity of the internal input message buffer. This
    // function allows derived classes to set policy for the session. In
    // general, users should try to set the retained size to a value slightly
    // greater than the expected average input message size.
    void set_retained_size(int n);

  private:
    // Inherited from Session_rep:
    bool do_handle_input(Buffer& input_buffer);
    void do_handle_processing(int pid);

    // Processes a single input message. This function is guaranteed to be
    // called once per message received, and messages will be supplied in the
    // order in which they were received. Derived classes should implement
    // this function with their session logic.
    virtual void process_message(Buffer& message, int pid) = 0;

    // Handler for a packet-size-exceeded error. This function will be called
    // automatically after reading an input packet whose size exceeds the
    // value passed to Message_session::set_max_packet_size. Derived classes
    // may implement custom error handling policies by overriding this
    // function, by either returning true, which allows processing to continue
    // normally, returning false, which closes the connection, or raising an
    // exception, which closes the connection and causes the exception to be
    // logged. By default, this function simply re-raises the exception.
    virtual bool handle_error(Packet_size_exceeded_error& e);

    // Handler for a message-size-exceeded error. This function will be called
    // automatically after reading an input message whose size exceeds the
    // value passed to Message_session::set_max_message_size. As with
    // Message_session::handle_error(Packet_size_exceeded_error&), derived
    // classes may implement custom error handling via this function. By
    // default, this function simply re-raises the exception.
    virtual bool handle_error(Message_size_exceeded_error& e);

    // Handler for a packet-size-exceeded error. This function will be called
    // automatically after reading an input packet that was sent out of
    // sequence. As with
    // Message_session::handle_error(Packet_size_exceeded_error&), derived
    // classes may implement custom error handling via this function. By
    // default, this function simply re-raises the exception.
    virtual bool handle_error(Packet_sequence_error& e);

    // This handler is invoked when the input buffer is full but does not
    // contain a complete message. Typically, the only way to recover from
    // this situation is to expand the input buffer. By default, this function
    // raises an Input_buffer_too_small_error.
    virtual bool handle_input_buffer_too_small(Buffer& input_buffer);

  private:
    // (this class provides the callback to Message_reader)
    class Message_sink : public Sink {
      public:
        Message_sink(Message_session& session)
                : m_session(session) {}

        void send(Buffer const& msg)
        { m_session.m_message_queue.enqueue(new Buffer(msg)); }

        void send(Byte const* data, int count)
        { m_session.m_message_queue.enqueue(new Buffer(data, count)); }

      private:
        Message_session& m_session;
    };

    typedef Sync_queue<Buffer*> Message_queue;

    Mutex m_process_lock;           // prevents concurrent processing
    Message_sink m_sink;            // the callback object given to m_reader
    Message_reader m_reader;        // persistent message reader
    Message_queue m_message_queue;  // queue of unprocessed messages

    class Post_processing;

    friend class Message_sink;
    friend class Post_processing;
};

} // namespace ares

#endif
