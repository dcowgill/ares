// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_test_ares_test_sink
#define included_test_ares_test_sink

#include "ares/buffer.hpp"
#include "ares/sink.hpp"

class Test_sink : public ares::Sink {
public:
    virtual ~Test_sink();

    void send(const ares::Buffer& b)
    {
        if (m_buffer.free() < b.size())
            m_buffer.set_capacity(m_buffer.size() + b.size());
        bool success = m_buffer.put(b);
        assert(success);
    }

    void reset() { m_buffer.clear(); }

    ares::Buffer& buffer() { return m_buffer; }

private:
    ares::Buffer m_buffer;
};

#endif
