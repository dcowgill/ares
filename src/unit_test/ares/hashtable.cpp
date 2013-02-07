// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/hashtable.hpp"

using namespace std;

typedef ares::Hashtable<int, int> Hashtable;

#define nelems(a) int(sizeof(a)/sizeof((a)[0]))

namespace
{
const int num_insertions = 10000;
}

class Hashtable_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_empty()
    {
        Hashtable h;
        CPPUNIT_ASSERT(h.is_empty());
        CPPUNIT_ASSERT_EQUAL(0, h.size());
        CPPUNIT_ASSERT(h.begin() == h.end());

        for (int i = 0; i < num_insertions; i++)
            CPPUNIT_ASSERT(!h.exists(i));
    }

    void test_insert()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            pair<Hashtable::Iterator, bool> p = h.insert(i,i*i);
            CPPUNIT_ASSERT(p.first != h.end());
            CPPUNIT_ASSERT(p.second);
        }

        for (int i = 0; i < num_insertions; i++) {
            pair<Hashtable::Iterator, bool> p = h.insert(i,i*i);
            CPPUNIT_ASSERT(!p.second);
        }

        for (int i = num_insertions-1; i >= 0; i--) {
            pair<Hashtable::Iterator, bool> p = h.insert(i,i*i);
            CPPUNIT_ASSERT(!p.second);
        }
    }

    void test_insert_and_find()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            pair<Hashtable::Iterator, bool> p = h.insert(i,i*i);
            CPPUNIT_ASSERT(p.second);
            Hashtable::Iterator iter = h.find(i);
            CPPUNIT_ASSERT(iter != h.end());
            CPPUNIT_ASSERT_EQUAL(i, iter->first);
            CPPUNIT_ASSERT_EQUAL(i*i, iter->second);
        }

        for (int i = 0; i < num_insertions; i++) {
            pair<Hashtable::Iterator, bool> p = h.insert(i,i*i);
            CPPUNIT_ASSERT(!p.second);
            Hashtable::Iterator iter = h.find(i);
            CPPUNIT_ASSERT(iter != h.end());
            CPPUNIT_ASSERT_EQUAL(i, iter->first);
            CPPUNIT_ASSERT_EQUAL(i*i, iter->second);
        }

        for (int i = num_insertions-1; i >= 0; i--) {
            pair<Hashtable::Iterator, bool> p = h.insert(i,i*i);
            CPPUNIT_ASSERT(!p.second);
            Hashtable::Iterator iter = h.find(i);
            CPPUNIT_ASSERT(iter != h.end());
            CPPUNIT_ASSERT_EQUAL(i, iter->first);
            CPPUNIT_ASSERT_EQUAL(i*i, iter->second);
        }
    }

    void test_subscript()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            h[i] = i*i;
            Hashtable::Iterator iter = h.find(i);
            CPPUNIT_ASSERT(iter != h.end());
            CPPUNIT_ASSERT_EQUAL(i, iter->first);
            CPPUNIT_ASSERT_EQUAL(i*i, iter->second);
        }

        for (int i = 0; i < num_insertions; i++) {
            h[i] *= i;
            Hashtable::Iterator iter = h.find(i);
            CPPUNIT_ASSERT(iter != h.end());
            CPPUNIT_ASSERT_EQUAL(i, iter->first);
            CPPUNIT_ASSERT_EQUAL(i*i*i, iter->second);
        }

        for (int i = num_insertions-1; i >= 0; i--) {
            CPPUNIT_ASSERT_EQUAL(i*i*i, h[i]);
        }
    }

    void test_erase()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            h.insert(i,i*i);
        }

        for (int i = 0; i < num_insertions; i += 2) {
            bool success = h.erase(i);
            CPPUNIT_ASSERT(success);
        }

        for (int i = 0; i < num_insertions; i++) {
            Hashtable::Iterator iter = h.find(i);
            if (i%2 == 0)
                CPPUNIT_ASSERT(iter == h.end());
            else
                CPPUNIT_ASSERT(iter != h.end());
        }

        for (int i = 1; i < num_insertions; i += 2) {
            bool success = h.erase(i);
            CPPUNIT_ASSERT(success);
        }

        CPPUNIT_ASSERT(h.is_empty());
        CPPUNIT_ASSERT_EQUAL(0, h.size());
    }

    void test_exists()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            CPPUNIT_ASSERT(!h.exists(i));
            h.insert(i,i*i);
            CPPUNIT_ASSERT(h.exists(i));
        }

        for (int i = 0; i < num_insertions; i++) {
            CPPUNIT_ASSERT(h.exists(i));
        }

        for (int i = 0; i < num_insertions; i += 2) {
            bool success = h.erase(i);
            CPPUNIT_ASSERT(success);
            CPPUNIT_ASSERT(!h.exists(i));
        }

        for (int i = 0; i < num_insertions; i++) {
            if (i%2 == 0)
                CPPUNIT_ASSERT(!h.exists(i));
            else
                CPPUNIT_ASSERT(h.exists(i));
        }

        for (int i = 1; i < num_insertions; i += 2) {
            bool success = h.erase(i);
            CPPUNIT_ASSERT(success);
            CPPUNIT_ASSERT(!h.exists(i));
        }
    }

    void test_iteration()
    {
        Hashtable h;
        int sum_keys1=0, sum_values1=0;
        int sum_keys2=0, sum_values2=0;

        for (int i = 0; i < num_insertions; i++) {
            h.insert(i,i*i);
            sum_keys1 += i;
            sum_values1 += i*i;
        }

        for (Hashtable::Iterator iter(h.begin()); iter != h.end(); ++iter) {
            sum_keys2 += iter->first;
            sum_values2 += iter->second;
        }

        CPPUNIT_ASSERT_EQUAL(sum_keys1, sum_keys2);
        CPPUNIT_ASSERT_EQUAL(sum_values1, sum_values2);
    }

    void test_size()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            h.insert(i,i*i);
            CPPUNIT_ASSERT_EQUAL(i + 1, h.size());
        }

        CPPUNIT_ASSERT_EQUAL(num_insertions, h.size());

        for (int i = 0; i < num_insertions; i++) {
            h.erase(i);
            CPPUNIT_ASSERT_EQUAL(num_insertions - i - 1, h.size());
        }

        CPPUNIT_ASSERT_EQUAL(0, h.size());
    }

    void test_clear()
    {
        Hashtable h;

        for (int i = 0; i < num_insertions; i++) {
            h.insert(i,i*i);
        }

        h.clear();

        CPPUNIT_ASSERT(h.is_empty());
        CPPUNIT_ASSERT_EQUAL(0, h.size());
        CPPUNIT_ASSERT(h.begin() == h.end());

        for (int i = 0; i < num_insertions; i++)
            CPPUNIT_ASSERT(!h.exists(i));
    }

    CPPUNIT_TEST_SUITE(Hashtable_tests);
    CPPUNIT_TEST(test_empty);
    CPPUNIT_TEST(test_insert);
    CPPUNIT_TEST(test_insert_and_find);
    CPPUNIT_TEST(test_subscript);
    CPPUNIT_TEST(test_erase);
    CPPUNIT_TEST(test_exists);
    CPPUNIT_TEST(test_iteration);
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_clear);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Hashtable_tests);
