// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/random.hpp"
#include "ares/sync_queue.hpp"
#include <list>

using namespace std;
using namespace ares;

namespace
{
const int num_insertions = 10000;
}

class Sync_queue_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_empty()
    {
        Sync_queue<int> q;
        CPPUNIT_ASSERT(q.is_empty());
        CPPUNIT_ASSERT_EQUAL(0, q.size());
    }

    void test_enqueue_and_size()
    {
        Sync_queue<int> q;

        for (int i = 0; i < num_insertions; i++) {
            CPPUNIT_ASSERT_EQUAL(i, q.size());
            q.enqueue(0);
            CPPUNIT_ASSERT_EQUAL(i+1, q.size());
        }
    }

    void test_enqueue_dequeue()
    {
        Sync_queue<int> q;

        for (int i = 0; i < num_insertions; i++) {
            q.enqueue(i*i);
        }
        CPPUNIT_ASSERT_EQUAL(num_insertions, q.size());

        for (int i = 0; i < num_insertions; i++) {
            int const j = q.dequeue();
            CPPUNIT_ASSERT_EQUAL(j, i*i);
        }
        CPPUNIT_ASSERT(q.is_empty());
    }

    void test_enqueue_dequeue_all()
    {
        Sync_queue<int> q;

        for (int i = 0; i < num_insertions; i++) {
            q.enqueue(i*i);
        }
        CPPUNIT_ASSERT_EQUAL(num_insertions, q.size());

        vector<int> v;

        // Insert w/ dummy values to make sure dequeue_all clears the vector.
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        int num_dequeued = q.dequeue_all(v);
        CPPUNIT_ASSERT_EQUAL(int(v.size()), num_dequeued);
        CPPUNIT_ASSERT_EQUAL(num_insertions, num_dequeued);
        CPPUNIT_ASSERT(q.is_empty());

        for (int i = 0; i < num_insertions; i++) {
            CPPUNIT_ASSERT_EQUAL(i*i, v[i]);
        }
    }

    void test_repeated_enqueue_dequeue()
    {
        Sync_queue<int> q;

        for (int n = 0; n < 10; n++) {
            for (int i = 0; i < num_insertions; i++) {
                q.enqueue(i*i);
            }
            CPPUNIT_ASSERT_EQUAL(num_insertions, q.size());

            for (int i = 0; i < num_insertions; i++) {
                int const j = q.dequeue();
                CPPUNIT_ASSERT_EQUAL(j, i*i);
            }
            CPPUNIT_ASSERT(q.is_empty());
        }

        CPPUNIT_ASSERT(q.is_empty());
    }

    void test_random_enqueue_dequeue()
    {
        list<int> control;  // this is a known-good queue implementation
        Sync_queue<int> q;

        // Enqueue N random integers then dequeue M (N and M are randomly
        // generated integers where 1000<=N<=10000 and M<=queue_size).
        //
        // Repeat this process 10 times.

        for (int i = 0; i < 10; i++) {
            int const N = Random::value_in_range(1000, 10000);
            for (int n = 0; n < N; n++) {
                int const r = Random::next_int(1000000);
                q.enqueue(r);
                control.push_front(r);
            }

            int const queue_size = q.size();
            CPPUNIT_ASSERT_EQUAL(int(control.size()), queue_size);

            int const M = Random::value_in_range(0, queue_size);
            for (int m = 0; m < M; m++) {
                int const actual = q.dequeue();
                int const expected = control.back();
                control.pop_back();
                CPPUNIT_ASSERT_EQUAL(expected, actual);
            }

            CPPUNIT_ASSERT_EQUAL(int(control.size()), q.size());
        }

        CPPUNIT_ASSERT_EQUAL(int(control.size()), q.size());

        // Dequeue whatever's left in the queues.

        while (!q.is_empty()) {
            CPPUNIT_ASSERT(!control.empty());
            int const actual = q.dequeue();
            int const expected = control.back();
            control.pop_back();
            CPPUNIT_ASSERT_EQUAL(expected, actual);
        }
        CPPUNIT_ASSERT(control.empty());
    }

    void test_random_enqueue_dequeue_all()
    {
        list<int> control;  // this is a known-good queue implementation
        Sync_queue<int> q;

        // Enqueue N random integers then dequeue all (N is a randomly
        // generated integer where 1000<=N<=10000);
        //
        // Repeat this process 10 times.

        vector<int> v;

        for (int i = 0; i < 10; i++) {
            int const N = Random::value_in_range(1000, 10000);
            for (int n = 0; n < N; n++) {
                int const r = Random::next_int(1000000);
                q.enqueue(r);
                control.push_front(r);
            }

            int const queue_size = q.size();
            CPPUNIT_ASSERT_EQUAL(int(control.size()), queue_size);

            int num_dequeued = q.dequeue_all(v);
            CPPUNIT_ASSERT_EQUAL(int(v.size()), num_dequeued);
            CPPUNIT_ASSERT_EQUAL(N, num_dequeued);
            CPPUNIT_ASSERT(q.is_empty());

            for (int i = 0; i < N; i++) {
                CPPUNIT_ASSERT(!control.empty());
                int const r1 = control.back();
                control.pop_back();
                CPPUNIT_ASSERT_EQUAL(r1, v[i]);
            }
            CPPUNIT_ASSERT(control.empty());
        }
    }

    CPPUNIT_TEST_SUITE(Sync_queue_tests);
    CPPUNIT_TEST(test_empty);
    CPPUNIT_TEST(test_enqueue_and_size);
    CPPUNIT_TEST(test_enqueue_dequeue);
    CPPUNIT_TEST(test_enqueue_dequeue_all);
    CPPUNIT_TEST(test_repeated_enqueue_dequeue);
    CPPUNIT_TEST(test_random_enqueue_dequeue);
    CPPUNIT_TEST(test_random_enqueue_dequeue_all);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Sync_queue_tests);
