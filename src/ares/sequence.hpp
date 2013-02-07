// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_sequence
#define included_ares_sequence

#include "ares/null_mutex.hpp"
#include "ares/utility.hpp"

namespace ares {

template<class LOCK = Null_mutex, typename INT = int>
class Sequence : boost::noncopyable {
  public:
    Sequence(INT init_val = 1, INT step_size = 1)
            : m_curr_val(init_val)
            , m_step_size(step_size)
    {}

    INT next_val()
    {
        m_lock.acquire();
        INT value = m_curr_val;
        m_curr_val += m_step_size;
        m_lock.release();
        return value;
    }

  private:
    INT m_curr_val;     // current value of the sequence
    INT m_step_size;    // sequence increment value
    LOCK m_lock;        // mutual exclusion lock
};

} // namespace ares

#endif
