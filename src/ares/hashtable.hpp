// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_hashtable
#define included_ares_hashtable

#include "ares/fixed_allocator.hpp"
#include "ares/utility.hpp"
#include <functional>       // for std::equal_to
#include <cstring>
#include <new>
#include <utility>

namespace ares {

// The default hash function simply casts to a 32-bit unsigned integer. Not
// appropriate for strings; see string_util.h for a good string hash function.
template<typename K> struct Default_hash {
    unsigned operator()(K const& key) const {
        return unsigned(key);
    }
};

// An efficient hashtable data structure. Although modeled after the stl
// containters, this class doesn't attempt to meet the requirements of one.
// The table will automatically resize itself when its load factor exceeds a
// certain threshold.
template
<
    typename K,                     // key type
    typename V,                     // value type
    class H = Default_hash<K>,      // computes hash values
    class E = std::equal_to<K>      // compares keys
    >
class Hashtable : boost::noncopyable {
  private:
    struct Slot {
        Slot* m_next;                   // next slot
        std::pair<K const, V> m_pair;   // key-value pair

        // Creators
        Slot(Slot* next, K const& key, V const& value)
                : m_next(next), m_pair(key, value) {}

        // Accessors
        K const& key() { return m_pair.first; }
        V& val() { return m_pair.second; }
    };

    Fixed_allocator m_pool; // pool allocator for slots
    H m_hash;               // hash function object
    E m_equal_to;           // equal-to function object
    int m_capacity;         // physical size of m_slots
    int m_mask;             // always equal to (m_capacity-1)
    int m_size;             // number of elements in table
    Slot** m_slots;         // the hashtable

  public:
    // A hashtable iterator. Its interface is essentially identical to that
    // used by those used by the stl associative containers, e.g. std::map.
    class Iterator {
      public:
        Iterator(Hashtable const* table, Slot* s);
        Iterator(Iterator const& i);
        Iterator& operator=(Iterator const& i);
        void operator++();
        std::pair<K const, V>& operator*() const;
        std::pair<K const, V>* operator->() const;
        bool operator==(Iterator const& i) const;
        bool operator!=(Iterator const& i) const;

      private:
        Hashtable const* m_table;   // parent hashtable
        Slot* m_slot;               // curr. link in chain
    };

    friend class Iterator;

  public:
    // Constructs an empty hash table. The caller may optionally hint at the
    // maximum number of elements this table will hold, which may allows it to
    // avoid some memory allocations when elements are inserted.
    Hashtable(int size_hint = 0);

    // Destructor.
    ~Hashtable();

    // Removes all elements from this table.
    void clear();

    // Inserts the specified key-value into this table. If the key already
    // exists in this table, this function has no effect. Returns a pair of
    // values: the iterator pointing to the key and its value, and a boolean
    // indicating whether the insertion was successful.
    std::pair<Iterator, bool> insert(K const& key, V const& value);

    // Erases the specified key from this table, returning true if it was
    // found and false otherwise.
    bool erase(K const& key);

    // Erases the key-value pair pointed to by the specified iterator.
    void erase(Iterator const& iter);

    // Returns a reference to the value associated with the specified key. If
    // the key doesn't exist in this table, it is inserted with a default
    // value, and a reference to that value is returned.
    V& operator[](K const& key);

    // Returns the beginning of the iteration for the elements of this table.
    Iterator begin() const;

    // Returns an iterator representing the item one past the final item in
    // this table. Thus, this iterator can represent the end of any iteration,
    // or a non-existent key.
    Iterator end() const;

    // Returns an iterator pointing to the entry for the specified key. If the
    // key is not in this table, returns the end iterator.
    Iterator find(K const& key) const;

    // Tests whether the specified key is in this table.
    bool exists(K const& key) const { return find(key) != end(); }

    // Returns the number of keys-value pairs in this table.
    int size() const { return m_size; }

    // Tests whether this table is empty.
    bool is_empty() const { return !m_size; }

    // Returns this table's current load factor, which is defined as the ratio
    // of its number of entries to its total number of hash buckets.
    double load_factor() const { return 1.0*m_size/m_capacity; }

    // Returns the number of key comparisons necessary to either locate the
    // specified key or determine that the key isn't in the table.
    int num_probes(K const& key) const;

