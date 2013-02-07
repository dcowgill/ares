// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/dispatcher.hpp"
#include "ares/command.hpp"
#include "ares/error.hpp"
#include "ares/guard.hpp"
#include "ares/log.hpp"
#include "ares/platform.hpp"
#include "ares/server_interface.hpp"
#include "ares/socket.hpp"
#include "ares/string_util.hpp"
#include "ares/trace.hpp"

using namespace std;

ares::Dispatcher::Dispatcher(Server_interface& server)
        : Component("dispatcher")
        , m_server(server)
        , m_dispatch_queue(1000)  // FIXME kludge to prevent overloading
        , m_last_snapshot(current_time())
        , m_num_buffers(0)
        , m_total_output_bytes(0)
        , m_total_output_bytes_left(0)
        , m_buffers_added(0)
        , m_buffers_sent(0)
        , m_writes(0)
        , m_zero_writes(0)
        , m_bytes_sent(0)
{}

ares::Dispatcher::~Dispatcher()
{}

void ares::Dispatcher::dispatch(Session c, Buffer* bp)
{
    m_dispatch_queue.enqueue(make_pair(c, Shared_buffer(bp)));
}

ares::Dispatcher_statistics ares::Dispatcher::statistics()
{
    Dispatcher_statistics stats;

    time_t current_time = ares::current_time();
    stats.m_elapsed_sec = current_time - m_last_snapshot;
    m_last_snapshot = current_time;

    stats.m_sessions_snap = m_sessions.size();
    stats.m_queued_dispatches_snap = m_dispatch_queue.size();
    stats.m_buffers_snap = m_num_buffers;
    stats.m_outbound_snap = m_total_output_bytes;
    stats.m_outbound_remaining_snap = m_total_output_bytes_left;
    stats.m_buffers_added = m_buffers_added;
    stats.m_buffers_sent = m_buffers_sent;
    stats.m_writes = m_writes;
    stats.m_zero_writes = m_zero_writes;
    stats.m_bytes_sent = m_bytes_sent;

    {
        Guard guard(m_lock);   // lock to update stats
        m_buffers_added = 0;
        m_buffers_sent = 0;
        m_writes = 0;
        m_zero_writes = 0;
        m_bytes_sent = 0;
    }

    return stats;
}

void ares::Dispatcher::run() try
{
    Trace::set_thread_name("dispatcher");

    while (!is_stopped()) {
        bool empty = m_sessions.empty(); // are we idle?
        int timeout = empty ? 500 : 0;  // block only if dispatcher is idle

        // Dequeue all available dispatches
        int count = m_dispatch_queue.dequeue_all(m_dispatches, timeout);
        if (count <= 0 && empty) {
            continue;  // nothing to do
        }

        Guard guard(m_lock);  // lock before modifying shared data

        // Process the dequeued dispatches
        for (int i = 0; i < count; i++) {
            add_dispatch(m_dispatches[i]);
        }
        m_num_buffers += count;
        m_dispatches.clear();

        // Process outgoing dispatches
        Session_map::iterator end(m_sessions.end());
        for (Session_map::iterator iter(m_sessions.begin()); iter != end; ) {
            try {
                write_dispatches(iter);
            }
            catch (IO_error& e) {
                Session s = iter->first;
                Log::writef(Log::NOTICE, "dispatcher: i/o error writing to "
                            "session (%s), killing", s->to_string().c_str());

                m_server.enqueue_command(new Remove_session_command(s));
                m_sessions.erase(iter++);
            }
        }
    }
}
catch (...) {
    Log::write(Log::ERROR, "dispatcher: unexpected exception, shutting down");
    shutdown();
}

void ares::Dispatcher::add_dispatch(Pending_dispatch p)
{
    // p is a (Session, Shared_buffer) pair.
    m_sessions[p.first].push_back(Dispatch(0, p.second));

    // Update statistics.
    int buf_size = p.second->size();
    m_total_output_bytes += buf_size;
    m_total_output_bytes_left += buf_size;
    m_buffers_added++;
}

void ares::Dispatcher::write_dispatches(Session_map::iterator& iter)
{
    Session session = iter->first;
    Dispatch_list& dispatch_list = iter->second;
    assert(!dispatch_list.empty());

    while (!dispatch_list.empty()) {
        Dispatch& dispatch = dispatch_list.front();
        int& pos = dispatch.first;
        Shared_buffer& buf = dispatch.second;
        int buf_size = buf->size();

        assert(pos >= 0 && pos < buf_size);
        int n = session->socket().write(buf->begin() + pos, buf_size - pos);
        m_writes++;

        if (n == 0) {
            m_zero_writes++;
            break;
        }

        m_bytes_sent += n;
        m_total_output_bytes_left -= n;
        pos += n;

        assert(pos >= 0 && pos <= buf_size);
        if (pos == buf_size) {
            dispatch_list.pop_front();
            m_buffers_sent++;
            m_num_buffers--;
            m_total_output_bytes -= buf_size;
        }
    }

    if (dispatch_list.empty())
        m_sessions.erase(iter++);
    else
        ++iter;
}

double ares::Dispatcher_statistics::writes_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*writes()/elapsed_sec();
}

double ares::Dispatcher_statistics::zero_writes_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*zero_writes()/elapsed_sec();
}

double ares::Dispatcher_statistics::bytes_sent_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*bytes_sent()/elapsed_sec();
}

int ares::Dispatcher_statistics::bytes_per_write() const
{
    return writes() == 0 ? 0 : bytes_sent()/writes();
}

double ares::Dispatcher_statistics::buffers_added_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*buffers_added()/elapsed_sec();
}

double ares::Dispatcher_statistics::buffers_sent_per_sec() const
{
    return elapsed_sec() == 0 ? 0 : 1.0*buffers_sent()/elapsed_sec();
}
