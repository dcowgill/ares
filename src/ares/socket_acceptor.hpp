// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_socket_acceptor
#define included_ares_socket_acceptor

#include "ares/sockfd_poller.hpp"
#include "ares/types.hpp"
#include "ares/utility.hpp"
#include <cassert>
#include <string>
#include <vector>

namespace ares {

class Socket;

class Socket_acceptor : boost::noncopyable {
  public:
    Socket_acceptor();
    ~Socket_acceptor();
    void bind(std::string address, std::string port);
    void close();
    int wait_for_connection(int millis, std::vector<Socket*>& set);
    bool is_bound() const;

  private:
    // (this handler is never called)
    struct Dummy_handler : public Sockfd_poller::Event_handler {
        Action operator()() { assert(0); return KEEP_EVENT; }
    };

    Sockfd m_handle;                // listen socket handle
    std::vector<char> m_sock_addr;  // socket address structure
    std::string m_address;          // storage for remote address
    Sockfd_poller m_poller;         // for non-blocking accept
    Dummy_handler m_handler;        // necessary for poller interface
};

} // namespace ares

#endif
