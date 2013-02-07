// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/net_tk.hpp"
#include "ares/error.hpp"
#include "ares/network_common.hpp"
#include "ares/utility.hpp"

// Include implementation of inet_pton and inet_ntop if this platform doesn't
// have them.

#ifndef HAVE_INET_PTON
extern "C" {
#include "inet_pton.c"
}
#endif

#ifndef HAVE_INET_NTOP
extern "C" {
#include "inet_ntop.c"
}
#endif

using namespace std;

namespace
{
// Tests if an errno value represents a transient send error on a
// non-blocking socket.
inline bool is_transient_send_error(int err_num)
{
    return errno==EAGAIN || errno==EWOULDBLOCK || errno==EINTR;
}

// Returns true if a given socket handle was just connected successfully,
// false otherwise. This function _must_ called after any apparently
// successful non-blocking connect to verify that it did in fact succeed.
bool verify_connect_succeeded(ares::Sockfd sockfd)
{
    int error = 0;
    socklen_t error_size = sizeof(error);

    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &error_size) != 0)
        return false;  // failed to get socket error option

    return (error == 0 || error == EISCONN);
}

// Perform a non-blocking connect with a timeout specified in
// milliseconds. Returns:
//
//    - success: [0, false]
//    - failure: [errno, false]
//    - timeout: [1, true]
//
pair<int,bool> connect_with_timeout(ares::Sockfd sockfd,
                                    struct addrinfo* aip,
                                    int timeout_millis)
{
    // Put the socket in non-blocking mode, then try to connect. If the
    // connect succeeds immediately, great; otherwise, if the error is
    // EINPROGRESS (or its equivalent), we use select(2) to wait the
    // specified period of time for the socket to become writable, at
    // which point we will know whether the connection succeeded.

    ares::net_tk::set_blocking(sockfd, false);

    int n = connect(sockfd, aip->ai_addr, aip->ai_addrlen);
    int connect_errno = errno;  // save errno from the connect(2) call

    if (n == 0)
        goto success;
    else if (errno==EINPROGRESS || errno==EWOULDBLOCK || errno==EAGAIN) {
#if defined(HAVE_SELECT)
        fd_set fdset;
        fd_set error_fdset;

        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);

        FD_ZERO(&error_fdset);
        FD_SET(sockfd, &error_fdset);

        struct timeval tv;
        tv.tv_sec = timeout_millis/1000;
        timeout_millis -= tv.tv_sec*1000;
        tv.tv_usec = timeout_millis*1000;

        int n = select(sockfd+1, 0, &fdset, &error_fdset, &tv);

        if (n >= 1 && !FD_ISSET(sockfd, &error_fdset))
            goto success;
        else if (n == 0)
            return make_pair(1, true);
#endif
    }
    return make_pair(connect_errno, false);

success:
    if (!verify_connect_succeeded(sockfd))
        return make_pair(connect_errno, false);
    ares::net_tk::set_blocking(sockfd, false);
    return make_pair(0, false);  // success
}

// extract_host converts the host stored in a socket address structure to
// presentation format and stores it in host, up to host_len characters.
// Returns 0 on success, else -1. On error, host will be set to an empty
// string.
int extract_host(struct sockaddr const* sa, socklen_t salen, string* host)
{
    if (!host)
        return -1;

    *host = "";  // in case of error
    vector<char> name(ares::net_tk::max_hostname_length());

    switch (sa->sa_family) {
        case AF_INET: {
            struct sockaddr_in* sin = (struct sockaddr_in*) sa;
            if (!inet_ntop(AF_INET, &sin->sin_addr, &name[0], name.size()))
                return -1;
            *host = &name[0];
            return 0;
        }
#ifdef IPV6
        case AF_INET6: {
            struct sockaddr_in6* sin6 = (struct sockaddr_in6*) sa;
            if (!inet_ntop(AF_INET6, &sin6->sin6_addr, &name[0], name.size()))
                return -1;
            *host = &name[0];
            return 0;
        }
#endif
#ifdef AF_UNIX
        case AF_UNIX: {
            struct sockaddr_un* unp = (struct sockaddr_un*) sa;
            if (unp->sun_path[0] == '\0') {
                strncpy(&name[0], "AF_UNIX: (no pathname bound)", name.size());
                name[name.size()-1] = '\0'; // strncpy doesn't always terminate
            }
            else
                snprintf(&name[0], name.size(), "%s", unp->sun_path);
            *host = &name[0];
            return 0;
        }
#endif
        default:
            ARES_PANIC(("invalid socket family (%d)", sa->sa_family));
    }
    return -1;
}

