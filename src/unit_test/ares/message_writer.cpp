// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/data_reader.hpp"
#include "ares/message_writer.hpp"
#include "unit_test/ares/test_sink.h"

using namespace std;
using namespace ares;

namespace
{
// (for readability)
const int MIN_PACKET_SIZE = Message_writer::MIN_PACKET_SIZE;
const int MAX_PACKET_SIZE = Message_writer::MAX_PACKET_SIZE;
}

class Message_writer_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_capacity()
    {
        Message_writer writer;
        CPPUNIT_ASSERT(MIN_PACKET_SIZE < MAX_PACKET_SIZE);

        writer.set_max_packet_size(MIN_PACKET_SIZE-1);
        CPPUNIT_ASSERT_EQUAL(MIN_PACKET_SIZE, writer.max_packet_size());

        writer.set_max_packet_size(MIN_PACKET_SIZE-100);
        CPPUNIT_ASSERT_EQUAL(MIN_PACKET_SIZE, writer.max_packet_size());

        writer.set_max_packet_size(MIN_PACKET_SIZE);
        CPPUNIT_ASSERT_EQUAL(MIN_PACKET_SIZE, writer.max_packet_size());

        writer.set_max_packet_size(MIN_PACKET_SIZE+1);
        CPPUNIT_ASSERT_EQUAL(MIN_PACKET_SIZE+1, writer.max_packet_size());

        writer.set_max_packet_size(MIN_PACKET_SIZE+100);
        CPPUNIT_ASSERT_EQUAL(MIN_PACKET_SIZE+100, writer.max_packet_size());

        writer.set_max_packet_size(MAX_PACKET_SIZE-100);
        CPPUNIT_ASSERT_EQUAL(MAX_PACKET_SIZE-100, writer.max_packet_size());

        writer.set_max_packet_size(MAX_PACKET_SIZE-1);
        CPPUNIT_ASSERT_EQUAL(MAX_PACKET_SIZE-1, writer.max_packet_size());

        writer.set_max_packet_size(MAX_PACKET_SIZE+1);
        CPPUNIT_ASSERT_EQUAL(MAX_PACKET_SIZE, writer.max_packet_size());

        writer.set_max_packet_size(MAX_PACKET_SIZE+100);
        CPPUNIT_ASSERT_EQUAL(MAX_PACKET_SIZE, writer.max_packet_size());
    }

    void test_empty_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);
        writer.begin_message();
        writer.end_message();

        Data_reader reader(m_sink.buffer());
        int size = reader.get_int32();
        int seq_num = reader.get_int16();
        bool is_chained = reader.get_int8();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(3, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
    }

    void test_one_byte_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);
        writer.begin_message();
        writer.put_int8(50);
        writer.end_message();

        Data_reader reader(m_sink.buffer());
        int size = reader.get_int32();
        int seq_num = reader.get_int16();
        bool is_chained = reader.get_int8();
        int byte = reader.get_int8();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(4, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
        CPPUNIT_ASSERT_EQUAL(50, byte);
    }

    void test_multibyte_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);
        writer.begin_message();
        writer.put_int8(123);                   // 1
        writer.put_int16(12345);                // 2
        writer.put_int32(1234567890);           // 4
        writer.put_string("Hello, world!");     // 4 + 13
        writer.put_string("abc");               // 4 + 3
        writer.put_string(string("foobar"));    // 4 + 6
        // ------
        // 41
        writer.end_message();

        Data_reader reader(m_sink.buffer());
        int size = reader.get_int32();
        int seq_num = reader.get_int16();
        bool is_chained = reader.get_int8();
        int n08 = reader.get_int8();
        int n16 = reader.get_int16();
        int n32 = reader.get_int32();
        string s1 = reader.get_string();
        string s2 = reader.get_string();
        string s3 = reader.get_string();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(2+1+41, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
        CPPUNIT_ASSERT_EQUAL(123, n08);
        CPPUNIT_ASSERT_EQUAL(12345, n16);
        CPPUNIT_ASSERT_EQUAL(1234567890, n32);
        CPPUNIT_ASSERT_EQUAL(string("Hello, world!"), s1);
        CPPUNIT_ASSERT_EQUAL(string("abc"), s2);
        CPPUNIT_ASSERT_EQUAL(string("foobar"), s3);
    }

    void test_chained_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(100);
        CPPUNIT_ASSERT_EQUAL(100, writer.max_packet_size());

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        Byte buf[100];

        Data_reader reader(m_sink.buffer());
        int size = reader.get_int32();
        int seq_num = reader.get_int16();
        bool is_chained = reader.get_int8();
        int n32 = reader.get_int32();
        reader.read(buf, 89);

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(100-4, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(true, is_chained);
        CPPUNIT_ASSERT_EQUAL(100, n32);
        CPPUNIT_ASSERT_EQUAL(string(89, 'X'), string(buf,buf+89));

        size = reader.get_int32();
        seq_num = reader.get_int16();
        is_chained = reader.get_int8();
        reader.read(buf, 11);

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(2+1+11, size);
        CPPUNIT_ASSERT_EQUAL(1, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
        CPPUNIT_ASSERT_EQUAL(string(11, 'X'), string(buf,buf+11));
    }

    void test_chained_message_1()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(100);
        CPPUNIT_ASSERT_EQUAL(100, writer.max_packet_size());

        writer.begin_message();
        writer.put_string(string(200, 'X'));    // 200 'X's
        writer.end_message();

        Byte buf[100];

        Data_reader reader(m_sink.buffer());
        int size = reader.get_int32();
        int seq_num = reader.get_int16();
        bool is_chained = reader.get_int8();
        int n32 = reader.get_int32();
        reader.read(buf, 89);

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(100-4, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(true, is_chained);
        CPPUNIT_ASSERT_EQUAL(200, n32);
        CPPUNIT_ASSERT_EQUAL(string(89, 'X'), string(buf,buf+89));

        size = reader.get_int32();
        seq_num = reader.get_int16();
        is_chained = reader.get_int8();
        reader.read(buf, 93);

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(100-4, size);
        CPPUNIT_ASSERT_EQUAL(1, seq_num);
        CPPUNIT_ASSERT_EQUAL(true, is_chained);
        CPPUNIT_ASSERT_EQUAL(string(93, 'X'), string(buf,buf+93));

        size = reader.get_int32();
        seq_num = reader.get_int16();
        is_chained = reader.get_int8();
        reader.read(buf, 18);

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(2+1+18, size);
        CPPUNIT_ASSERT_EQUAL(2, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
        CPPUNIT_ASSERT_EQUAL(string(18, 'X'), string(buf,buf+18));
    }

    void test_multiple_messages()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);

        writer.begin_message();
        writer.put_int32(15);
        writer.end_message();

        writer.begin_message();
        writer.put_int32(30);
        writer.end_message();

        Data_reader reader(m_sink.buffer());
        int size = reader.get_int32();
        int seq_num = reader.get_int16();
        bool is_chained = reader.get_int8();
        int n32 = reader.get_int32();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(7, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
        CPPUNIT_ASSERT_EQUAL(15, n32);

        size = reader.get_int32();
        seq_num = reader.get_int16();
        is_chained = reader.get_int8();
        n32 = reader.get_int32();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(7, size);
        CPPUNIT_ASSERT_EQUAL(0, seq_num);
        CPPUNIT_ASSERT_EQUAL(false, is_chained);
        CPPUNIT_ASSERT_EQUAL(30, n32);
    }

    CPPUNIT_TEST_SUITE(Message_writer_tests);
    CPPUNIT_TEST(test_capacity);
    CPPUNIT_TEST(test_empty_message);
    CPPUNIT_TEST(test_one_byte_message);
    CPPUNIT_TEST(test_multibyte_message);
    CPPUNIT_TEST(test_chained_message);
    CPPUNIT_TEST(test_chained_message_1);
    CPPUNIT_TEST(test_multiple_messages);
    CPPUNIT_TEST_SUITE_END();

  private:
    Test_sink m_sink;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Message_writer_tests);
