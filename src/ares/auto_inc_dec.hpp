// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_auto_inc_dec
#define included_ares_auto_inc_dec

namespace ares {

template<typename T>
class Auto_inc_dec {
  public:
    explicit Auto_inc_dec(T& val) : m_val(val) { ++m_val; }
    ~Auto_inc_dec() { --m_val; }

  private:
    T& m_val;
};

} // namespace ares

#endif
