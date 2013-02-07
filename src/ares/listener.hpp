// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_listener
#define included_ares_listener

// This is an implementation file; do not use directly.

#include "ares/command_queue.hpp"
#include "ares/component.hpp"
#include "ares/service.hpp"
#include "ares/socket_acceptor.hpp"

namespace ares {

class Server_interface;
class Service;

// Listener is the framework component responsible for accepting incoming
// connections and constructing new sessions.
class Listener : public Component {
  public:
    // Constructs a listener for a specified service.
    Listener(Service& service, Server_interface& server, int id);

    // Destroys this object and closes its listen socket.
    ~Listener();

  private:
    void run();

  private:
    Service& m_service;             // the listen service
    Server_interface& m_server;     // server interface
    int const m_id;                 // unique ID assigned to this listener
    Socket_acceptor m_acceptor;     // socket acceptor/factory
    std::vector<Socket*> m_sockets; // for passing to acceptor
};

} // namespace ares

#endif
