// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/receiver.hpp"
#include "ares/command.hpp"
#include "ares/error.hpp"
#include "ares/guard.hpp"
#include "ares/log.hpp"
#include "ares/platform.hpp"
#include "ares/socket.hpp"
#include "ares/string_util.hpp"
#include "ares/trace.hpp"
#include "ares/utility.hpp"
#include <vector>

using namespace std;
using ares::Receiver;

Receiver::Receiver(Server_interface& server)
        : Component("receiver")
        , m_server(server)
        , m_last_snapshot(current_time())
        , m_reads(0)
        , m_bytes_read(0)
{}

Receiver::~Receiver()
{}

void Receiver::add_session(Session s)
{
    m_update_queue.enqueue(make_pair(true, s));
}

void Receiver::remove_session(Session s)
{
    m_update_queue.enqueue(make_pair(false, s));
}

int Receiver::num_sessions() const
{
    Guard guard(m_lock);
    return m_sessions.size();
}

ares::Receiver_statistics Receiver::statistics()
{
    Receiver_statistics stats;
    Guard guard(m_lock);             // lock access to m_sessions

    time_t const current_time = ares::current_time();
    stats.m_elapsed_sec = current_time - m_last_snapshot;
    m_last_snapshot = current_time;

    stats.m_reads = m_reads;
    stats.m_bytes_read = m_bytes_read;

    m_reads = 0;
    m_bytes_read = 0;

    stats.m_sessions_snap = m_sessions.size();
    stats.m_queued_updates_snap = m_update_queue.size();

    return stats;
}

void Receiver::run() try
{
    Trace::set_thread_name("receiver");
    int const DELAY = 50;             // milliseconds to wait for an event

    while (!is_stopped()) {
        // If we're not managing any sockets, check for new sessions.
        if (m_poller.num_sockets() == 0) {
            briefly_wait_for_update();
            if (m_poller.num_sockets() == 0)
                continue;
        }

        // Poll managed sockets for readiness events.
        int const num_events = m_poller.wait_for_event(DELAY);

        // Acquire an exclusive lock on the receiver.
        Guard guard(m_lock);

        // Process any events that occurred.
        if (num_events > 0)
            m_poller.process_events();

        // Process the add-remove queue.
        if (int count = m_update_queue.dequeue_all(m_updates)) {
            for (int i = 0; i < count; i++)
                process(m_updates[i]);
            m_updates.clear();
        }
    }
}
catch (Exception& e) {
    Log::writef(Log::ERROR, "rcvr: unexpected exception, shutting down: %s",
                e.to_string().c_str());
    shutdown();
}
catch (...) {
    Log::writef(Log::ERROR, "rcvr: unexpected exception, shutting down");
    shutdown();
}

void Receiver::briefly_wait_for_update()
{
    try { process(m_update_queue.dequeue(100)); } catch (Timeout_error&) {}
}

void Receiver::process(Pending_update p)
{
    // p is a (bool,Session) pair (if p.first is true, we are adding p.second)

    Session& session = p.second;

    if (p.first) {
        ARES_TRACE(("adding session [%s]", session->to_string().c_str()));

        // Create a socket event handler and store it in our internal table.
        Socket_event_handler* handler =
                new Socket_event_handler(*this, session, new Buffer);
        bool const was_inserted =
                m_sessions.insert(
                    make_pair(session->socket().handle(), handler)).second;

        if (was_inserted) {
            // Give the session a chance to initialize the input buffer.
            session->handle_init(*handler->m_buffer);

            // Add the socket to our i/o event poller.
            if (!m_poller.add(session->socket().handle(),
                              Sockfd_poller::EVENT_READABLE,
                              *handler))
            {
                ARES_PANIC(("couldn't add session [%s] to i/o event poller",
                            session->to_string().c_str()));
            }
        }
        else {
            delete handler;
            ARES_TRACE(("failed to add session: duplicate"));
            Log::writef(Log::WARNING, "rcvr: redundant Add_session_command "
                        "for session (%s)", session->to_string().c_str());
        }

        assert(int(m_sessions.size()) == m_poller.num_sockets());
    }
    else {
        ARES_TRACE(("removing session [%s]", session->to_string().c_str()));

        // Note: we must not remove the socket from the i/o poller unless we
        // find the session in our registry first; although session ids are
        // not reused, socket handles (which the poller uses as a key) may be
        // reused immediately.

        Session_map::iterator i = m_sessions.find(session->socket().handle());
        if (i != m_sessions.end()) {
            delete i->second;           // delete the socket event handler
            session->handle_shutdown(); // call session's shutdown handler
            m_sessions.erase(i);
            m_poller.remove(session->socket().handle());  // ok to fail
        }
        assert(int(m_sessions.size()) == m_poller.num_sockets());
    }
}


Receiver::Socket_event_handler::~Socket_event_handler()
{
    delete m_buffer;
}

Receiver::Socket_event_handler::Action
Receiver::Socket_event_handler::operator()()
{
    // First, we fill the session's input buffer, watching for an i/o
    // exception (if one is raised, we kill the session). After the physical
    // read, we invoke the session's input callback. Finally, if the session
    // callback returns false, we kill the session.

    bool remove = false;
    Action action = DISCARD_EVENT;

    try {
        // Try to fill the session's input buffer.
        int const free_space = m_buffer->free();
        int const n = m_session->socket().read(*m_buffer);
        m_receiver.m_reads++;

        if (n > 0) {                        // successfully read n bytes
            m_receiver.m_bytes_read += n;
            if (!m_session->handle_input(*m_buffer))
                remove = true;
            else if (n == free_space)
                action = KEEP_EVENT;
        }
        else if (n < 0)                     // end-of-file received
            remove = true;
        else if (n == 0 && free_space == 0) // input buffer was full
            action = KEEP_EVENT;
    }
    catch (IO_error& e) {
        remove = true;
        Log::writef(Log::NOTICE, "rcvr: i/o error reading from session (%s), "
                    "closing connection: %s", m_session->to_string().c_str(),
                    e.to_string().c_str());
    }
    catch (Exception& e) {
        remove = true;
        Log::writef(Log::WARNING, "rcvr: error: %s", e.to_string().c_str());
    }

    if (remove) {
        // Note: we don't actually need to return REMOVE_SOCKET from this
        // function because the receiver will automatically remove sockets
        // from the i/o poller when they are closed.
        m_receiver.process(make_pair(false, m_session));
    }

    return action;
}


double ares::Receiver_statistics::reads_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*reads()/elapsed_sec();
}

double ares::Receiver_statistics::bytes_read_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*bytes_read()/elapsed_sec();
}

int ares::Receiver_statistics::bytes_per_read() const
{
    return reads() == 0 ? 0 : bytes_read()/reads();
}
