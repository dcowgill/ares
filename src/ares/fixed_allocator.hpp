// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_fixed_allocator
#define included_ares_fixed_allocator

#include <cstddef>
#include <new>      // virtually all clients will need this

namespace ares {

// An allocator for fixed-sized blocks. Due to their generality, the standard
// functions malloc and new are less efficient in terms of running time and
// space overhead.
class Fixed_allocator {
  public:
    // Constructs an allocator that will allocator fixed blocks of block_size
    // bytes. The caller may optionally specify the number of blocks the
    // allocator should internally allocate at one time.
    Fixed_allocator(std::size_t block_size, int blocks_per_chunk = 0);

    // Destroys this object and frees all memory allocated by it.
    ~Fixed_allocator();

    // Returns a fixed-size memory block. The size of the memory block was
    // passed to this object's constructor.
    void* allocate();

    // Frees memory allocated by this object's allocate function. Undefined if
    // the given pointer was not allocated by this object.
    void free(void* p);

    // Frees all memory allocated by this object. Invalidates all pointers
    // previously returned by calling this object's allocate function.
    void release();

  private:
    void replenish();               // gets more memory

    class Chunk_list;               // opaque chunk-manager class
    struct Link { Link* m_next; };  // free list node type

    Chunk_list* m_allocator;        // chunk manager object
    Link* m_free_list;              // list of free blocks
    std::size_t const m_block_size; // fixed block size
    int const m_blocks_per_chunk;   // blocks to allocate when out of memory
};

// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

inline void* Fixed_allocator::allocate()
{
    if (!m_free_list)
        replenish();
    void* p = static_cast<void*>(m_free_list);
    m_free_list = m_free_list->m_next;
    return p;
}

inline void Fixed_allocator::free(void* p)
{
    Link* link = static_cast<Link*>(p);
    link->m_next = m_free_list;
    m_free_list = link;
}

} // namespace ares

#endif
