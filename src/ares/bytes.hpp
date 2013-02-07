// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_bytes
#define included_ares_bytes

#include "ares/types.hpp"
#include <algorithm>
#include <cstring>
#include <string>

namespace ares {

// A dynamic array of bytes. This class is low-level and should only rarely be
// used instead of std::vector<Byte>. Specifically, Bytes offers something
// std::vector does not: a contiguous array with separately adjustable logical
// and physical sizes. For example, the following code is not legal C++:
//
//      std::vector<Byte> v;
//      v.reserve(100);
//      memcpy(&v[0], ptr, 100);
//
// Although the code would be legal if "reserve" were replaced with "resize",
// the vector would no longer maintain a logical size because the user
// hijacked the size to indicate capacity. The Bytes class is narrowly focused
// for providing such capability:
//
//      Bytes b;
//      b.set_min_capacity_no_copy(100); // warning: destroys b's contents!
//      memcpy(b.buf(), ptr, 100);
//      b.set_size(100);                 // caller must specify logical size
//
// This class is primarily useful when the program must store and provide
// access to an array of bytes, but must also pass the array to a low-level
// (typically C) function that expects to write directly to a byte pointer.
class Bytes {
  public:
    // Constructs a byte array, optionally specifying its initial capacity. If
    // the capacity is not provided, a small default is chosen.
    explicit Bytes(int capacity=0);

    // Constructs an array consisting of the first count bytes in data. The
    // capacity of the source array may be greater than or equal to count.
    Bytes(Byte const* data, int count);

    // Constructs an array consisting of the bytes in the given string. This
    // is semantically equivalent to Bytes(s.data(),s.size()).
    explicit Bytes(std::string const& s);

    // Constructs a copy of another byte array. The new array's capacity will
    // be just large enough to contain the valid bytes in b; that is, its
    // capacity will be equal to b.size()(). (Corollary: the new array is not
    // an exact duplicate of the old one.)
    Bytes(Bytes const& b);

    // Destructor.
    ~Bytes();

    // Assigns the contents of b to the array. This array's new capacity will
    // be at least b.size() bytes, but is otherwise not defined.
    Bytes& operator=(Bytes const& b);

    // Stores the first first count bytes in data in this byte array,
    // destroying whatever data was there previously. If the array's current
    // capacity is greater than count, it will not be changed; otherwise, the
    // capacity will be equal to count. This array's _size_ will be exactly
    // equal to count. This function has no effect if count is not positive.
    void assign(Byte const* data, int count);

    // Swaps the internal representation of this array with that of b.
    void swap(Bytes& b);

    // Changes the capacity, or physical size, of the array. If n is negative,
    // the array capacity will be set to zero. Warning: If n is less than the
    // current capacity, the contents of the array will be truncated, and its
    // size will be reduced to n.
    void set_capacity(int n);

    // Insures that this array's capacity, or physical size, is at least n
    // bytes. This function is similar to Bytes::set_capacity except that it
    // does not modify the array if the current capacity is greater than n.
    void set_min_capacity(int n);

    // Works like Bytes::set_min_capacity but destroys the contents of the
    // array. As a result, this function is more efficient than
    // set_min_capacity when the array capacity must be increased.
    void set_min_capacity_no_copy(int n);

    // Changes the size of this array. If n is greater than the current array
    // capacity, the array size will be set to the current capacity. If n is
    // negative, the array size will be set to zero.
    void set_size(int n);

    // Returns a writable pointer to the beginning of the internal byte array.
    // The returned pointer is guaranteed to point to at least capacity()
    // bytes of contiguous storage.
    Byte* buf() { return m_data; }

    // Returns a read-only pointer to the beginning of the internal byte
    // array. The returned pointer is guaranteed to point to at least
    // capacity() bytes of contiguous storage, of which size() bytes are
    // valid.
    Byte const* begin() const { return m_data; }

    // Returns a read-only pointer to one past the last valid element in the
    // array. The pointer returned by this function is always equal to
    // begin()+size().
    Byte const* end() const { return begin() + size(); }

    // Returns the capacity of this array.
    int capacity() const { return m_capacity; }

    // Returns the size of this array.
    int size() const { return m_length; }

    // Converts the contents of the array to a string.
    std::string to_string() const;

  private:
    enum { DEFAULT_CAPACITY=32 };

    int m_capacity;     // physical size
    int m_length;       // logical size
    Byte* m_data;       // byte array
};

// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

inline Bytes::Bytes(int capacity)
        : m_capacity(capacity > 0 ? capacity : DEFAULT_CAPACITY)
        , m_length(0)
        , m_data(new Byte[m_capacity])
{}

inline Bytes::Bytes(Byte const* data, int count)
        : m_capacity(count > 0 ? count : 0)
        , m_length(m_capacity)
        , m_data(new Byte[m_capacity])
{
    std::memcpy(m_data, data, m_length);
}

inline Bytes::Bytes(std::string const& s)
        : m_capacity(s.size())
        , m_length(m_capacity)
        , m_data(new Byte[m_capacity])
{
    std::memcpy(m_data, s.data(), m_length);
}

inline Bytes::Bytes(Bytes const& b)
        : m_capacity(b.m_length)
        , m_length(m_capacity)
        , m_data(new Byte[m_capacity])
{
    std::memcpy(m_data, b.m_data, m_length);
}

inline Bytes::~Bytes()
{
    delete [] m_data;
}

inline Bytes& Bytes::operator=(Bytes const& b)
{
    if (this != &b) {
        if (m_capacity < b.m_length) {
            Byte* p = new Byte[b.m_length]; // might throw
            delete [] m_data;
            m_data = p;
            m_capacity = b.m_length;
        }
        m_length = b.m_length;
        memcpy(m_data, b.m_data, m_length);
    }
    return *this;
}

inline void Bytes::assign(Byte const* data, int count)
{
    if (count >= 0) {
        set_min_capacity_no_copy(count);
        set_size(count);
        std::memcpy(buf(), data, count);
    }
}

inline void Bytes::swap(Bytes& b)
{
    std::swap(m_capacity, b.m_capacity);
    std::swap(m_length, b.m_length);
    std::swap(m_data, b.m_data);
}

inline void Bytes::set_capacity(int n)
{
    if (n < 0) n = 0;
    Byte* p = new Byte[n];        // might throw
    m_capacity = n;
    if (m_length > m_capacity)    // reduce size if necessary
        m_length = m_capacity;
    std::memcpy(p, m_data, m_length);
    delete [] m_data;
    m_data = p;
}

inline void Bytes::set_min_capacity(int n)
{
    if (n > m_capacity) {         // are we big enough already?
        Byte* p = new Byte[n];    // might throw
        m_capacity = n;
        std::memcpy(p, m_data, m_length);
        delete [] m_data;
        m_data = p;
    }
}

inline void Bytes::set_min_capacity_no_copy(int n)
{
    if (n > m_capacity) {         // are we big enough already?
        Byte* p = new Byte[n];    // might throw
        m_capacity = n;
        delete [] m_data;
        m_data = p;
    }
}

inline void Bytes::set_size(int n)
{
    if (n <= 0)
        m_length = 0;
    else if (n <= m_capacity)
        m_length = n;
    else
        m_length = m_capacity;
}

inline std::string Bytes::to_string() const
{
    char const* p = (char const*) begin();
    return std::string(p, size());
}

} // namespace ares

#endif