// extract_port extracts the port number stored in a socket address
// structure. Returns 0 on success, -1 on failure (for example, if the
// socket family is not supported).
int extract_port(struct sockaddr const* sa, socklen_t salen, int* port)
{
    if (!port)
        return -1;

    switch (sa->sa_family) {
        case AF_INET: {
            struct sockaddr_in* sin = (struct sockaddr_in*) sa;
            *port = (int) htons(sin->sin_port);
            return 0;
        }
#ifdef IPV6
        case AF_INET6: {
            struct sockaddr_in6* sin6 = (struct sockaddr_in6*) sa;
            *port = (int) htons(sin6->sin6_port);
            return 0;
        }
#endif
    }
    return -1;
}
}

//
// Net_util
//

int const ares::net_tk::FAMILY_DOMAIN = AF_UNIX;
int const ares::net_tk::FAMILY_INET   = AF_INET;
int const ares::net_tk::FAMILY_INET6  = AF_INET6;

ares::Sockfd ares::net_tk::null_socket()
{
    return -1;
}

ares::Sockfd ares::net_tk::connect_tcp(char const* address,
                                       char const* port,
                                       string* remote_address,
                                       int* remote_port,
                                       int timeout_millis)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6 is OK
    hints.ai_socktype = SOCK_STREAM;    // TCP socket

    struct addrinfo* result;
    if (getaddrinfo(address, port, &hints, &result) != 0)
        throw Network_error("getaddrinfo", errno); // name resolution failed

    int sockfd = -1;        // the socket handle
    int connect_errno = 0;  // errno generated by connect(2) call

    for (struct addrinfo* p = result; p != 0; p = p->ai_next) {
        // Try each socket address in turn. It is not unexpected for the call
        // to socket() to fail, as we may have passed it an IPv6 address on a
        // host that does not support IPv6.

        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0)
            continue;

        // If the caller did not specify a timeout, use a blocking connect;
        // otherwise, call connect_with_timeout() to perform a non-blocking
        // connect.

        if (timeout_millis <= 0) {
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
                goto connect_succeeded;
            connect_errno = errno;
        }
        else {
            pair<int, bool> result = // <errno, did_time_out>
                    connect_with_timeout(sockfd, p, timeout_millis);

            connect_errno = result.first;
            bool const timed_out = result.second;

            if (connect_errno == 0)
                goto connect_succeeded;

            if (timed_out) {
                ::close(sockfd);    // connection timed out
                sockfd = -1;
                break;
            }
        }

        ::close(sockfd);            // connection failed
        sockfd = -1;
        continue;

  connect_succeeded:
        extract_host(p->ai_addr, p->ai_addrlen, remote_address);
        extract_port(p->ai_addr, p->ai_addrlen, remote_port);
        set_blocking(sockfd, true);
        break;
    }

    freeaddrinfo(result);

    if (sockfd == -1)
        throw Network_error("connect", connect_errno);

    return sockfd;
}

ares::Sockfd ares::net_tk::listen_tcp(char const* address,
                                      char const* port,
                                      int* len_ptr,
                                      int backlog)
{
    int listenfd = -1;
    int n;
    struct addrinfo hints;
    struct addrinfo* result;

    // If a host is not specified (host may be null), AI_PASSIVE and AF_UNSPEC
    // will cause two socket address structures to be returned: the first for
    // IPv6 and the second for IPv4 (assuming a dual-stack host).

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;    // TCP socket

    // Call getaddrinfo() to create a list of sockaddr structures.

    if ((n = getaddrinfo(address, port, &hints, &result)) != 0)
        throw Network_error("getaddrinfo", errno);

    struct addrinfo* p = result;
    do {
        // Call the socket() and bind() functions to create the socket and
        // bind it to the address, respectively. If either one fails, we
        // ignore the current socket address and move on to the next one.

        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd < 0)
            continue;

        // We always use the SO_REUSEADDR socket option, in order to listen on
        // multiple ports for the same address, and to listen on a port that
        // is currently servicing a connection.

        int const on = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(listenfd);
    } while ((p = p->ai_next) != 0);

    if (p == 0) {
        freeaddrinfo(result);
        throw Network_error("bind", errno);
    }

    listen(listenfd, backlog);

    if (len_ptr != 0)
        *len_ptr = p->ai_addrlen;

    freeaddrinfo(result);
    return listenfd;
}

