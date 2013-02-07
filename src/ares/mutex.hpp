// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_mutex
#define included_ares_mutex

#include "ares/platform.hpp"
#include "ares/utility.hpp"

namespace ares {

// A mutual-exclusion lock. Uses pthread semantics.
class Mutex : boost::noncopyable {
  public:
    Mutex();
    ~Mutex();

    // Acquires this lock for exclusive access.
    void acquire();

    // Similar to acquire, except this function will not block if the lock is
    // currently held by another thread. Returns true if the lock was
    // acquired, false otherwise.
    bool acquire_no_wait();

    // Releases this lock if the current thread has acquired it. Will throw an
    // exception if this thread doesn't hold the lock.
    void release();

    // Returns an opaque reference to the underlying platform-specific mutex
    // handle. This function is necessary when another platform-specific
    // function requires a mutex handle.
    void* handle();

  private:
    pthread_mutex_t m_mutex;
};

} // namespace ares

#endif
