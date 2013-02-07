// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_test_ares_queue_sink
#define included_test_ares_queue_sink

#include "ares/buffer.hpp"
#include "ares/sink.hpp"
#include <list>

class Queue_sink : public ares::Sink {
public:
    virtual ~Queue_sink();

    void send(const ares::Buffer& b)
    {
        m_queue.push_back(b);
    }

    void reset() { m_queue.clear(); }

    ares::Buffer dequeue()
    {
        assert(!m_queue.empty());
        ares::Buffer b = m_queue.front();
        m_queue.pop_front();
        return b;
    }

    int size() const
    {
        return m_queue.size();
    }

private:
    std::list<ares::Buffer> m_queue;
};

#endif