ares::Sockfd ares::net_tk::connect_udp(char const* address, char const* port)
{
    int sockfd = -1;
    int n;
    struct addrinfo hints;
    struct addrinfo* result;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;     // UDP socket

    if ((n = getaddrinfo(address, port, &hints, &result)) != 0)
        return -1;

    struct addrinfo* p = result;
    do {
        // Try each socket address in turn. It is not unexpected for the call
        // to socket() to fail, as we may have passed it an IPv6 address on a
        // host that does not support IPv6.

        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0)
            continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        close(sockfd);      // connection failed
    } while ((p = p->ai_next) != 0);

    freeaddrinfo(result);
    return sockfd;
}

ares::Sockfd ares::net_tk::listen_udp(char const* address,
                                      char const* port,
                                      int* len_ptr)
{
    int listenfd = -1;
    int n;
    struct addrinfo hints;
    struct addrinfo* result;

    // If a host is not specified (host may be null), AI_PASSIVE and AF_UNSPEC
    // will cause to socket address structures to be returned: the first for
    // IPv6 and the second for IPv4 (assuming a dual-stack host).

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;     // UDP socket

    // Call getaddrinfo() to create a list of sockaddr structures.

    if ((n = getaddrinfo(address, port, &hints, &result)) != 0) {
        return -1;
    }

    struct addrinfo* p = result;
    do {
        // Call the socket() and bind() functions to create the socket and
        // bind it to the address, respectively. If either one fails, we
        // ignore the current socket address and move on to the next one.

        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd < 0)
            continue;

        // We always use the SO_REUSEADDR socket option, in order to listen on
        // multiple ports for the same address or to listen on an address and
        // port that is currently servicing a connection.

        int const on = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        close(listenfd);
    } while ((p = p->ai_next) != 0);

    if (p == 0)
        return -1;  // the final socket() or bind() call failed

    if (len_ptr != 0)
        *len_ptr = p->ai_addrlen;

    freeaddrinfo(result);
    return listenfd;
}

void ares::net_tk::close_socket(Sockfd sock, bool linger)
{
    if (::close(sock) != 0)
        throw Network_error("close", errno);

    // TODO: support linger option.
}

ares::Sockfd ares::net_tk::accept(Sockfd listen_sock)
{
    int sockfd;

    errno = 0;
    if ((sockfd = ::accept(listen_sock, 0, 0)) < 0) {
        if (errno != EAGAIN && errno != EINTR)
            throw Network_error("accept", errno);
        return null_socket();
    }
    return sockfd;
}

ares::Sockfd ares::net_tk::accept(Sockfd listen_sock,
                                  void* addr_ptr,
                                  int addr_len,
                                  string* remote_address,
                                  int* remote_port)
{
    // This version of accept stores the peer hostname and port number in host
    // and port, respectively. It also requires storage for a socket address
    // structure of the appropriate size, addr_len, which must be provided by
    // the client (because its size is protocol-dependent).

    assert(addr_ptr && remote_address && remote_port);

    struct sockaddr* sa = (struct sockaddr*) addr_ptr;
    int sockfd;

    errno = 0;
    if ((sockfd = ::accept(listen_sock, sa, (socklen_t*) &addr_len)) < 0) {
        if (errno != EAGAIN && errno != EINTR)
            throw Network_error("accept", errno);
        return null_socket();
    }

    extract_host((struct sockaddr*) addr_ptr, addr_len, remote_address);
    extract_port((struct sockaddr*) addr_ptr, addr_len, remote_port);
    return sockfd;
}

int ares::net_tk::read_tcp(Sockfd sock, Byte* buf, int count)
{
    int n;

    errno = 0;
    if ((n = recv(sock, buf, count, 0)) < 0) {
        if (errno != EAGAIN && errno != EINTR)
            throw Network_io_error("recv", errno);
        return 0;
    }
    else if (n == 0)
        return count <= 0 ? 0 : -1;
    return n;
}

int ares::net_tk::read_all_tcp(Sockfd sock, Byte* buf, int count)
{
    int nleft = count;
    int n;

#if defined(HAVE_POLL)
    struct pollfd pollfds[1];
    pollfds[0].fd = sock;
    pollfds[0].events = POLLIN;
#elif defined(HAVE_SELECT)
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
#endif

    while (nleft > 0) {
        errno = 0;
        if ((n = read(sock, buf, nleft)) < 0) {
            if (errno != EAGAIN && errno != EINTR)
                throw Network_io_error("read", errno);
#if defined(HAVE_POLL)
            n = poll(pollfds, 1, POLL_FOREVER);
            if (n < 0 || is_poll_error(pollfds[0].revents))
                throw Network_io_error("poll", errno);
#elif defined(HAVE_SELECT)
            if (select(sock+1, &fdset, 0, 0, 0) <= 0)
                throw Network_io_error("select", errno);
#endif
            continue;
        }
        else if (n == 0)
            return count <= 0 ? 0 : -1;

        nleft -= n;
        buf   += n;
    }
    return count;
}

