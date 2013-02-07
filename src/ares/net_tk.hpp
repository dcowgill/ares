// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_net_tk
#define included_ares_net_tk

#include "ares/types.hpp"
#include <string>

namespace ares { namespace net_tk {

extern int const FAMILY_DOMAIN;
extern int const FAMILY_INET;
extern int const FAMILY_INET6;

Sockfd null_socket();

Sockfd connect_tcp(char const* address,
                   char const* port,
                   std::string* remote_address=0,
                   int* remote_port=0,
                   int timeout_millis=0);

Sockfd listen_tcp(char const* address,
                  char const* port,
                  int* len_ptr,
                  int backlog=50);

Sockfd connect_udp(char const* address, char const* port);
Sockfd listen_udp(char const* address, char const* port, int* len_ptr);

void close_socket(Sockfd sock, bool linger = false);

Sockfd accept(Sockfd listen_sock);
Sockfd accept(Sockfd listen_sock,
              void* addr_ptr,
              int addr_len,
              std::string* remote_address,
              int* remote_port);

int read_tcp(Sockfd sock, Byte* buf, int count);
int read_all_tcp(Sockfd sock, Byte* buf, int count);

// Writes up to count bytes from buf to the specified TCP socket. The valid
// return values are as follows:
//
//  -1: End-of-file encountered; socket is closed.
//   0: Socket is non-blocking and no data were sent.
//  >0: This many bytes were sent successfully.
//
// Note that the number of bytes sent may be less than the desired count. If
// count is less than 1, the function returns 0 without error. Throws an
// Network_io_error exception if an i/o error occurs.
int write_tcp(Sockfd sock, Byte const* buf, int count);

// Works similarly to write_tcp, but this function continues trying to send
// until the requested number of bytes are sent, an i/o error occurs, or
// end-of-file is encountered.
int write_all_tcp(Sockfd sock, Byte const* buf, int count);

void set_blocking(Sockfd sock, bool on);
void set_tcp_no_delay(Sockfd sock, bool on);

std::string my_hostname();
int max_hostname_length();
bool reverse_name_lookup(char const* address, int family, std::string* host);


// module initializer
struct Net_tk_init { Net_tk_init(); ~Net_tk_init(); };
static Net_tk_init s_ares_net_tk_init;

} } // namespace ares::net_tk

#endif
