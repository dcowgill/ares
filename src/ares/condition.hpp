// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_condition
#define included_ares_condition

#include "ares/platform.hpp"
#include "ares/utility.hpp"

namespace ares {

class Mutex;

class Condition : boost::noncopyable {
  public:
    Condition(Mutex& m);
    ~Condition();
    bool wait(int millis = 0);
    void signal();
    void broadcast();

  private:
    Mutex& m_mutex;         // mutex needed for synchronization
    pthread_cond_t m_cond;  // opaque pointer to sys condition var
};

} // namespace ares

#endif