int ares::net_tk::write_tcp(Sockfd sock, Byte const* buf, int count)
{
    if (count <= 0)
        return 0;

    int n;

    errno = 0;
    if ((n = send(sock, buf, count, 0)) < 0) {
        if (!is_transient_send_error(errno))
            throw Network_io_error("write", errno);
        return 0;   // ok: non-blocking i/o would have blocked
    }
    else if (n == 0)
        return -1;  // end-of-file encountered
    return n;
}

int ares::net_tk::write_all_tcp(Sockfd sock, Byte const* buf, int count)
{
    if (count <= 0)
        return 0;

    int num_left = count;
    int n;

#if defined(HAVE_POLL)
    struct pollfd pollfds[1];
    pollfds[0].fd = sock;
    pollfds[0].events = POLLOUT;
#elif defined(HAVE_SELECT)
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
#endif

    while (num_left > 0) {
        errno = 0;
        if ((n = send(sock, buf, num_left, 0)) < 0) {
            if (!is_transient_send_error(errno))
                throw Network_io_error("write", errno);
#if defined(HAVE_POLL)
            n = poll(pollfds, 1, POLL_FOREVER);
            if (n < 0 || is_poll_error(pollfds[0].revents))
                throw Network_io_error("poll", errno);
#elif defined(HAVE_SELECT)
            if (select(sock+1, 0, &fdset, 0, 0) <= 0)
                throw Network_io_error("select", errno);
#endif
            continue;   // end-of-file encountered
        }
        else if (n == 0)
            return -1;

        num_left -= n;
        buf += n;
    }
    assert(num_left == 0);
    return count;
}

void ares::net_tk::set_blocking(Sockfd sock, bool on)
{
    // Get the current socket flags.

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1)
        throw Network_error("fcntl", errno);

    // Change only the blocking-IO flag and reset.

    flags = on ? flags & ~O_NONBLOCK : flags | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) != 0)
        throw Network_error("fcntl", errno);
}

void ares::net_tk::set_tcp_no_delay(Sockfd sock, bool on)
{
    int const val = on;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) != 0)
        throw Network_error("setsockopt", errno);
}

string ares::net_tk::my_hostname()
{
    int const max_len = max_hostname_length();
    char* name = new char[max_len];

    if (gethostname(name, max_len) != 0) {
        delete [] name;
        throw Network_error("gethostname", errno);
    }

    name[max_len-1] = '\0';  // gethostname doesn't always null terminate
    string hostname = name;
    delete [] name;
    return hostname;
}

int ares::net_tk::max_hostname_length()
{
    return NI_MAXHOST;
}

bool ares::net_tk::reverse_name_lookup(char const* address,
                                       int family,
                                       string* host)
{
#ifdef HAVE_GETNAMEINFO
    assert(host != 0);
    *host = "";

    // Create the in_addr{} structure, calling inet_pton() to convert the
    // supplied IP address to numeric format.

    struct in_addr addr;
    if (inet_pton(family, address, &addr) == 0)
        return false;

    // Fill in the sockaddr_in{} structure fields.

    struct sockaddr_in si;
    memset(&si, 0, sizeof(si));
    si.sin_family      = family;
    si.sin_port        = htons(0);
    si.sin_addr.s_addr = addr.s_addr;

    // Call getnameinfo() to get the name information.

    vector<char> name(max_hostname_length());
    if (getnameinfo((struct sockaddr*) &si, sizeof(si),
                    &name[0], name.size(), 0, 0, 0) < 0)
    {
        return false;
    }

    *host = &name[0];
    return true;
#else
    return false;   // always fail for platforms without getnameinfo()
#endif
}


// module initialization

namespace { int nifty_counter = 0; }

ares::net_tk::Net_tk_init::Net_tk_init()
{
    if (0 == nifty_counter++) {

        // Trap and ignore SIGPIPE. This signal is raised often when writing
        // to sockets and is difficult to avoid. The default handling of
        // SIGPIPE is unforgiving (process aborts), so we ignore it here.

        signal(SIGPIPE, SIG_IGN);
    }
}

ares::net_tk::Net_tk_init::~Net_tk_init()
{
    if (0 == --nifty_counter) {
        // Cleanup.
    }
}
