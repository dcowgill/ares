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

using namespace std;
using namespace ares;
using ares::Bytes;

#define nelems(a) int(sizeof(a)/sizeof((a)[0]))

class Bytes_tests : public CppUnit::TestFixture {
    // This vector contains the bytes that make up the string "Hello, world!"
    vector<Byte> m_hello_world_bytes;

    string m_small_data;    // a short text string
    string m_large_data;    // a hundred times bigger than m_small_data

  public:
    void setUp()
    {
        // Populate m_hello_world_bytes
        char const text[] = "Hello, world!";
        for (char const* p = text; *p; p++)
            m_hello_world_bytes.push_back(static_cast<char>(*p));

        // Populate two sample data strings
        m_small_data = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        m_large_data.clear();
        for (int i = 0; i < 100; i++)
            m_large_data += m_small_data;
    }

    void tearDown()
    {
        m_hello_world_bytes.clear();
    }

    void test_empty()
    {
        Bytes bytes;
        CPPUNIT_ASSERT(bytes.capacity() >= 0);
        CPPUNIT_ASSERT_EQUAL(0, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string(""), bytes.to_string());
    }

    void test_construct_with_initial_capacity()
    {
        int const CAPACITY = 1234;
        Bytes bytes(CAPACITY);
        CPPUNIT_ASSERT(bytes.capacity() >= CAPACITY);
        CPPUNIT_ASSERT_EQUAL(0, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string(""), bytes.to_string());
    }

    void test_construct_with_empty_data()
    {
        Bytes bytes(0, 0);
        CPPUNIT_ASSERT(bytes.capacity() >= 0);
        CPPUNIT_ASSERT_EQUAL(0, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string(""), bytes.to_string());
    }

    void test_construct_with_data()
    {
        int const size = m_hello_world_bytes.size();
        Bytes bytes(&m_hello_world_bytes[0], size);
        CPPUNIT_ASSERT(bytes.capacity() >= size);
        CPPUNIT_ASSERT_EQUAL(size, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string("Hello, world!"), bytes.to_string());
    }

    void test_construct_with_empty_string()
    {
        string empty_string;
        Bytes bytes(empty_string);
        CPPUNIT_ASSERT(bytes.capacity() >= int(empty_string.size()));
        CPPUNIT_ASSERT_EQUAL(int(empty_string.size()), bytes.size());
        CPPUNIT_ASSERT_EQUAL(empty_string, bytes.to_string());
    }

