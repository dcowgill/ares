// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_utility
#define included_ares_utility

#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>

namespace ares {

// Delete utility function for use with STL algorithms. It is intended for use
// with containers of heap-allocated pointers that need to be deallocated all
// at once (most likely within a destructor). For example:
//
//      std::vector<int*> c;
//      c.push_back(new int);
//      c.push_back(new int);
//      std::for_each(c.begin(), c.end(), delete_fun<int>);
template<typename T>
inline void delete_fun(T* p) throw ()
{
    delete p;
}

// Similar to delete_fun, this function uses delete[] instead.
//
//      std::vector<int*> c;
//      c.push_back(new int[10]);
//      c.push_back(new int[10]);
//      std::for_each(c.begin(), c.end(), array_delete_fun<int>);
template<typename T>
inline void array_delete_fun(T* p) throw ()
{
    delete[] p;
}

// A divide function that returns zero when the divisor is zero. Intended for
// use with integral numeric types.
template<typename T>
inline T safe_divide(T lhs, T rhs) throw ()
{
    return rhs != 0 ? lhs/rhs : 0;
}

template<class T, class R = void>
class Finally_mem_fn_call {
  public:
    typedef R (T::*Mem_fn)();

    Finally_mem_fn_call(T& obj, Mem_fn fn)
            : m_obj(obj)
            , m_fn(fn)
    {}

    ~Finally_mem_fn_call()
    {
        (m_obj.*m_fn)();
    }

  private:
    T& m_obj;
    Mem_fn m_fn;
};

// This macro provides a simple printf-and-abort facility. It's similar to the
// standard assert macro, except this one takes printf-style arguments instead
// of a boolean expression. Note that the macro arguments must be doubly
// parenthesized, e.g. ARES_PANIC(("Hello, %s!", "world"));
#define ARES_PANIC(args)                                        \
    ares::panic(__FILE__, __LINE__, __PRETTY_FUNCTION__) args

// This function is used by ARES_PANIC; never call it directly.
void (*panic(char const*,int,char const*))(char const*, ...);

} // namespace ares

#endif
