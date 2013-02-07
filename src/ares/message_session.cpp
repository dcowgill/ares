// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/message_session.hpp"
#include "ares/command.hpp"
#include "ares/guard.hpp"
#include "ares/server_interface.hpp"
#include "ares/socket.hpp"
#include "ares/trace.hpp"
#include <memory>

using namespace std;
using ares::Message_session;

// This class insures that, even if an exception is thrown while processing a
// message, the remaining messages will be processed by the server. Note that
// this class begins in the disabled state and must be explicitly enabled;
// this allows the caller to fine-tune the scheduling of the destructor call.
class Message_session::Post_processing {
  public:
    Post_processing(Message_session* sess)
            : m_session(*sess)
            , m_enabled(false)
    {}

    ~Post_processing()
    {
        if (m_enabled && !m_session.m_message_queue.is_empty()) {
            m_session.server().enqueue_command(
                new Process_session_command(&m_session));
        }
    }

    void enable()
    {
        m_enabled=true;
    }

  private:
    Message_session& m_session;
    bool m_enabled;
};


Message_session::Message_session(Server_interface& server, Socket* socket)
        : Session_rep(server, socket)
        , m_sink(*this)
        , m_reader(m_sink)
{}

void Message_session::set_max_packet_size(int n)
{
    m_reader.set_max_packet_size(n);
}

void Message_session::set_max_message_size(int n)
{
    m_reader.set_max_message_size(n);
}

void Message_session::set_retained_size(int n)
{
    m_reader.set_retained_size(n);
}

bool Message_session::do_handle_input(Buffer& input_buffer)
{
    int total_messages = 0;

    for (;;) {
        try {
            if (m_reader.read_messages(input_buffer) == 0) {
                break;
            }
        }
        catch (Packet_size_exceeded_error& e) {
            if (!handle_error(e))
                return false;
        }
        catch (Message_size_exceeded_error& e) {
            if (!handle_error(e))
                return false;
        }
        catch (Packet_sequence_error& e) {
            if (!handle_error(e))
                return false;
        }
        total_messages += m_reader.num_messages();
    }

    if (total_messages > 0) {
        Command* cmd = new Process_session_command(this);
        server().enqueue_command(cmd);
        ARES_TRACE(("read %d message(s) [%s] [cmd=%p]", total_messages,
                    socket().to_string().c_str(), cmd));
    }

    // If the input buffer is full but we did not get a message, we must do
    // something (presumably one of [1] expand the input buffer, [2] invoke a
    // custom error routine, or [3] throw an exception).

    if (input_buffer.free() == 0 && total_messages == 0) {
        if (!handle_input_buffer_too_small(input_buffer))
            return false;  // close the session

        if (input_buffer.free() == 0)  // if true, error handler didn't help
            throw Input_buffer_too_small_error(input_buffer.capacity());
    }

    return true;
}

void Message_session::do_handle_processing(int pid)
{
    // There is one tricky aspect to this function: we must make sure that the
    // Post_processing destructor is called AFTER we release the lock (i.e.
    // after the Guard destructor is called); otherwise, there is a race
    // condition. Therefore, we must construct the Post_processing object
    // before constructing the lock-guard, but enable it only after the lock
    // has been successfully acquired.

    ARES_TRACE(("callback: handle_processing"));

    Post_processing post_processing(this);  // post-processing actions

    Guard guard(m_process_lock, false);
    if (!guard.acquired()) {                // prevent concurrent processing
        ARES_TRACE(("concurrent processing abort"));
        return;
    }

    post_processing.enable();               // OK, lock is acquired

    int count = m_message_queue.size();     // process a fixed # of messages
    for (int i = 0; i < count; i++) {
        auto_ptr<Buffer> message(m_message_queue.dequeue());
        ARES_TRACE(("processing %d-byte message", message->size()));
        process_message(*message, pid);
    }
}

bool Message_session::handle_error(Packet_size_exceeded_error& e)
{
    throw e;
}

bool Message_session::handle_error(Message_size_exceeded_error& e)
{
    throw e;
}

bool Message_session::handle_error(Packet_sequence_error& e)
{
    throw e;
}

bool Message_session::handle_input_buffer_too_small(Buffer& input_buffer)
{
    if (input_buffer.capacity() >= m_reader.max_packet_size())
        throw Input_buffer_too_small_error(input_buffer.capacity());

    input_buffer.set_capacity(m_reader.max_packet_size());
    return true;
}
