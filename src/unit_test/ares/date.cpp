// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/date.hpp"
#include "ares/random.hpp"

using namespace std;
using namespace ares;
using ares::Date;
using ares::Random;

#define CHECK_DATE(y,m,d,h,i,s, date) {                                 \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(year)", y, date.year());         \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(month)", m, date.month());       \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(day)", d, date.day());           \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(hour)", h, date.hour());         \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(minutes)", i, date.minutes());   \
        CPPUNIT_ASSERT_EQUAL_MESSAGE("(seconds)", s, date.seconds());   \
    }

class Date_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_default_date()
    {
        Date d;
        CHECK_DATE(1970,1,1,0,0,0, d);
    }

    void test_construct_with_timestamp()
    {
        Date d1(946736709);
        CHECK_DATE(2000,1,1,14,25,9, d1);

        Date d2(-885754019);
        CHECK_DATE(1941,12,7,5,13,1, d2);

        Date d3(0);
        CHECK_DATE(1970,1,1,0,0,0, d3);

        Date d4(-1);
        CHECK_DATE(1969,12,31,23,59,59, d4);

        Date d5(2135118600);
        CHECK_DATE(2037,8,29,0,30,0, d5);

        Date d6(197208001);
        CHECK_DATE(1976,4,1,12,0,1, d6);

        Date d7(-2000000000);
        CHECK_DATE(1906,8,16,20,26,40, d7);

        Date d8(2000000000);
        CHECK_DATE(2033,5,18,3,33,20, d8);
    }

    void test_construct_with_valid_date()
    {
        Date d(2004, 2, 14, 16, 30, 8);  // 14-FEB-2004 @ 16:30:08
        CHECK_DATE(2004,2,14,16,30,8, d);
    }

    void test_set_with_valid_date()
    {
        Date d(2004, 2, 14, 16, 30, 8);  // 14-FEB-2004 @ 16:30:08
        d.set(1988, 12, 31, 23, 59, 59); // 31-DEC-1988 @ 23:59:59
        CHECK_DATE(1988,12,31,23,59,59, d);
    }

    void test_set_without_hour_min_sec()
    {
        Date d;

        // without seconds
        d.set(2004, 2, 14, 8, 45);
        CHECK_DATE(2004,2,14,8,45,0, d);

        // without minutes, seconds
        d.set(2004, 2, 14, 8);
        CHECK_DATE(2004,2,14,8,0,0, d);

        // without hours, minutes, seconds
        d.set(2004, 2, 14);
        CHECK_DATE(2004,2,14,0,0,0, d);
    }

    void test_set_with_invalid_month()
    {
        Date d;

        // should become Dec 15, 1999
        d.set(2000, 0, 15);
        CHECK_DATE(1999,12,15,0,0,0, d);

        // should become Jul 15, 1999
        d.set(2000, -5, 15);
        CHECK_DATE(1999,7,15,0,0,0, d);

        // should become Dec 1, 1999
        d.set(2000, -1, 31);
        CHECK_DATE(1999,12,1,0,0,0, d);

        // should become Mar 3, 1999
        d.set(2000, -10, 31);
        CHECK_DATE(1999,3,3,0,0,0, d);

        // should become Dec 31, 1998
        d.set(2000, -12, 31);
        CHECK_DATE(1998,12,31,0,0,0, d);

        // should become Dec 1, 1997
        d.set(2000, -25, 31);
        CHECK_DATE(1997,12,1,0,0,0, d);

        // should become Jul 1, 1971
        d.set(2000, -341, 1);
        CHECK_DATE(1971,7,1,0,0,0, d);

        // should become Jan 1, 2001
        d.set(2000, 13, 1);
        CHECK_DATE(2001,1,1,0,0,0, d);

        // should become Mar 3, 2001
        d.set(2000, 14, 31);
        CHECK_DATE(2001,3,3,0,0,0, d);

        // should become Mar 3, 2065
        d.set(2000, 782, 31);
        CHECK_DATE(2065,3,3,0,0,0, d);
    }

    void test_set_with_invalid_day()
    {
        Date d;

        // should become Dec 31, 1999
        d.set(2000, 1, 0);
        CHECK_DATE(1999,12,31,0,0,0, d);

        // should become Dec 30, 1999
        d.set(2000, 1, -1);
        CHECK_DATE(1999,12,30,0,0,0, d);

        // should become Nov 30, 1999
        d.set(2000, 1, -31);
        CHECK_DATE(1999,11,30,0,0,0, d);

        // should become Jan 1, 1999
        d.set(2000, 1, -364);
        CHECK_DATE(1999,1,1,0,0,0, d);

        // should become Mar 1, 2000
        d.set(2000, 2, 30);
        CHECK_DATE(2000,3,1,0,0,0, d);

        // should become Mar 1, 1999
        d.set(1999, 2, 29);
        CHECK_DATE(1999,3,1,0,0,0, d);

        // should become Feb 29, 2000
        d.set(2000, 3, 0);
        CHECK_DATE(2000,2,29,0,0,0, d);

        // should become Feb 28, 1999
        d.set(1999, 3, 0);
        CHECK_DATE(1999,2,28,0,0,0, d);

        // should become Jul 28, 2000
        d.set(2000, 5, 89);
        CHECK_DATE(2000,7,28,0,0,0, d);

        // should become Jan 1, 2000
        d.set(1999, 1, 366);
        CHECK_DATE(2000,1,1,0,0,0, d);

        // should become Jan 1, 2001
        d.set(2000, 1, 367);
        CHECK_DATE(2001,1,1,0,0,0, d);
    }

    void test_set_with_invalid_hour()
    {}

    void test_set_with_invalid_minutes()
    {}

    void test_set_with_invalid_seconds()
    {}

    void test_day_of_year()
    {
        Date d;

        // Jun 13, 1978
        d.set(1978, 6, 13);
        CPPUNIT_ASSERT_EQUAL(164, d.day_of_year());

        // Dec 7, 1941
        d.set(1941, 12, 7);
        CPPUNIT_ASSERT_EQUAL(341, d.day_of_year());

        // Jun 6, 1944
        d.set(1944, 6, 6);
        CPPUNIT_ASSERT_EQUAL(158, d.day_of_year());

        // Sep 1, 2012
        d.set(2012, 9, 1);
        CPPUNIT_ASSERT_EQUAL(245, d.day_of_year());

        // Dec 31, 2000
        d.set(2000, 12, 31);
        CPPUNIT_ASSERT_EQUAL(366, d.day_of_year());

        // Dec 31, 1999
        d.set(1999, 12, 31);
        CPPUNIT_ASSERT_EQUAL(365, d.day_of_year());

        // Jan 1, 1900
        d.set(1900, 1, 1);
        CPPUNIT_ASSERT_EQUAL(1, d.day_of_year());
    }

    void test_day_of_week()
    {
        Date d;

        // Test a week in Feb 2000
        for (int i = 0; i <= 6; i++) {
            d.set(2000, 2, i+13);
            CPPUNIT_ASSERT_EQUAL(i, d.day_of_week());
        }

        // Test a week in Dec 2006
        for (int i = 0; i <= 6; i++) {
            d.set(2006, 12, i+24);
            CPPUNIT_ASSERT_EQUAL(i, d.day_of_week());
        }

        // Test a week in Apr 1865
        for (int i = 0; i <= 6; i++) {
            d.set(1865, 4, i+16);
            CPPUNIT_ASSERT_EQUAL(i, d.day_of_week());
        }

        // Test a week spanning Jul-Aug 1945
        d.set(1945, 7, 29);
        CPPUNIT_ASSERT_EQUAL(0, d.day_of_week());
        d.set(1945, 7, 30);
        CPPUNIT_ASSERT_EQUAL(1, d.day_of_week());
        d.set(1945, 7, 31);
        CPPUNIT_ASSERT_EQUAL(2, d.day_of_week());
        d.set(1945, 8, 1);
        CPPUNIT_ASSERT_EQUAL(3, d.day_of_week());
        d.set(1945, 8, 2);
        CPPUNIT_ASSERT_EQUAL(4, d.day_of_week());
        d.set(1945, 8, 3);
        CPPUNIT_ASSERT_EQUAL(5, d.day_of_week());
        d.set(1945, 8, 4);
        CPPUNIT_ASSERT_EQUAL(6, d.day_of_week());
    }

    void test_compare()
    {
        // This function systematically compares N^2 date pairs

        int const MIN = -2000000000;    // 1906-08-16 20:26:40
        int const MAX =  2000000000;    // 2033-05-18 03:33:20

        // This increment gives us ((max-min)/incr)^2 = 1.6*10^9 pairs
        int const INCR = 100000;

        for (int i = MIN; i <= MAX; i += INCR) {
            for (int j = -MIN; j <= MAX; j += INCR) {
                if (i < j) {
                    CPPUNIT_ASSERT(Date(i).compare(Date(j)) < 0);
                    CPPUNIT_ASSERT(Date(j).compare(Date(i)) > 0);
                }
                else if (i > j) {
                    CPPUNIT_ASSERT(Date(i).compare(Date(j)) > 0);
                    CPPUNIT_ASSERT(Date(j).compare(Date(i)) < 0);
                }
                else {
                    CPPUNIT_ASSERT(Date(i).compare(Date(j)) ==0);
                    CPPUNIT_ASSERT(Date(j).compare(Date(i)) ==0);
                }
            }
        }
    }

    void test_compare_probabilistic()
    {
        // This function generates N random date pairs to compare

        int const MIN = -1000000000;
        int const MAX =  1000000000;

        // Perform 100,000 comparisons
        for (int i = 0; i <= 100000; i++) {
            int t1 = Random::next_int(MAX-MIN+1)+MIN;
            int t2 = Random::next_int(MAX-MIN+1)+MIN;

            if (t1 < t2) {
                CPPUNIT_ASSERT(Date(t1).compare(Date(t2)) < 0);
                CPPUNIT_ASSERT(Date(t2).compare(Date(t1)) > 0);
            }
            else if (t1 > t2) {
                CPPUNIT_ASSERT(Date(t1).compare(Date(t2)) > 0);
                CPPUNIT_ASSERT(Date(t2).compare(Date(t1)) < 0);
            }
            else {
                CPPUNIT_ASSERT(Date(t1).compare(Date(t2)) ==0);
                CPPUNIT_ASSERT(Date(t2).compare(Date(t1)) ==0);
            }
        }
    }

    void test_to_timestamp()
    {
        // To test the to_timestamp function, it is sufficient to test that
        // the following condition holds for all t: Date(t).to_timestamp()==t
        //
        // Since we can't test for all values of t, we can try a few edge
        // cases, then test a large number of values at random.

        for (int i = -1000000; i <= 1000000; i++)
            CPPUNIT_ASSERT_EQUAL(time_t(i), Date(i).to_timestamp());

        CPPUNIT_ASSERT_EQUAL(time_t(-1000000000),
                             Date(-1000000000).to_timestamp());
        CPPUNIT_ASSERT_EQUAL(time_t( 1000000000),
                             Date( 1000000000).to_timestamp());
        CPPUNIT_ASSERT_EQUAL(time_t(-2147483647),
                             Date(-2147483647).to_timestamp());
        CPPUNIT_ASSERT_EQUAL(time_t( 2147483647),
                             Date( 2147483647).to_timestamp());

        // Test a large number of random timestamps in the range [-1B,1B]
        for (int i = 0; i <= 100000; i++) {
            time_t t = Random::next_int(2000000001)-1000000000;
            CPPUNIT_ASSERT_EQUAL(t, Date(t).to_timestamp());
        }
    }

    void test_default_to_string()
    {
        // This test is simple because the to_string function's default
        // behavior is trivial. Therefore we need only prove its contract.
        // The standard ISO-8601 format is "yyyy-mm-dd hh24:mi:ss"

        Date d(2012, 6, 1, 22, 5, 8);
        CPPUNIT_ASSERT_EQUAL(string("2012-06-01 22:05:08"), d.to_string());
    }

    CPPUNIT_TEST_SUITE(Date_tests);
    CPPUNIT_TEST(test_default_date);
    CPPUNIT_TEST(test_construct_with_timestamp);
    CPPUNIT_TEST(test_construct_with_valid_date);
    CPPUNIT_TEST(test_set_with_valid_date);
    CPPUNIT_TEST(test_set_without_hour_min_sec);
    CPPUNIT_TEST(test_set_with_invalid_month);
    CPPUNIT_TEST(test_set_with_invalid_day);
    CPPUNIT_TEST(test_set_with_invalid_hour);
    CPPUNIT_TEST(test_set_with_invalid_minutes);
    CPPUNIT_TEST(test_set_with_invalid_seconds);
    CPPUNIT_TEST(test_day_of_year);
    CPPUNIT_TEST(test_day_of_week);
    CPPUNIT_TEST(test_compare);
    CPPUNIT_TEST(test_compare_probabilistic);
    CPPUNIT_TEST(test_to_timestamp);
    CPPUNIT_TEST(test_default_to_string);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Date_tests);
