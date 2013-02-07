// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_socket
#define included_ares_socket

#include "ares/buffer.hpp"
#include "ares/date.hpp"
#include "ares/types.hpp"
#include "ares/utility.hpp"
#include <string>

namespace ares {

class Socket : boost::noncopyable {
  public:
    virtual ~Socket();

    int read(Byte* dest, int count);
    int read(Buffer& b);
    int write(Byte const* data, int count);
    int write(Buffer const& b);
    int write_all(Byte const* data, int count);
    int write_all(Buffer const& b);
    void set_blocking(bool on);
    void set_tcp_no_delay(bool on);

    bool is_blocking() const { return m_is_blocking; }
    Sockfd handle() const { return m_handle; }
    Date created() const { return m_created; }
    std::string const& remote_address() const { return m_remote_addr; }
    int remote_port() const { return m_remote_port; }
    int num_bytes_received() const { return m_num_bytes_received; }
    int num_bytes_sent() const { return m_num_bytes_sent; }
    std::string to_string() const;

  private:
    Sockfd const m_handle;              // system-level socket handle
    std::string const m_remote_addr;    // address of the remote host
    int const m_remote_port;            // port of the remote host
    Date const m_created;               // date socket was constructed
    bool m_is_blocking;                 // true if blocking
    int m_num_bytes_received;           // count of bytes received
    int m_num_bytes_sent;               // count of bytes sent

    // Constructs a new Socket with an existing connection (which is
    // specified by the Socket descriptor). This is the only
    // constructor for Socket objects. Corollary: a Socket object by
    // definition represents an established connection.
    Socket(Sockfd s, std::string const& remote_addr, int remote_port);

    friend Socket* connect_tcp(std::string const&,std::string const&,int);
    friend class Socket_acceptor;
};


// FREE FUNCTIONS

Socket* connect_tcp(std::string const& address,
                    std::string const& service,
                    int timeout_millis = 0);

} // namespace ares

#endif
