// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_shared_queue
#define included_ares_shared_queue

#include "ares/auto_inc_dec.hpp"
#include "ares/condition.hpp"
#include "ares/error.hpp"
#include "ares/null_mutex.hpp"
#include "ares/sync_queue.hpp"
#include <list>

// TODO add enqueue_all to optimize 1:N producer:consumer case? (prob. no)

// FIXME The line "m_empty_cond.wait(max_wait_millis)" is broken wherever it
// appears, it means we will wait more than the specified amount of time if we
// are woken up spuriously.

namespace ares {

// Synchronized queue.
template <typename T>
class Shared_queue {
  public:
    // This typedef preserves the queue item type, allowing template client
    // code to reference it.
    typedef T Item_type;

    // Creates a queue that can contain up to "capacity" items. If capacity is
    // zero or less, the queue size is unbounded.
    explicit Shared_queue(int capacity = 0)
            : m_capacity(capacity > 0 ? capacity : 0)
            , m_empty_cond(m_mutex)
            , m_full_cond(m_mutex)
            , m_num_enqueue_waiters(0)
            , m_num_dequeue_waiters(0)
    {}

    // Enqueues a value in the queue. If the queue is full, waits up to
    // max_wait_millis milliseconds for space to become available. Does not
    // wait at all if max_wait_millis is zero or less. Returns true if the
    // item was enqueued, false if the function timed out.
    bool enqueue(Item_type item, int max_wait_millis = 0)
    {
        Guard guard(m_mutex);

        if (is_full_i()) {
            if (max_wait_millis <= 0)
                return false;

            while (is_full_i()) {   // loop to avoid race condition
                Auto_inc_dec<int> inc_dec(m_num_enqueue_waiters);
                if (!m_full_cond.wait(max_wait_millis))
                    return false;
            }
        }
        assert(!is_full_i());   // queue is not full, and we have the lock

        m_queue.enqueue(item);

        // If necessary, signal waiting consumers or producers.
        if (m_num_dequeue_waiters > 0)
            m_empty_cond.signal();
        else if (m_num_enqueue_waiters > 0)
            m_full_cond.signal();

        return true;
    }

    // Dequeues a value from the queue. If the queue is empty, waits up to
    // max_wait_millis milliseconds for a value to become available. Does not
    // wait at all if max_wait_millis is zero or less. This method requires
    // that the item type have a copy constructor. Throws a Timeout_error if
    // the function times out.
    Item_type dequeue(int max_wait_millis = 0)
    {
        Guard guard(m_mutex);

        if (m_queue.is_empty()) {
            if (max_wait_millis <= 0)
                throw Timeout_error();

            while (m_queue.is_empty()) {      // loop to avoid race condition
                Auto_inc_dec<int> inc_dec(m_num_dequeue_waiters);
                if (!m_empty_cond.wait(max_wait_millis))
                    throw Timeout_error();
            }
        }
        // The queue is not empty, and we have the lock.
        assert(!m_queue.is_empty());

        Item_type item(m_queue.dequeue());

        // If necessary, signal waiting producers or consumers.
        if (m_num_enqueue_waiters > 0)
            m_full_cond.signal();
        else if (m_num_dequeue_waiters > 0)
            m_empty_cond.signal();

        return item;
    }

    // Works like Shared_queue::dequeue(int), except this method returns the
    // item via a reference argument and does not throw an exception on
    // timeout. This method requires that the item type be assignable. Returns
    // true if an item was dequeued, false if the function timed out.
    bool dequeue(Item_type& item, int max_wait_millis = 0)
    {
        Guard guard(m_mutex);

        if (m_queue.is_empty()) {
            if (max_wait_millis <= 0)
                return false;

            while (m_queue.is_empty()) {      // loop to avoid race condition
                Auto_inc_dec<int> inc_dec(m_num_dequeue_waiters);
                if (!m_empty_cond.wait(max_wait_millis))
                    return false;
            }
        }
        // The queue is not empty, and we have the lock.
        assert(!m_queue.is_empty());

        item = m_queue.dequeue();

        // If necessary, signal waiting producers or consumers.
        if (m_num_enqueue_waiters > 0)
            m_full_cond.signal();
        else if (m_num_dequeue_waiters > 0)
            m_empty_cond.signal();

        return true;
    }

    // Dequeues all enqueued values, storing them in v. If the queue is empty,
    // waits up to max_wait_millis milliseconds for a value to become
    // available. Does not wait at all if max_wait_millis is zero or less.
    // Returns the number of dequeued items, or zero on timeout.
    int dequeue_all(std::vector<Item_type>& v, int max_wait_millis = 0)
    {
        Guard guard(m_mutex);

        if (m_queue.is_empty()) {
            if (max_wait_millis <= 0)
                return 0;

            while (m_queue.is_empty()) {      // loop to avoid race condition
                Auto_inc_dec<int> inc_dec(m_num_dequeue_waiters);
                if (!m_empty_cond.wait(max_wait_millis))
                    return 0;
            }
        }
        // The queue is not empty, and we have the lock.
        assert(!m_queue.is_empty());

        m_queue.dequeue_all(v);

        // Signal waiting producers if necessary.
        if (m_num_enqueue_waiters > 0)
            m_full_cond.signal();

        return v.size();
    }

    // Changes the queue capacity. If n is zero or less, the queue will not be
    // bounded. Note that if the new capacity is less than the number of items
    // currently in the queue, no items will be removed and the queue will be
    // over capacity until sufficiently many items are dequeued.
    void set_capacity(int n)
    {
        Guard guard(m_mutex);
        m_capacity = n > 0 ? n : 0;
    }

    // Tests if the queue is empty.
    bool is_empty() const
    {
        Guard guard(m_mutex);
        return m_queue.is_empty();
    }

    // Tests if the queue is full.
    bool is_full() const
    {
        Guard guard(m_mutex);
        return is_full_i();
    }

    // Returns the number of items in the queue.
    int size() const
    {
        Guard guard(m_mutex);
        return m_queue.size();
    }

    // Returns the maximum number of items that may be stored in the queue.
    int capacity() const
    {
        Guard guard(m_mutex);
        return m_capacity;
    }

  private:
    bool is_full_i() const
    {
        return m_capacity > 0 && m_queue.size() >= m_capacity;
    }

  private:
    typedef Sync_queue<Item_type, Null_mutex> Queue_type;

    Queue_type m_queue;             // the queue implementation
    int m_capacity;                 // maximum size (<=0 is unlimited)
    mutable Mutex m_mutex;          // make queue updates thread-safe
    Condition m_empty_cond;         // to wait on empty status
    Condition m_full_cond;          // to wait on full status
    int m_num_enqueue_waiters;      // # of blocked enqueue attempts
    int m_num_dequeue_waiters;      // # of blocked dequeue attempts
};

} // namespace ares

#endif
