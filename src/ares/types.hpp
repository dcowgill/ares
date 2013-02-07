// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_types
#define included_ares_types

#include <sys/types.h>

namespace ares {

typedef int8_t Int8;
typedef u_int8_t Uint8;
typedef int16_t Int16;
typedef u_int16_t Uint16;
typedef int32_t Int32;
typedef u_int32_t Uint32;
typedef int64_t Int64;
typedef u_int64_t Uint64;

typedef Uint8 Octet;
typedef Octet Byte;

typedef pthread_t Thread_id;    // unique ID of a thread
typedef int Sockfd;             // socket handle

} // namespace ares

#endif
