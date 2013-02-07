// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/platform.hpp"
#include "ares/string_tokenizer.hpp"
#include "ares/string_util.hpp"

// Standard headers
#include <cassert>
#include <cstdlib>          // realpath
#include <cstdio>           // snprintf
#include <cstring>          // memset memcpy strncpy
#include <memory>           // auto_ptr
#include <vector>

// UNIX headers
#include <pwd.h>            // for getpwuid(3) and struct passwd
#include <signal.h>         // ANSI C signals
#include <sys/resource.h>   // for getrlimit(2) and setrlimit(2)
#include <sys/stat.h>       // for S_xxx file mode constants and umask(2)
#include <sys/time.h>       // timeval{} for gettimeofday()
#include <sys/types.h>      // basic system data types
#include <unistd.h>

using namespace std;

string ares::system_process_id()
{
    return boost::lexical_cast<string>(static_cast<int>(getpid()));
}

string ares::system_parent_process_id()
{
    return boost::lexical_cast<string>(static_cast<int>(getppid()));
}

string ares::system_username()
{
    struct passwd* p = getpwuid(getuid());
    return p ? p->pw_name : "";
}

time_t ares::current_time()
{
    return time(0);
}

ares::Int64 ares::current_time_millis()
{
    struct timeval tv;
    return gettimeofday(&tv, 0) >= 0
            ? Int64(tv.tv_sec)*1000 + Int64(tv.tv_usec)/1000
            : -1;  // error
}

void ares::milli_sleep(int millis)
{
#if defined(HAVE_USLEEP)
    usleep(millis * 1000);
#elif defined(HAVE_POLL)
    poll(0, 0, millis);
#elif defined(HAVE_SELECT)
    struct timeval tv;
    tv.tv_sec = millis / 1000;
    tv.tv_usec = (millis - (tv.tv_sec * 1000)) * 1000;
    select(0, 0, 0, 0, &tv);
#endif
}

void ares::micro_sleep(int micros)
{
#if defined(HAVE_USLEEP)
    usleep(micros);
#elif defined(HAVE_POLL)
    poll(0, 0, micros / 1000);
#elif defined(HAVE_SELECT)
    struct timeval tv;
    tv.tv_sec = micros / 1000000;
    tv.tv_usec = micros - (tv.tv_sec * 1000000);
    select(0, 0, 0, 0, &tv);
#endif
}

static int const s_resources[] = {  // This table translates our resource
    RLIMIT_CPU,                     // constants (ResourceType) to the values
    RLIMIT_CORE,                    // in <sys/resource.h>
    RLIMIT_FSIZE,
    RLIMIT_DATA,
    RLIMIT_STACK,
    RLIMIT_NOFILE
};

void ares::set_resource_limit(Resource_type res, pair<int,int> limit)
{
    struct rlimit rlim;
    rlim.rlim_cur = limit.first >= 0 ? limit.first : RLIM_INFINITY;
    rlim.rlim_max = limit.second >= 0 ? limit.second : RLIM_INFINITY;
    if (setrlimit(s_resources[res], &rlim) != 0)
        throw System_error("setrlimit", errno);
}

pair<int,int> ares::get_resource_limit(Resource_type res)
{
    struct rlimit rlim;
    if (getrlimit(s_resources[res], &rlim) < 0)
        throw System_error("getrlimit", errno);
    return make_pair(rlim.rlim_cur, rlim.rlim_max);
}


// module initialization

namespace { int nifty_counter = 0; }

ares::Platform_init::Platform_init()
{
    if (0 == nifty_counter++) {
        // Platform init
    }
}

ares::Platform_init::~Platform_init()
{
    if (0 == --nifty_counter) {
        // Clean up
    }
}
