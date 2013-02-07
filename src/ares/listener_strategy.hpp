// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_listener_strategy
#define included_ares_listener_strategy

namespace ares {

class Server_interface;
class Socket;

// Listener_strategy is an interface for application-specific connection
// handling. When a Listener object receives a new connection, it forwards the
// new socket to its strategy object, along with a reference to a
// Server_interface instance through which the strategy may interact with the
// server.
class Listener_strategy {
  public:
    virtual ~Listener_strategy() {}

    // This function is called when a listener detects an incoming connection.
    // It can perform whatever application-level tasks are necessary when a
    // new connection is first established.
    virtual void handle_connection(Server_interface& server,
                                   Socket* socket) = 0;
};

} // namespace ares

#endif
