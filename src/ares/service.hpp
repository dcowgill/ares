// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_service
#define included_ares_service

#include <string>

namespace ares {

class Listener_strategy;

// Represents a logical service handled by a server.
class Service {
  public:
    Service(std::string const& name,
            std::string const& address,
            std::string const& port,
            Listener_strategy* strategy);

    ~Service();

    std::string const& name() const { return m_name; }
    std::string const& address() const { return m_address; }
    std::string const& port() const { return m_port; }
    Listener_strategy* strategy() const { return m_strategy; }

  private:
    std::string const m_name;       // unique name identifying this service
    std::string const m_address;    // host for which server can listen
    std::string const m_port;       // numeric port or network service name
    Listener_strategy* m_strategy;  // links framework to the application
};

} // namespace ares

#endif