  private:
    int init_capacity(int hint) const;
    int bucket(K const& key) const { return m_hash(key) & m_mask; }
    Slot** find_slot(K const& key) const;
    void resize_on_insert(int size);
    void resize_on_erase(int size);
    void rehash(Slot** old_slots, int old_capacity);
};

// Returned by the hashtable_statistics function (see below).
struct Hashtable_statistics {
    int size;               // number of elements in the table
    double load_factor;     // proportion of buckets that are occupied
    double mean_probes;     // mean probes required per key
    int max_probes;         // maximum no. of probes required for any key
};

// Gathers some statistics on a given hashtable and returns them in a
// Hashtable_statistics object. Useful primarily for debugging performance
// problems in the hashtable implementation.
template<typename K, typename V, class H, class E>
Hashtable_statistics hashtable_statistics(Hashtable<K,V,H,E> const& table);


// ##################################################################
// The following consists of function definitions for this component.
// ##################################################################

// +-----------+
// | Hashtable |
// +-----------+

template<typename K, typename V, class H, class E>
Hashtable<K,V,H,E>::Hashtable(int size_hint)
        : m_pool(sizeof(Slot))
        , m_capacity(init_capacity(size_hint))
        , m_mask(m_capacity-1)
        , m_size(0)
        , m_slots(new Slot*[m_capacity])
{
    std::memset(m_slots, 0, m_capacity * sizeof(m_slots[0]));
}

template<typename K, typename V, class H, class E>
Hashtable<K,V,H,E>::~Hashtable()
{
    clear();
    delete [] m_slots;
}

template<typename K, typename V, class H, class E>
void Hashtable<K,V,H,E>::clear()
{
    // call destructors for all slots
    for (int i = 0; i < m_capacity; i++)
        for (Slot* p = m_slots[i]; p; p = p->m_next)
            p->~Slot();

    m_size = 0;
    std::memset(m_slots, 0, m_capacity * sizeof(m_slots[0]));
    m_pool.release();  // frees all memory allocated to slots
}

template<typename K, typename V, class H, class E>
std::pair<typename Hashtable<K,V,H,E>::Iterator, bool>
Hashtable<K,V,H,E>::insert(K const& key, V const& value)
{
    resize_on_insert(m_size + 1);

    int const slot_num = bucket(key);
    Slot* first = m_slots[slot_num];

    for (Slot* p = first; p; p = p->m_next)
        if (m_equal_to(p->key(), key))
            return std::make_pair(Iterator(this, p), false);

    Slot* p = new (m_pool.allocate()) Slot(first, key, value);
    m_slots[slot_num] = p;
    m_size++;
    return std::make_pair(Iterator(this, p), true);
}

template<typename K, typename V, class H, class E>
bool Hashtable<K,V,H,E>::erase(K const& key)
{
    resize_on_erase(m_size - 1);

    Slot** slot = find_slot(key);
    if (!*slot) return false;
    Slot* q = *slot;
    *slot = (*slot)->m_next;
    q->~Slot();
    m_pool.free(q); //delete q;
    m_size--;
    return true;
}

template<typename K, typename V, class H, class E>
void Hashtable<K,V,H,E>::erase(Iterator const& iter)
{
    // TODO: could this be made more efficient?
    erase(iter->first);
}

template<typename K, typename V, class H, class E>
V& Hashtable<K,V,H,E>::operator[](K const& key)
{
    resize_on_insert(m_size + 1);

    int const slot_num = bucket(key);
    Slot* first = m_slots[slot_num];

    for (Slot* p = first; p; p = p->m_next)
        if (m_equal_to(p->key(), key))
            return p->val();

    Slot* p = new (m_pool.allocate()) Slot(first, key, V());
    m_slots[slot_num] = p;
    m_size++;
    return p->val();
}

template<typename K, typename V, class H, class E>
inline typename Hashtable<K,V,H,E>::Iterator
Hashtable<K,V,H,E>::begin() const
{
    for (int i = 0; i < m_capacity; i++)
        if (m_slots[i])
            return Iterator(this, m_slots[i]);
    return end();
}

template<typename K, typename V, class H, class E>
inline typename Hashtable<K,V,H,E>::Iterator
Hashtable<K,V,H,E>::end() const
{
    return Iterator(this, 0);
}

template<typename K, typename V, class H, class E>
inline typename Hashtable<K,V,H,E>::Iterator
Hashtable<K,V,H,E>::find(K const& key) const
{
    for (Slot* p = m_slots[bucket(key)]; p; p = p->m_next)
        if (m_equal_to(p->key(), key))
            return Iterator(this, p);
    return end();
}

template<typename K, typename V, class H, class E>
int Hashtable<K,V,H,E>::num_probes(K const& key) const
{
    int n = 1;
    for (Slot* p = m_slots[bucket(key)]; p; p = p->m_next, n++)
        if (m_equal_to(p->key(), key))
            break;
    return n;
}

// Returns a power of two greater than or equal to max(hint, 32).
template<typename K, typename V, class H, class E>
int Hashtable<K,V,H,E>::init_capacity(int hint) const
{
    if (hint < 32) return 32;
    int n = 64;
    while (n < hint) n *= 2;
    return n;
}

// Returns a pointer to the slot for key, or a pointer to null if not found.
template<typename K, typename V, class H, class E>
typename Hashtable<K,V,H,E>::Slot**
Hashtable<K,V,H,E>::find_slot(K const& key) const
{
    Slot** p = &m_slots[bucket(key)];
    for (; *p; p = &(*p)->m_next)
        if (m_equal_to((*p)->key(), key))
            break;
    return p;
}

// Resizes table if necessary on insert to handle specified size.
template<typename K, typename V, class H, class E>
void Hashtable<K,V,H,E>::resize_on_insert(int size)
{
    // don't let load factor exceed 0.5
    if (2*size > m_capacity) {
        int old_capacity = m_capacity;
        m_capacity *= 4;                // load factor will be 0.25
        m_mask = m_capacity - 1;
        Slot** old_slots = m_slots;
        m_slots = new Slot*[m_capacity];
        std::memset(m_slots, 0, m_capacity * sizeof(m_slots[0]));
        rehash(old_slots, old_capacity);
    }
}

// Resizes table if necessary on erasure to handle specified size.
template<typename K, typename V, class H, class E>
void Hashtable<K,V,H,E>::resize_on_erase(int size)
{
    // don't let load factor drop below 0.1
    if (size*10 < m_capacity && m_capacity > 32) {
        int old_capacity = m_capacity;
        m_capacity /= 2;                // load factor will be 0.2
        m_mask = m_capacity - 1;
        Slot** old_slots = m_slots;
        m_slots = new Slot*[m_capacity];
        std::memset(m_slots, 0, m_capacity * sizeof(m_slots[0]));
        rehash(old_slots, old_capacity);
    }
}

// Rehashes the table; used by resize_on_insert and resize_on_erase.
template<typename K, typename V, class H, class E>
void Hashtable<K,V,H,E>::rehash(Slot** old_slots, int old_capacity)
{
    for (int i = 0; i < old_capacity; i++) {
        Slot* p = old_slots[i];
        while (p) {
            Slot* q = p;
            p = p->m_next;
            Slot** slot = &m_slots[bucket(q->key())];
            q->m_next = *slot;
            *slot = q;
        }
    }
    delete [] old_slots;
}

// +---------------------+
// | Hashtable::Iterator |
// +---------------------+

template<typename K, typename V, class H, class E>
inline Hashtable<K,V,H,E>::Iterator::Iterator(Hashtable const* table, Slot* s)
        : m_table(table)
        , m_slot(s)
{}

template<typename K, typename V, class H, class E>
inline Hashtable<K,V,H,E>::Iterator::Iterator(Iterator const& i)
        : m_table(i.m_table)
        , m_slot(i.m_slot)
{}

template<typename K, typename V, class H, class E>
inline typename Hashtable<K,V,H,E>::Iterator&
Hashtable<K,V,H,E>::Iterator::operator=(Iterator const& i)
{
    m_table = i.m_table;
    m_slot = i.m_slot;
    return *this;
}

template<typename K, typename V, class H, class E>
void Hashtable<K,V,H,E>::Iterator::operator++()
{
    Slot* old = m_slot;

    if (m_slot)
        m_slot = m_slot->m_next;

    if (!m_slot) {
        int n = m_table->bucket(old->key()) + 1;
        for (; n < m_table->m_capacity; n++) {
            if (m_table->m_slots[n]) {
                m_slot = m_table->m_slots[n];
                return;
            }
        }
    }
}

template<typename K, typename V, class H, class E>
inline std::pair<K const,V>& Hashtable<K,V,H,E>::Iterator::operator*() const
{
    return m_slot->m_pair;
}

template<typename K, typename V, class H, class E>
inline std::pair<K const,V>* Hashtable<K,V,H,E>::Iterator::operator->() const
{
    return &m_slot->m_pair;
}

template<typename K, typename V, class H, class E>
inline bool Hashtable<K,V,H,E>::Iterator::operator==(Iterator const& i) const
{
    return m_slot == i.m_slot;
}

template<typename K, typename V, class H, class E>
inline bool Hashtable<K,V,H,E>::Iterator::operator!=(Iterator const& i) const
{
    return !(*this == i);
}

// +----------------+
// | Free functions |
// +----------------+

template<typename K, typename V, class H, class E>
Hashtable_statistics hashtable_statistics(Hashtable<K,V,H,E> const& table)
{
    typedef typename Hashtable<K,V,H,E>::Iterator Iter;

    Hashtable_statistics stats;
    stats.size = table.size();
    stats.load_factor = table.load_factor();
    stats.mean_probes = 0.0;
    stats.max_probes = 0;

    // Iterate over keys in table, computing search cost for each one.
    Iter end = table.end();
    for (Iter i = table.begin(); i != end; ++i) {
        int n = table.num_probes(i->first);
        if (stats.max_probes < n) stats.max_probes = n;
        stats.mean_probes += n;
    }
    stats.mean_probes /= stats.size;
    return stats;
}

} // namespace ares

#endif
