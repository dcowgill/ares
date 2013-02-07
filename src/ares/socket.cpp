// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/socket.hpp"
#include "ares/net_tk.hpp"
#include "ares/platform.hpp"
#include "ares/string_util.hpp"

using namespace std;
using ares::Socket;

Socket::Socket(Sockfd s, string const& remote_addr, int remote_port)
        : m_handle(s)
        , m_remote_addr(remote_addr)
        , m_remote_port(remote_port)
        , m_created(Date::now())
        , m_is_blocking(true)
        , m_num_bytes_received(0)
        , m_num_bytes_sent(0)
{
    // We always disable Nagle's algorithm by default.
    set_tcp_no_delay(true);
}

Socket::~Socket()
{
    net_tk::close_socket(m_handle);
}

int Socket::read(Byte* dest, int count)
{
    int n = net_tk::read_tcp(m_handle, dest, count);
    m_num_bytes_received += n;
    return n;
}

int Socket::read(Buffer& b)
{
    int n = read(b.end(), b.free());
    if (n > 0) b.advance(n);
    return n;
}

int Socket::write(Byte const* data, int count)
{
    int n = net_tk::write_tcp(m_handle, data, count);
    m_num_bytes_sent += n;
    return n;
}

int Socket::write(Buffer const& b)
{
    return write(b.begin(), b.size());
}

int Socket::write_all(Byte const* data, int count)
{
    int n = net_tk::write_all_tcp(m_handle, data, count);
    assert(n == count);
    m_num_bytes_sent += n;
    return n;
}

int Socket::write_all(Buffer const& b)
{
    return write_all(b.begin(), b.size());
}

void Socket::set_blocking(bool on)
{
    if (is_blocking() != on) {
        net_tk::set_blocking(m_handle, on);
        m_is_blocking = on;
    }
}

void Socket::set_tcp_no_delay(bool on)
{
    net_tk::set_tcp_no_delay(m_handle, on);
}

string Socket::to_string() const
{
    return format("%s:%d", remote_address().c_str(), remote_port());
}


//
// FREE FUNCTIONS
//

Socket* ares::connect_tcp(string const& address,
                          string const& port,
                          int timeout_millis)
{
    string remote_addr;
    int remote_port;

    Sockfd sockfd = net_tk::connect_tcp(address.c_str(),
                                        port.c_str(),
                                        &remote_addr,
                                        &remote_port,
                                        timeout_millis);

    return new Socket(sockfd, remote_addr, remote_port);
}
