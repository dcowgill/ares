// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_rwlock
#define included_ares_rwlock

#include "ares/platform.hpp"
#include "ares/utility.hpp"

namespace ares {

// A reader-writer lock. Uses pthread semantics.
class Rwlock : boost::noncopyable {
  public:
    Rwlock();
    ~Rwlock();

    // Acquires this lock for reading. If there are no writers waiting, this
    // function will not block even if other readers hold the lock.
    void acquire_read();

    // Acquires this lock for writing. Will block other readers and writers.
    // Readers will not be able to acquire the lock while a writer is waiting.
    void acquire_write();

    // Releases this lock if the current thread has acquired it for either
    // reading or writing. Will throw an exception if this thread doesn't hold
    // the lock.
    void release();

  private:
    pthread_rwlock_t m_rwlock;  // opaque lock type
};

} // namespace ares

#endif
