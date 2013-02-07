// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_sync_queue
#define included_ares_sync_queue

#include "ares/fixed_allocator.hpp"
#include "ares/guard.hpp"
#include "ares/mutex.hpp"
#include <assert.h>
#include <vector>

namespace ares {

// Synchronized queue.
template <typename T, class MUTEX = Mutex>
class Sync_queue {
  public:
    // This typedef preserves the queue item type, allowing template client
    // code to reference it.
    typedef T Item_type;

    // Creates an unbounded, synchronized queue.
    Sync_queue()
            : m_pool(sizeof(Link))
            , m_head(0)
            , m_size(0)
    {}

    // Destructor.
    ~Sync_queue()
    {
        while (m_head != 0) {
            Link* p = m_head;
            m_head = m_head->m_next;
            p->~Link();
        }
    }

    // Enqueues a value in the queue.
    void enqueue(Item_type item)
    {
        Guard_tmpl<MUTEX> guard(m_mutex);
        Link* new_link = new (m_pool.allocate()) Link(item, 0);
        if (m_head == 0)
            m_head = new_link;
        else
            *m_end = new_link;
        m_end = &(new_link->m_next);
        ++m_size;
    }

    // Dequeues a value from the queue and returns it. Aborts if the queue is
    // empty. This method requires that the item type have a copy constructor.
    Item_type dequeue()
    {
        Guard_tmpl<MUTEX> guard(m_mutex);
        assert(m_head != 0);
        Link* p = m_head;
        Item_type item = p->m_item;
        m_head = m_head->m_next;
        p->~Link();
        m_pool.free(p);
        --m_size;
        return item;
    }

    // Dequeues all enqueued values, storing them in the provided vector. If
    // the queue is empty, returns zero. Note that the existing contents of
    // the vector are removed.
    int dequeue_all(std::vector<Item_type>& v)
    {
        Guard_tmpl<MUTEX> guard(m_mutex);
        v.clear();
        while (m_head != 0) {
            Link* p = m_head;
            m_head = m_head->m_next;
            v.push_back(p->m_item);
            p->~Link();
            m_pool.free(p);
        }
        m_size = 0;
        return v.size();
    }

    // Tests if the queue is empty.
    bool is_empty() const
    {
        Guard_tmpl<MUTEX> guard(m_mutex);
        return m_size == 0;
    }

    // Returns the number of items in the queue.
    int size() const
    {
        Guard_tmpl<MUTEX> guard(m_mutex);
        return m_size;
    }

  private:
    struct Link {
        Item_type m_item;
        Link* m_next;

        Link(Item_type item, Link* next)
                : m_item(item), m_next(next) {}
    };

    mutable MUTEX m_mutex;      // make queue updates thread-safe
    Fixed_allocator m_pool;     // pool memory allocator
    Link* m_head;               // first link in list
    Link** m_end;               // end of list (point of insertion)
    int m_size;                 // number of items in queue
};

} // namespace ares

#endif
