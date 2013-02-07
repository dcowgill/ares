// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_trace
#define included_ares_trace

#include "ares/config.h"
#include <string>

namespace ares {

// Simple trace facility

struct Trace {
    static bool is_enabled();
    static void set_dump_dest(std::string const& path);
    static void set_trace_modules(std::string const& trace_str);
    static void set_thread_name(char const* name);
    static bool should_trace(char const* current_file);
    static void no_trace(char const* format, ...) {}

#ifdef ARES_TRACE_ENABLED
#define ARES_TRACE(a)                                   \
    do {                                                \
        if (ares::Trace::should_trace(__FILE__))        \
            ares::Trace::trace(__FILE__,__LINE__) a;    \
    } while (0)                                         \

    typedef void (*Trace_fn)(char const* format, ...);
    static Trace_fn trace(char const* file, int line);
#else
#define ARES_TRACE(a)
#endif
};

} // namespace ares

#endif
