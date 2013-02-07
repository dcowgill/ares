// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/socket_acceptor.hpp"
#include "ares/error.hpp"
#include "ares/net_tk.hpp"
#include "ares/socket.hpp"
#include "ares/trace.hpp"
#include <cassert>
#include <cerrno>
#include <cstdio>

using namespace std;
using ares::Socket_acceptor;

Socket_acceptor::Socket_acceptor()
        : m_handle(net_tk::null_socket())
{}

Socket_acceptor::~Socket_acceptor() try
{
    if (is_bound())
        close();
}
catch (...) {}

void Socket_acceptor::bind(string address, string port)
{
    if (is_bound()) {
        throw Listen_socket_already_bound_error();
    }

    int sock_addr_len;
    try {
        m_handle = net_tk::listen_tcp(address.c_str(),
                                      port.c_str(),
                                      &sock_addr_len);
    }
    catch (Network_error& cause) {
        Bind_failed_error e(address.c_str(), port.c_str());
        e.set_cause(cause);     // translate exception
        throw e;
    }

    assert(m_handle != net_tk::null_socket());
    net_tk::set_blocking(m_handle, false);
    m_sock_addr.resize(sock_addr_len);

    if (!m_poller.add(m_handle, Sockfd_poller::EVENT_READABLE, m_handler)) {
        throw Internal_error("sd", "unexpected failure", m_handle);
    }
}

void Socket_acceptor::close()
{
    if (!is_bound()) {
        throw Listen_socket_not_bound_error();
    }
    m_poller.remove(m_handle);
    net_tk::close_socket(m_handle);
    m_handle = net_tk::null_socket();
}

int Socket_acceptor::wait_for_connection(int millis, vector<Socket*>& set)
{
    if (!is_bound()) {
        throw Listen_socket_not_bound_error();
    }

    set.resize(0);

    if (m_poller.wait_for_event(millis) == 0) {
        return 0;
    }

    for (;;) {
        int port;
        Sockfd s = net_tk::accept(m_handle,
                                  &m_sock_addr[0],
                                  m_sock_addr.size(),
                                  &m_address,
                                  &port);

        if (s == net_tk::null_socket())
            break;

        set.push_back(new Socket(s, m_address, port));
    }

    return set.size();
}

bool Socket_acceptor::is_bound() const
{
    return m_handle != net_tk::null_socket();
}
