// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_shared_ptr
#define included_ares_shared_ptr

#include "ares/guard.hpp"
#include "ares/platform.hpp"
#include <assert.h>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace ares {

// Derive from this class to add an intrusive reference count to another
// class. Useful when combined with boost::intrusive_ptr.
class Reference_counted {
  public:
    Reference_counted() : m_count(0) {}
    virtual ~Reference_counted();
    void add_ref() { ++m_count; }
    bool release() { return --m_count == 0; }

  private:
    int m_count;
};

// Similar to Reference_counted, but this class's functions are reentrant.
class Thread_safe_reference_counted {
  public:
    Thread_safe_reference_counted() : m_count(0) {}
    virtual ~Thread_safe_reference_counted();
    void add_ref() { Guard g(m_mutex); ++m_count; }
    bool release() { Guard g(m_mutex); return --m_count == 0; }

  private:
    Mutex m_mutex;
    int m_count;
};

// The following functions are designed to work with the boost::intrusive_ptr
// template class. Any class derived from either Reference_counted or
// Thread_safe_reference_counted may be wrapped in a boost::intrusive_ptr and
// these functions will be called to increment and decrement-and-test the
// pointer's reference count.

template<class T> inline void intrusive_ptr_add_ref(T* p)
{
    p->add_ref();
}

template<class T> inline void intrusive_ptr_release(T* p)
{
    if (p->release())
        delete p;
}

} // namespace ares

#endif
