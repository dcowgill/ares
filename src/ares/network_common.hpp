// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_network_common
#define included_ares_network_common

// ares headers
#include "ares/config.h"

// Standard headers
#include <cassert>
#include <cerrno>
#include <cstdio>           // snprintf
#include <cstring>          // memset strncpy
#include <map>
#include <vector>

// UNIX headers
#include <arpa/inet.h>      // inet(3) functions
#include <fcntl.h>          // for nonblocking I/O
#include <netdb.h>          // for various defns
#include <netinet/in.h>     // sockaddr_in{} and other Internet defns
#include <netinet/tcp.h>    // more Internet protocol defines
#include <signal.h>         // signal
#include <sys/ioctl.h>      // needed for socket ioctl's
#include <sys/socket.h>     // basic socket definitions
#include <sys/types.h>      // basic system data types
#include <sys/un.h>         // for Unix domain sockets
#include <sys/utsname.h>    // uname(2)
#include <unistd.h>

#if defined(HAVE_POLL)
#include <poll.h>           // poll(2)
#include <sys/poll.h>
#endif

// NI_MAXHOST is not defined on many systems.

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef AF_INET6
#define AF_INET6 AF_MAX+1
#endif

// Max size of an IPv4 address string is:
//
//    "ddd.ddd.ddd.ddd\0"
//     1234567890123456

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

// Define the following even if IPv6 not supported, so we can always allocate
// an adequately sized buffer, without #ifdefs in the code.
//
// Max size of an IPv6 address string is:
//
//    "xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:ddd.ddd.ddd.ddd\0"
//     1234567890123456789012345678901234567890123456

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

namespace
{
#if defined(HAVE_POLL)
// Returns true if the poll event set contains an error event.
bool is_poll_error(int e)
{
    enum { ERROR_BITS = POLLERR | POLLHUP | POLLNVAL };
    return e & ERROR_BITS;
}

// poll blocks when its timeout value is -1.
int const POLL_FOREVER = -1;

// A poll non-event.
int const NO_EVENT = 0;
#endif

// The maximum size to which the queue of incoming connections may grow
// for a listen socket. The effect of this parameter is not well-defined
// across platforms.
int const MAX_PENDING  = 100;
}

#endif
