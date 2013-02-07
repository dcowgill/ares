// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/buffer.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>

using namespace std;

enum { GROW_FACTOR = 2, DEFAULT_CAPACITY = 32 };

ares::Buffer::Buffer(int capacity)
        : m_array(capacity > 0 ? capacity : DEFAULT_CAPACITY)
        , m_write(0)
        , m_read(0)
        , m_capacity(m_array.size())
{
    check_invariants();
}

ares::Buffer::Buffer(Byte const* buf, int count)
        : m_array(buf, buf+count)
        , m_write(count)
        , m_read(0)
        , m_capacity(count > 0 ? count : DEFAULT_CAPACITY)
{
    if (int(m_array.size()) < capacity()) {
        m_array.resize(capacity());
    }
    check_invariants();
}

ares::Buffer::Buffer(string const& s)
        : m_array(s.data(), s.data() + s.length())
        , m_write(s.length())
        , m_read(0)
        , m_capacity(s.length() > 0 ? s.length() : DEFAULT_CAPACITY)
{
    if (int(m_array.size()) < capacity()) {
        m_array.resize(capacity());
    }
    check_invariants();
}

ares::Buffer::Buffer(Buffer const& b)
        : m_array(b.begin(), b.end())
        , m_write(b.m_write - b.m_read)
        , m_read(0)
        , m_capacity(b.capacity())
{
    if (int(m_array.size()) < capacity()) {
        m_array.resize(capacity());
    }
    check_invariants();
}

ares::Buffer& ares::Buffer::operator=(Buffer const& b)
{
    Buffer temp(b);
    swap(temp);
    return *this;
}

void ares::Buffer::swap(Buffer& b)
{
    m_array.swap(b.m_array);
    ::swap(m_write, b.m_write);
    ::swap(m_read, b.m_read);
    ::swap(m_capacity, b.m_capacity);
}

void ares::Buffer::assign(Byte const* buf, int count)
{
    set_min_capacity(count);
    clear();
    put(buf, count);
}

bool ares::Buffer::put(Buffer const& b)
{
    if (b.size() > free()) {
        return false;
    }
    memcpy(end(), b.begin(), b.size());
    advance(b.size());
    return true;
}

bool ares::Buffer::put(Byte const* buf, int count)
{
    assert(buf != 0 && count >= 0);

    if (count > free()) {
        return false;
    }
    memcpy(end(), buf, count);
    advance(count);
    return true;
}

bool ares::Buffer::get(Byte* buf, int count)
{
    assert(buf != 0 && count >= 0);

    if (count > size()) {
        return false;
    }
    memcpy(buf, begin(), count);
    consume(count);
    return true;
}

void ares::Buffer::reserve()
{
    static double const MAX_WASTE = 0.5;    // threshold to shift array

    int min_size = capacity() + m_read;
    int cur_size = m_array.size();

    // If the current capacity is sufficient, return.

    if (cur_size >= min_size) {
        return;
    }

    // Compute the proportion of the underlying vector that would lie before
    // the read position if we were to resize it to the minimum size. If the
    // proportion is below some threshold, resize.

    if (m_read <= int(min_size * MAX_WASTE)) {
        do { cur_size *= GROW_FACTOR; } while (cur_size < min_size);
        m_array.resize(cur_size);
        return;
    }

    // We have determined that resetting the read position to zero and copying
    // the array contents is the best approach. It may also be necessary to
    // resize the underlying vector.

    min_size -= m_read;
    if (cur_size < min_size) {
        do { cur_size *= GROW_FACTOR; } while (cur_size < min_size);
        m_array.resize(cur_size);
    }

    // Shift the array elements m_read places down.

    memmove(&m_array[0], begin(), size());
    m_write -= m_read;
    m_read = 0;
}

void ares::Buffer::check_invariants()
{
    assert(capacity() >= 1);
    assert(int(m_array.size()) >= capacity() + m_read);
    assert(int(m_array.size()) >= free() + m_write);
    assert(size() >= 0);
    assert(free() >= 0);
    assert(size() + free() == capacity());
}
