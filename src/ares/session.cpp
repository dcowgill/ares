// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/session.hpp"
#include "ares/buffer.hpp"
#include "ares/mutex.hpp"
#include "ares/sequence.hpp"
#include "ares/server_interface.hpp"
#include "ares/socket.hpp"
#include "ares/string_util.hpp"
#include "ares/trace.hpp"

using namespace std;
using ares::Session_info;
using ares::Session_rep;

namespace
{
// Parameterize a Sequence with a mutex lock to insure that generated ID
// numbers are never duplicated. We don't necessarily need this lock if we
// can insure that only one thread in the system ever creates Session_rep
// instances, but it's better to be safe than sorry.

typedef ares::Sequence<ares::Mutex> Locked_sequence;
Locked_sequence s_id_seq;

// Action string constants
string const ACTION_INIT       = "initializing";
string const ACTION_SHUTDOWN   = "shutting down";
string const ACTION_INPUT      = "processing input";
string const ACTION_PROCESSING = "processing";
string const ACTION_IDLE       = "idle";
}

Session_rep::Session_rep(Server_interface& server, Socket* socket)
        : m_id(s_id_seq.next_val())
        , m_socket(socket)
        , m_server(server)
        , m_use_io_slave(false)
{
    assert(socket != 0);
    m_use_io_slave = false;
}

Session_rep::~Session_rep()
{
    ARES_TRACE(("session destructor called [%s]", to_string().c_str()));
    delete m_socket;
}

void Session_rep::send(Buffer const& buffer)
{
    if (m_use_io_slave)
        server().dispatch(this, new Buffer(buffer));
    else
        socket().write_all(buffer);
}

bool Session_rep::handle_input(Buffer& input_buffer)
{
    set_action(ACTION_INPUT);
    bool b = do_handle_input(input_buffer);
    set_action(ACTION_IDLE);
    return b;
}

void Session_rep::handle_processing(int pid)
{
    set_action(ACTION_PROCESSING);
    do_handle_processing(pid);
    set_action(ACTION_IDLE);
}

void Session_rep::handle_init(Buffer& input_buffer)
{
    set_action(ACTION_INIT);
    do_handle_init(input_buffer);
    set_action(ACTION_IDLE);
}

void Session_rep::handle_shutdown()
{
    set_action(ACTION_SHUTDOWN);
    do_handle_shutdown();
    set_action(ACTION_IDLE);
}

string Session_rep::to_string() const
{
    return format("%d@%s:%d", id(), socket().remote_address().c_str(),
                  socket().remote_port());
}


Session_info::Session_info(Session_rep const& s)
        : m_session_id(s.id())
        , m_action(s.action())
        , m_socket_handle(s.socket().handle())
        , m_is_socket_blocking(s.socket().is_blocking())
        , m_created(s.socket().created())
        , m_remote_address(s.socket().remote_address())
        , m_remote_port(s.socket().remote_port())
        , m_num_bytes_received(s.socket().num_bytes_received())
        , m_num_bytes_sent(s.socket().num_bytes_sent())
{}
