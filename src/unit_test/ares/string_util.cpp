// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/string_util.hpp"

using namespace std;
using namespace ares;

class String_util_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}
    void tearDown() {}

    CPPUNIT_TEST_SUITE(String_util_tests);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(String_util_tests);
