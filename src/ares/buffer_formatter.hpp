// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_buffer_formatter
#define included_ares_buffer_formatter

#include "ares/error.hpp"
#include "ares/utility.hpp"

namespace ares {

class Buffer;

// The base class for buffer formatting objects, providing the basic functions
// required by all buffer formatters. A buffer formatter is an object that
// wraps a Buffer object and reads from or writes to it. Buffers and
// formatters are the primary classes through which input and output are
// accomplished in systems built using this framework.
//
// Note: Unless otherwise noted, formatter objects do not expand their held
// buffer objects when writing to them. Therefore, any write operation may
// raise an overflow error if the buffer capacity is insufficient.
//
// Note: Once a buffer formatter operation fails, subsequent operations will
// also fail until the #reset function is called.
class Buffer_formatter : boost::noncopyable {
  public:
    // Constructs a buffer formatter given a buffer to operate on. The buffer
    // assigned to this formatter object may not be changed.
    Buffer_formatter(Buffer& b)
            : m_buffer(b)
            , m_throw(false)
            , m_fail(false)
    {}

    // Destructor.
    virtual ~Buffer_formatter();

    // Resets the error state of this formatter to the original, non-error
    // state.
    void reset() { m_fail = false; }

    // Specifies whether errors, i.e. underflow when reading or overflow when
    // writing, should throw Buffer_formatter_error exceptions. By default,
    // errors do not cause exceptions to be thrown.
    void raise_errors(bool b) { m_throw = b; }

    // Tests whether this formatter is in the error state. This operator
    // allows the formatter to be tested as a boolean expression.
    operator void*() const { return m_fail ? (void*) 0 : (void*) -1; }

    // Tests whether this formatter is not in the error state. The natural
    // counterpart to operator void* (see above).
    bool operator!() const { return m_fail; }

  protected:
    // Returns the buffer passed to this object's constructor.
    Buffer& buffer() { return m_buffer; }

    // Indicates that a read or write operation on a buffer failed. Derived
    // classes should call this function whenever an error occurs.
    void fail();

  private:
    Buffer& m_buffer;   // the wrapped buffer object
    bool m_throw;       // if true, errors will result in exceptions
    bool m_fail;        // if true, this formatter is in the failure state
};

} // namespace ares

#endif
