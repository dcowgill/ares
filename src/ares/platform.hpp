// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_platform
#define included_ares_platform

#include "ares/config.h"
#include "ares/error.hpp"
#include "ares/types.hpp"
#include "ares/utility.hpp"
#include <cerrno>
#include <ctime>
#include <string>
#include <utility>

#if defined(HAVE_LIBPTHREAD) && defined(HAVE_PTHREAD_H)
#include <pthread.h>
#else
#error "pthread library is required"
#endif

namespace ares {

// +---------------------------------+
// | Process information and control |
// +---------------------------------+

// Returns a string representation of the id of the current process.
std::string system_process_id();

// Returns a string representation of the id of the current process's parent.
std::string system_parent_process_id();

// Returns the name of the operating system user executing this process.
std::string system_username();

// +--------------------------+
// | Time and sleep functions |
// +--------------------------+

// Returns the current operating system time as a unix timestamp, i.e. seconds
// since 1970-01-01 00:00:00 UTC.
std::time_t current_time();

// Returns the current operating system time as a unix timestamp in
// milliseconds, i.e. milliseconds since 1970-01-01 00:00:00 UTC.
Int64 current_time_millis();

// Puts the current thread to sleep for the specified number of milliseconds.
void milli_sleep(int millis);

// Puts the current thread to sleep for the specified number of microseconds.
void micro_sleep(int micros);

// +---------------------------+
// | System resource functions |
// +---------------------------+

// Resource usage
enum Resource_type {
    CPU,
    CORE_SIZE,
    FILE_SIZE,
    DATA_SIZE,
    STACK_SIZE,
    OPEN_FILES
};

// Sets the soft and hard limits (in that order) for resource type res.
void set_resource_limit(Resource_type res, std::pair<int,int> limit);

// Gets the soft and hard limits (in that order) for resource type res.
std::pair<int,int> get_resource_limit(Resource_type res);


// module initialiation
struct Platform_init { Platform_init(); ~Platform_init(); };
static Platform_init s_ares_platform_init;

} // namespace ares

#endif
