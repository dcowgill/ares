// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/command.hpp"
#include "ares/server_interface.hpp"
#include "ares/socket.hpp"
#include "ares/trace.hpp"

using namespace std;

ares::Command::~Command()
{}


ares::Session_command::Session_command(Session s)
        : m_session(s)
{}

ares::Session_command::~Session_command()
{}


ares::Add_session_command::Add_session_command(Session s)
        : Session_command(s)
{}

void ares::Add_session_command::execute(Server_interface& server, int)
{
    ARES_TRACE(("executing Add_session_command"));
    server.add_session(session());
}


ares::Remove_session_command::Remove_session_command(Session s)
        : Session_command(s)
{}

void ares::Remove_session_command::execute(Server_interface& server, int)
{
    ARES_TRACE(("executing Remove_session_command"));
    server.remove_session(session());
}


ares::Dispatch_command::Dispatch_command(Session s, Buffer const& b)
        : Session_command(s)
        , m_buffer(new Buffer(b))
{}

void ares::Dispatch_command::execute(Server_interface& server, int)
{
    ARES_TRACE(("executing Dispatch_command"));
    server.dispatch(session(), m_buffer);
}


ares::Process_session_command::Process_session_command(Session s)
        : Session_command(s)
{}

void ares::Process_session_command::execute(Server_interface&, int pid)
{
    ARES_TRACE(("executing Process_session_command"));
    session()->handle_processing(pid);
}
