// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/fixed_allocator.hpp"
#include "ares/utility.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;
using ares::Fixed_allocator;

namespace
{
size_t align_block_size(size_t block_size, size_t link_size)
{
    if (block_size < link_size)
        block_size = link_size;

    // Choose a power of two greater than or equal to the block size.
    size_t aligned_block_size = 2;
    while (aligned_block_size < block_size)
        aligned_block_size *= 2;
    return aligned_block_size;
}
}

// This class manages the list of memory chunks created by Fixed_allocator. It
// allows the caller to allocate variable-size chunks one at a time, then
// release them all at once.
class Fixed_allocator::Chunk_list {
  public:
    char* allocate(size_t chunk_size);
    void release();

  private:
    vector<char*> m_chunks;     // array of managed memory chunks
};

char* Fixed_allocator::Chunk_list::allocate(size_t chunk_size)
{
    char* chunk = new char[chunk_size];
    m_chunks.push_back(chunk);
    return chunk;
}

void Fixed_allocator::Chunk_list::release()
{
    for_each(m_chunks.begin(), m_chunks.end(), array_delete_fun<char>);
    m_chunks.resize(0);
    vector<char*>().swap(m_chunks); // vector-resize trick
}


Fixed_allocator::Fixed_allocator(size_t block_size, int blocks_per_chunk)
        : m_allocator(new Chunk_list)
        , m_free_list(0)
        , m_block_size(align_block_size(block_size, sizeof(Link)))
        , m_blocks_per_chunk(blocks_per_chunk > 0 ? blocks_per_chunk : 10)
{}

Fixed_allocator::~Fixed_allocator()
{
    release();
    delete m_allocator;
}

void Fixed_allocator::release()
{
    m_allocator->release();
    m_free_list = 0;
}

void Fixed_allocator::replenish()
{
    assert(m_free_list == 0);
    size_t const chunk_size = m_block_size * m_blocks_per_chunk;

    // Allocate a new memory chunk.
    char* chunk = m_allocator->allocate(chunk_size);
    char* last = chunk + chunk_size - m_block_size;

    // Link the blocks together to create the free list.
    for (char* p = chunk; p <= last; p += m_block_size) {
        Link* curr = reinterpret_cast<Link*>(p);
        Link* next = reinterpret_cast<Link*>(p + m_block_size);
        curr->m_next = next;
    }

    // Terminate the list.
    Link* last_link = reinterpret_cast<Link*>(last);
    last_link->m_next = 0;

    // Assign the free list head to the start of the list.
    m_free_list = reinterpret_cast<Link*>(chunk);
}
