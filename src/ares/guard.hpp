// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_guard
#define included_ares_guard

#include "ares/mutex.hpp"
#include "ares/rwlock.hpp"
#include "ares/utility.hpp"

namespace ares {

template<class MUTEX>
class Guard_tmpl : boost::noncopyable {
  public:
    Guard_tmpl(MUTEX& m)
            : m_mutex(&m)
    {
        m.acquire();
    }

    Guard_tmpl(MUTEX& m, bool wait)
    {
        if (wait) {
            m.acquire();
            m_mutex = &m;
        } else {
            m_mutex = m.acquire_no_wait() ? &m : 0;
        }
    }

    ~Guard_tmpl()
    try {
        if (m_mutex)
            m_mutex->release();
    }
    catch (...) {}

    void disable()
    {
        m_mutex = 0;
    }

    bool acquired() const
    {
        return m_mutex != 0;
    }

  private:
    MUTEX* m_mutex;     // the held lock
};

// Create a convenient typedef for the most common use of Guard_tmpl.
typedef Guard_tmpl<Mutex> Guard;

class Guard_rw : boost::noncopyable {
  public:
    enum Mode {
        SHARED,
        EXCLUSIVE,
    };

    Guard_rw(Rwlock& rwlock, Mode mode);
    ~Guard_rw();
    void promote();
    void demote();

  private:
    // This enum contains the possible values of m_lock_held.
    enum { UNLOCKED = -1, READ = SHARED, WRITE = EXCLUSIVE };

    Rwlock& m_rwlock;       // reference to read-write lock
    int m_lock_held;        // type of lock currently held
};

} // namespace ares

#endif
