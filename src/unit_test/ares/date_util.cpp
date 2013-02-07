// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/bytes.hpp"
#include "ares/date.hpp"
#include "ares/date_util.hpp"
#include "ares/random.hpp"

using namespace std;
using namespace ares;

class Date_util_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_date_pickling()
    {
        // Testing this is easy. We take a large set of dates (some
        // predictable, some randomly chosen) and pickle and unpickle them. If
        // the output date is identical to the input date, the test passes.

        Bytes bytes;

        // Systematic
        for (int i = -100000; i <= 100000; i++) {
            Date d1(i), d2;
            to_bytes(bytes, d1);
            bool success = from_bytes(d2, bytes);
            CPPUNIT_ASSERT(success);
            CPPUNIT_ASSERT_EQUAL(d1, d2);
        }

        // Probabilistic
        for (int i = 0; i < 100000; i++) {
            Date d1(Random::next_int(2000000001)-1000000000), d2;
            to_bytes(bytes, d1);
            bool success = from_bytes(d2, bytes);
            CPPUNIT_ASSERT(success);
            CPPUNIT_ASSERT_EQUAL(d1, d2);
        }
    }

    CPPUNIT_TEST_SUITE(Date_util_tests);
    CPPUNIT_TEST(test_date_pickling);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Date_util_tests);
