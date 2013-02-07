// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/string_tokenizer.hpp"

using namespace std;
using namespace ares;
using ares::String_tokenizer;

#define CONSUME(tokenizer, token) {                             \
        CPPUNIT_ASSERT(tokenizer.has_next());                   \
        CPPUNIT_ASSERT_EQUAL(string(token), tokenizer.next());  \
    }

class String_tokenizer_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_basic()
    {
        String_tokenizer tok(" a bb  ccc   dddd   eee  ff g ");
        CONSUME(tok, "a");
        CONSUME(tok, "bb");
        CONSUME(tok, "ccc");
        CONSUME(tok, "dddd");
        CONSUME(tok, "eee");
        CONSUME(tok, "ff");
        CONSUME(tok, "g");
    }

    void test_various_whitespace_delimeters()
    {
        String_tokenizer tok(" a\n\rbb \n\n ccc \r  dddd   eee\n\n\nff\rg\n");
        CONSUME(tok, "a");
        CONSUME(tok, "bb");
        CONSUME(tok, "ccc");
        CONSUME(tok, "dddd");
        CONSUME(tok, "eee");
        CONSUME(tok, "ff");
        CONSUME(tok, "g");
    }

    void test_custom_delimiters()
    {
        String_tokenizer tok(" ONE 1 TWO2THREE3FOUR 4 FIVE5SIX 6 SEVEN  7",
                             "0123456789");
        CONSUME(tok, " ONE ");
        CONSUME(tok, " TWO");
        CONSUME(tok, "THREE");
        CONSUME(tok, "FOUR ");
        CONSUME(tok, " FIVE");
        CONSUME(tok, "SIX ");
        CONSUME(tok, " SEVEN  ");
    }

    void test_reset()
    {
        String_tokenizer tok("ONE1TWO22THREE333FOUR4444FIVE55555",
                             "0123456789");
        CONSUME(tok, "ONE");
        CONSUME(tok, "TWO");
        CONSUME(tok, "THREE");

        tok.reset("ONE1TWO22THREE333FOUR4444FIVE55555");
        CONSUME(tok, "ONE");
        CONSUME(tok, "TWO");
        CONSUME(tok, "THREE");
        CONSUME(tok, "FOUR");
        CONSUME(tok, "FIVE");

        tok.reset("ONE1TWO22THREE333FOUR4444FIVE55555");
        CONSUME(tok, "ONE");
        CONSUME(tok, "TWO");
        CONSUME(tok, "THREE");
        CONSUME(tok, "FOUR");
        CONSUME(tok, "FIVE");
    }

    void test_set_delim()
    {
        String_tokenizer tok("ONE1TWO22THREE333FOUR4444FIVE55555",
                             "0123456789");
        CONSUME(tok, "ONE");
        CONSUME(tok, "TWO");

        tok.set_delim("ABCDEFGHIJKLMNOPQRSTUVWXYZ");  // skips "22"
        CONSUME(tok, "333");
        CONSUME(tok, "4444");
        CONSUME(tok, "55555");
    }

    void test_reset_and_set_delim()
    {
        String_tokenizer tok(" ONE 1 TWO2THREE3FOUR 4 FIVE5SIX 6 SEVEN  7",
                             "0123456789");
        CONSUME(tok, " ONE ");
        CONSUME(tok, " TWO");
        CONSUME(tok, "THREE");
        CONSUME(tok, "FOUR ");

        tok.reset(" a bb  ccc   dddd   eee  ff g ");
        tok.set_delim(" ");
        CONSUME(tok, "a");
        CONSUME(tok, "bb");
        CONSUME(tok, "ccc");
        CONSUME(tok, "dddd");
        CONSUME(tok, "eee");
        CONSUME(tok, "ff");
        CONSUME(tok, "g");
    }

    void test_rest()
    {
        String_tokenizer tok(" a bb  ccc   dddd   eee  ff g ");
        CONSUME(tok, "a");
        CONSUME(tok, "bb");
        CONSUME(tok, "ccc");
        CPPUNIT_ASSERT_EQUAL(string("dddd   eee  ff g "), tok.rest());
    }

    void test_empty_rest()
    {
        String_tokenizer tok(" a bb  ccc   dddd   eee  ff g ");
        while (tok.has_next()) tok.next();
        CPPUNIT_ASSERT_EQUAL(string(""), tok.rest());
    }

    CPPUNIT_TEST_SUITE(String_tokenizer_tests);
    CPPUNIT_TEST(test_basic);
    CPPUNIT_TEST(test_various_whitespace_delimeters);
    CPPUNIT_TEST(test_custom_delimiters);
    CPPUNIT_TEST(test_reset);
    CPPUNIT_TEST(test_set_delim);
    CPPUNIT_TEST(test_reset_and_set_delim);
    CPPUNIT_TEST(test_rest);
    CPPUNIT_TEST(test_empty_rest);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(String_tokenizer_tests);
