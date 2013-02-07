// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_null_mutex
#define included_ares_null_mutex

namespace ares {

struct Null_mutex {
    void acquire() {}
    void release() {}
};

} // namespace ares

#endif
