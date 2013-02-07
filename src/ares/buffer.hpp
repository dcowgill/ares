// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_buffer
#define included_ares_buffer

#include "ares/error.hpp"
#include "ares/shared_ptr.hpp"
#include "ares/types.hpp"
#include <vector>

namespace ares {

// A sliding-window byte buffer. This class is the basis for virtually all i/o
// performed by the system; that is, input is typically stored directly in a
// Buffer object, and output is typically written from a Buffer object. Data
// are stored in and extracted from buffers using classes derived from
// Buffer_formatter.
class Buffer : public Reference_counted {
  public:
    // Constructs a buffer, optionally specifying its initial capacity. If the
    // capacity is left unspecified, a small default value is used. The
    // capacity can never be zero; the minimum capacity is 1.
    explicit Buffer(int capacity = 0);

    // Constructs a buffer containing the first count bytes in buf. The buffer
    // capacity will be exactly count unless it is zero, in which case a small
    // default value is used.
    Buffer(Byte const* buf, int count);

    // Constructs a buffer containing the bytes in the given string. The
    // buffer capacity will be exactly the length of s unless it is empty, in
    // which case a small default value is used.
    explicit Buffer(std::string const& s);

    // Constructs a copy of a given buffer.
    Buffer(Buffer const& b);

    // Assigns the contents of a buffer to this one.
    Buffer& operator=(Buffer const& b);

    // Swaps the contents of a buffer with the contents of this one.
    void swap(Buffer& b);

    // Clears all data from the buffer.
    void clear() { m_write = m_read = 0; }

    // Assigns the first count bytes in buf to this buffer. The existing
    // contents of the buffer are destroyed, and the buffer will be expanded
    // if necessary to accomodate the data.
    void assign(Byte const* buf, int count);

    // Writes the contents of b to the end of this buffer. If the buffer's
    // capacity is insufficient to append the bytes, returns false without
    // modifying the buffer.
    bool put(Buffer const& b);

    // Writes the first count bytes from buf to the end of this buffer. If the
    // buffer's capacity is insufficient to append the bytes, returns false
    // without modifying the buffer. Undefined if buf is null or count is less
    // than zero.
    bool put(Byte const* buf, int count);

    // Moves count bytes from the beginning of this buffer into buf. If there
    // are fewer than count bytes available, returns false without modifying
    // the buffer. Undefined if buf is null or count is less than zero.
    bool get(Byte* buf, int count);

    // Discards the next n bytes in the buffer. This function is generally
    // used after reading and processing n bytes from the pointer returned by
    // the member function begin. Undefined if n is negative. Throws a
    // Range_error exception if fewer than n bytes are in the buffer.
    void consume(int n);

    // Advances the buffer's write position n bytes, as if that many bytes had
    // just been written to the buffer. This function is generally used in
    // conjuction with an external write to the pointer returned by the member
    // function end. Undefined if n is negative. Throws a Range_error
    // exception if the buffer capacity is insufficient.
    void advance(int n);

    // The opposite of advance, this function moves the buffer's write
    // position n bytes backwards. This allows the caller to write to a
    // previous position in the buffer. Undefined if n is negative. Throws a
    // Range_error exception if fewer than n bytes are in the buffer.
    void reverse(int n);

    // This function sets the buffer write position to a previously marked
    // value. Undefined if mark was not returned by a prior call to mark or if
    // an invalidating member function has been called since (see mark for a
    // list of those invalidating functions).
    void reset(int mark);

    // Changes the capacity of the buffer. If this would cause data to be
    // lost, the most recently written bytes are discarded first. The buffer
    // capacity can never be less than 1; if n is less than 1, the new
    // capacity will be 1.
    void set_capacity(int n);

    // Insures that the capacity of this buffer is at least n bytes.
    void set_min_capacity(int n);

    // Returns a pointer to the beginning of readable bytes in the buffer.
    // This pointer is guaranteed to point to a contiguous sequence of bytes
    // whose length is returned by the size member function.
    Byte* begin();

    // Returns a pointer to the beginning of writable bytes in the buffer.
    // This pointer is guaranteed to point to a contiguous sequence of bytes
    // whose length is returned by the free member function.
    Byte* end();

    // Returns a read-only pointer to the beginning of readable bytes in the
    // buffer. This pointer is guaranteed to point to a contiguous sequence of
    // bytes whose length is returned by the size member function.
    Byte const* begin() const;

    // Returns a read-only pointer to one past the last readable byte in the
    // buffer.
    Byte const* end() const;

    // Returns a value that can be passed to reset to change the buffer's
    // write position to whatever it is now. This allows clients to write to
    // arbitrary points in the buffer. Warning: The value returned by this
    // function is invalidated by calls to clear, swap, get, consume, and
    // set_capacity. An invalid mark should not be passed to the reset
    // function.
    int mark() const;

    // Returns the number of valid bytes stored in the buffer.
    int size() const;

    // Returns the amount of free space in the buffer.
    int free() const;

    // Returns the total capacity of the buffer.
    int capacity() const;

  private:
    void reserve();
    void check_invariants();

  private:
    std::vector<Byte> m_array;  // byte array
    int m_write;                // write position
    int m_read;                 // read position
    int m_capacity;             // logical capacity
};

// Buffers are reference-counted objects, and may therefore be wrapped in a
// boost::intrusive_ptr to enable automatic garbage collection.
typedef boost::intrusive_ptr<Buffer> Shared_buffer;

// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

inline void Buffer::consume(int n)
{
    if (n < 0 || n > size())
        throw Range_error();
    m_read += n;
    reserve();
    check_invariants();
}

inline void Buffer::advance(int n)
{
    if (n < 0 || n > free())
        throw Range_error();
    m_write += n;
    check_invariants();
}

inline void Buffer::reverse(int n)
{
    if (n < 0 || n > size())
        throw Range_error();
    m_write -= n;
    check_invariants();
}

inline void Buffer::reset(int mark)
{
    m_write = mark;
    check_invariants();
}

inline void Buffer::set_capacity(int n)
{
    m_capacity = n;
    reserve();
    check_invariants();
}

inline void Buffer::set_min_capacity(int n)
{
    if (capacity() < n)
        set_capacity(n);
}

inline Byte* Buffer::begin()
{
    return &m_array[m_read];
}

inline Byte* Buffer::end()
{
    return &m_array[m_write];
}

inline Byte const* Buffer::begin() const
{
    return &m_array[m_read];
}

inline Byte const* Buffer::end() const
{
    return &m_array[m_write];
}

inline int Buffer::mark() const
{
    return m_write;
}

inline int Buffer::size() const
{
    return m_write-m_read;
}

inline int Buffer::free() const
{
    return capacity() - size();
}

inline int Buffer::capacity() const
{
    return m_capacity;
}

} // namespace ares

#endif