    void test_construct_with_string()
    {
        Bytes bytes(m_small_data);
        CPPUNIT_ASSERT(bytes.capacity() >= int(m_small_data.size()));
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes.size());
        CPPUNIT_ASSERT_EQUAL(m_small_data, bytes.to_string());
    }

    void test_copy_constructor()
    {
        Bytes bytes1(m_small_data);
        Bytes bytes2(bytes1);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes2.capacity());
        CPPUNIT_ASSERT_EQUAL(bytes2.capacity(), bytes2.size());
    }

    void test_copy_constructor_of_empty_array()
    {
        Bytes bytes1;
        Bytes bytes2(bytes1);
        CPPUNIT_ASSERT_EQUAL(0, bytes2.capacity());
        CPPUNIT_ASSERT_EQUAL(bytes2.capacity(), bytes2.size());
    }

    void test_assignment()
    {
        Bytes bytes1(m_large_data);
        Bytes bytes2(m_small_data);
        bytes2 = bytes1;
        CPPUNIT_ASSERT(bytes2.capacity() >= int(m_large_data.size()));
        CPPUNIT_ASSERT_EQUAL(int(m_large_data.size()), bytes2.size());
    }

    void test_assign()
    {
        int const size = m_hello_world_bytes.size();
        Bytes bytes;
        bytes.assign(&m_hello_world_bytes[0], size);
        CPPUNIT_ASSERT(bytes.capacity() >= size);
        CPPUNIT_ASSERT_EQUAL(size, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string("Hello, world!"), bytes.to_string());

        // Shrink the array
        bytes.assign(&m_hello_world_bytes[0], size/2);
        CPPUNIT_ASSERT(bytes.capacity() >= size/2);
        CPPUNIT_ASSERT_EQUAL(size/2, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string("Hello,"), bytes.to_string());

        // Expand the array
        bytes.assign(&m_hello_world_bytes[0], size);
        CPPUNIT_ASSERT(bytes.capacity() >= size);
        CPPUNIT_ASSERT_EQUAL(size, bytes.size());
        CPPUNIT_ASSERT_EQUAL(string("Hello, world!"), bytes.to_string());
    }

    void test_swap()
    {
        Bytes bytes1(m_small_data);
        Bytes bytes2(m_large_data);

        int capacity1 = bytes1.capacity();
        int size1 = bytes1.size();
        int capacity2 = bytes2.capacity();
        int size2 = bytes2.size();

        bytes1.swap(bytes2);

        CPPUNIT_ASSERT_EQUAL(capacity1, bytes2.capacity());
        CPPUNIT_ASSERT_EQUAL(size1, bytes2.capacity());
        CPPUNIT_ASSERT_EQUAL(capacity2, bytes1.capacity());
        CPPUNIT_ASSERT_EQUAL(size2, bytes1.capacity());
    }

    void test_set_capacity()
    {
        Bytes bytes(m_small_data);

        bytes.set_capacity(m_small_data.size()*10);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()*10), bytes.capacity());
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes.size());

        bytes.set_capacity(m_small_data.size()/2);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()/2), bytes.capacity());
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()/2), bytes.size());
        CPPUNIT_ASSERT_EQUAL(m_small_data.substr(0, m_small_data.size()/2),
                             bytes.to_string());
    }

    void test_set_min_capacity()
    {
        Bytes bytes(m_small_data);

        bytes.set_min_capacity(m_small_data.size()*10);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()*10), bytes.capacity());
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes.size());
        CPPUNIT_ASSERT_EQUAL(m_small_data, bytes.to_string());

        bytes.set_min_capacity(1);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()*10), bytes.capacity());
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes.size());
        CPPUNIT_ASSERT_EQUAL(m_small_data, bytes.to_string());
    }

    void test_set_min_capacity_no_copy()
    {
        Bytes bytes(m_small_data);

        bytes.set_min_capacity_no_copy(m_small_data.size()*10);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()*10), bytes.capacity());
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes.size());

        bytes.set_min_capacity_no_copy(1);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()*10), bytes.capacity());
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()), bytes.size());
    }

    void test_set_size()
    {
        Bytes bytes(m_small_data);

        bytes.set_size(m_small_data.size()*10);
        CPPUNIT_ASSERT_EQUAL(bytes.capacity(), bytes.size());

        bytes.set_size(m_small_data.size()/2);
        CPPUNIT_ASSERT_EQUAL(int(m_small_data.size()/2), bytes.size());
        CPPUNIT_ASSERT_EQUAL(m_small_data.substr(0, m_small_data.size()/2),
                             bytes.to_string());
    }

    CPPUNIT_TEST_SUITE(Bytes_tests);
    CPPUNIT_TEST(test_empty);
    CPPUNIT_TEST(test_construct_with_initial_capacity);
    CPPUNIT_TEST(test_construct_with_empty_data);
    CPPUNIT_TEST(test_construct_with_data);
    CPPUNIT_TEST(test_construct_with_empty_string);
    CPPUNIT_TEST(test_construct_with_string);
    CPPUNIT_TEST(test_copy_constructor);
    CPPUNIT_TEST(test_copy_constructor_of_empty_array);
    CPPUNIT_TEST(test_assignment);
    CPPUNIT_TEST(test_assign);
    CPPUNIT_TEST(test_swap);
    CPPUNIT_TEST(test_set_capacity);
    CPPUNIT_TEST(test_set_min_capacity);
    CPPUNIT_TEST(test_set_min_capacity_no_copy);
    CPPUNIT_TEST(test_set_size);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Bytes_tests);
