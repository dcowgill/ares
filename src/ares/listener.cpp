// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/listener.hpp"
#include "ares/error.hpp"
#include "ares/listener_strategy.hpp"
#include "ares/log.hpp"
#include "ares/platform.hpp"
#include "ares/server_interface.hpp"
#include "ares/service.hpp"
#include "ares/socket.hpp"
#include "ares/string_util.hpp"
#include "ares/trace.hpp"

using namespace std;
using ares::Listener;

Listener::Listener(Service& service, Server_interface& server, int id)
        : Component("listener", service.name())
        , m_service(service)
        , m_server(server)
        , m_id(id)
{
    // Bind our socket acceptor to our specified address and port. Let
    // exceptions propagate to the caller.

    m_acceptor.bind(m_service.address(), m_service.port());
}

Listener::~Listener()
{}

void Listener::run()
{
    int const DELAY = 250;  // yield milliseconds

    Trace::set_thread_name(format("lsnr_%d", m_id).c_str());

    // Loop until stopped, polling for queued incoming connections. Delegate
    // the action to take on a new connection to the held Listener_strategy.

    while (!is_stopped()) {
        try {
            m_acceptor.wait_for_connection(DELAY, m_sockets);
            for (int i = 0; i < int(m_sockets.size()); i++) {
                Socket* socket = m_sockets[i];
                Log::writef(Log::DEBUG,
                            "lsnr (%d): connection to %s:%s from %s", m_id,
                            m_service.address().c_str(),
                            m_service.port().c_str(),
                            socket->to_string().c_str());
                m_service.strategy()->handle_connection(m_server, socket);
            }
        }
        catch (Network_error& e) {
            Log::writef(Log::WARNING, "lsnr (%d): transient(?) network error, "
                        "pausing 5s: %s", m_id, e.to_string().c_str());
            milli_sleep(5000);
        }
        catch (Exception& e) {
            Log::writef(Log::ERROR, "lsnr (%d): unexpected error, "
                        "shutting down: %s", m_id, e.to_string().c_str());
            return;
        }
        catch (...) {
            Log::writef(Log::ERROR, "lsnr (%d): unexpected error, "
                        "shutting down", m_id);
            return;
        }
    }
}
