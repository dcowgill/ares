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
#include "ares/message_reader.hpp"
#include "ares/message_writer.hpp"
#include "unit_test/ares/queue_sink.h"
#include "unit_test/ares/test_sink.h"

using namespace std;
using namespace ares;

// TODO run all tests with really small packet_size in message_writer

namespace
{
// (for readability)
const int MIN_PACKET_SIZE = Message_writer::MIN_PACKET_SIZE;
const int MAX_PACKET_SIZE = Message_writer::MAX_PACKET_SIZE;
}

class Message_reader_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_empty_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);
        writer.begin_message();
        writer.end_message();

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(1, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());
        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(0, msg.size());
    }

    void test_one_byte_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);
        writer.begin_message();
        writer.put_int8(50);
        writer.end_message();

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(1, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());
        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(1, msg.size());

        Data_reader data_reader(msg);
        int byte = data_reader.get_int8();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(50, byte);
    }

    void test_multibyte_message()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MAX_PACKET_SIZE);
        writer.begin_message();
        writer.put_int8(123);                           // 1
        writer.put_int16(12345);                        // 2
        writer.put_int32(1234567890);                   // 4
        writer.put_string("Hello, world!");             // 4 + 13
        writer.put_bytes((Byte const*) "abc", 3);       // 4 + 3
        writer.put_string(string("foobar"));            // 4 + 6
        // ------
        // 41
        writer.end_message();

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(1, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());
        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(41, msg.size());

        Data_reader data_reader(msg);
        int n08 = data_reader.get_int8();
        int n16 = data_reader.get_int16();
        int n32 = data_reader.get_int32();
        string s1 = data_reader.get_string();
        string s2 = data_reader.get_string();
        string s3 = data_reader.get_string();

        CPPUNIT_ASSERT(reader);
        CPPUNIT_ASSERT_EQUAL(123, n08);
        CPPUNIT_ASSERT_EQUAL(12345, n16);
        CPPUNIT_ASSERT_EQUAL(1234567890, n32);
        CPPUNIT_ASSERT_EQUAL(string("Hello, world!"), s1);
        CPPUNIT_ASSERT_EQUAL(string("abc"), s2);
        CPPUNIT_ASSERT_EQUAL(string("foobar"), s3);
    }

    void test_chained_message()
    {
        Byte buf[100];

        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(100);
        CPPUNIT_ASSERT_EQUAL(100, writer.max_packet_size());

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(1, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());

        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(104, msg.size());

        Data_reader data_reader(msg);
        int n32 = data_reader.get_int32();
        data_reader.read(buf, 100);

        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(100, n32);
        CPPUNIT_ASSERT_EQUAL(string(100, 'X'), string(buf,buf+100));
    }

    void test_chained_message_1()
    {
        Byte buf[200];

        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(100);
        CPPUNIT_ASSERT_EQUAL(100, writer.max_packet_size());

        writer.begin_message();
        writer.put_string(string(200, 'X'));    // 200 'X's
        writer.end_message();

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(1, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());

        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(204, msg.size());
        Data_reader data_reader(msg);
        int n32 = data_reader.get_int32();
        data_reader.read(buf, 200);
        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(200, n32);
        CPPUNIT_ASSERT_EQUAL(string(200, 'X'), string(buf,buf+200));
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

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(2, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());

        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(4, msg.size());
        Data_reader data_reader(msg);
        int n32 = data_reader.get_int32();
        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(15, n32);

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(4, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(30, n32);
    }

    void test_multiple_chained_messages()
    {
        Byte buf[400];

        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MIN_PACKET_SIZE);

        writer.begin_message();
        writer.put_string(string(200, 'X'));    // 200 'X's
        writer.end_message();

        writer.begin_message();
        writer.put_string(string(300, 'X'));    // 300 'X's
        writer.end_message();

        writer.begin_message();
        writer.put_string(string(400, 'X'));    // 400 'X's
        writer.end_message();

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);
        int n = reader.read_messages(m_sink.buffer());
        CPPUNIT_ASSERT_EQUAL(3, n);
        CPPUNIT_ASSERT_EQUAL(n, m_queue_sink.size());

        Buffer msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(204, msg.size());
        Data_reader data_reader(msg);
        int n32 = data_reader.get_int32();
        data_reader.read(buf, 200);
        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(200, n32);
        CPPUNIT_ASSERT_EQUAL(string(200, 'X'), string(buf,buf+200));

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(304, msg.size());
        n32 = data_reader.get_int32();
        data_reader.read(buf, 300);
        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(300, n32);
        CPPUNIT_ASSERT_EQUAL(string(300, 'X'), string(buf,buf+300));

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(404, msg.size());
        n32 = data_reader.get_int32();
        data_reader.read(buf, 400);
        CPPUNIT_ASSERT(data_reader);
        CPPUNIT_ASSERT_EQUAL(400, n32);
        CPPUNIT_ASSERT_EQUAL(string(400, 'X'), string(buf,buf+400));
    }

    void test_overflow()
    {
        m_sink.reset();
        Message_writer writer(m_sink);

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(100);
        try {
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }
    }

    void test_overflow_1()
    {
        m_sink.reset();
        Message_writer writer(m_sink);

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(103);
        try {
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(104);
        try {
            reader.read_messages(m_sink.buffer());
        }
        catch (Message_size_exceeded_error&) {
            CPPUNIT_ASSERT(false);
        }

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(105);
        try {
            reader.read_messages(m_sink.buffer());
        }
        catch (Message_size_exceeded_error&) {
            CPPUNIT_ASSERT(false);
        }
    }

    void test_overflow_1_with_min_packet_size()
    {
        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MIN_PACKET_SIZE);

        m_queue_sink.reset();
        Message_reader reader(m_queue_sink);

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(103);
        try {
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(104);
        try {
            reader.read_messages(m_sink.buffer());
        }
        catch (Message_size_exceeded_error&) {
            CPPUNIT_ASSERT(false);
        }

        writer.begin_message();
        writer.put_string(string(100, 'X'));    // 100 'X's
        writer.end_message();

        reader.set_max_message_size(105);
        try {
            reader.read_messages(m_sink.buffer());
        }
        catch (Message_size_exceeded_error&) {
            CPPUNIT_ASSERT(false);
        }
    }

    void test_overflow_2()
    {
        Buffer msg;
        Data_reader data_reader(msg);
        Byte buf[400];
        int n32;

        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(50);

        Message_reader reader(m_queue_sink);
        reader.set_max_message_size(150);

        writer.begin_message();
        writer.put_string(string(100, 'X'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(200, 'Y'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(101, 'Z'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(102, 'A'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(200, 'B'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(103, 'C'));
        writer.end_message();

        try {
            m_queue_sink.reset();
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }

        CPPUNIT_ASSERT_EQUAL(1, m_queue_sink.size());

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(104, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(100, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'X'), string(buf,buf+n32));

        try {
            m_queue_sink.reset();
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }

        CPPUNIT_ASSERT_EQUAL(2, m_queue_sink.size());

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(105, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(101, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'Z'), string(buf,buf+n32));

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(106, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(102, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'A'), string(buf,buf+n32));

        try {
            m_queue_sink.reset();
            reader.read_messages(m_sink.buffer());
        }
        catch (Message_size_exceeded_error&) {
            CPPUNIT_ASSERT(false);
        }

        CPPUNIT_ASSERT_EQUAL(1, m_queue_sink.size());

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(107, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(103, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'C'), string(buf,buf+n32));
    }

    void test_overflow_2_with_min_packet_size()
    {
        Buffer msg;
        Data_reader data_reader(msg);
        Byte buf[400];
        int n32;

        m_sink.reset();
        Message_writer writer(m_sink);
        writer.set_max_packet_size(MIN_PACKET_SIZE);

        Message_reader reader(m_queue_sink);
        reader.set_max_message_size(150);

        writer.begin_message();
        writer.put_string(string(100, 'X'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(200, 'Y'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(101, 'Z'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(102, 'A'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(200, 'B'));
        writer.end_message();
        writer.begin_message();
        writer.put_string(string(103, 'C'));
        writer.end_message();

        try {
            m_queue_sink.reset();
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }

        CPPUNIT_ASSERT_EQUAL(1, m_queue_sink.size());

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(104, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(100, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'X'), string(buf,buf+n32));

        try {
            m_queue_sink.reset();
            reader.read_messages(m_sink.buffer());
            CPPUNIT_ASSERT(false);
        }
        catch (Message_size_exceeded_error&) {
            // OK
        }

        CPPUNIT_ASSERT_EQUAL(2, m_queue_sink.size());

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(105, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(101, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'Z'), string(buf,buf+n32));

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(106, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(102, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'A'), string(buf,buf+n32));

        try {
            m_queue_sink.reset();
            reader.read_messages(m_sink.buffer());
        }
        catch (Message_size_exceeded_error&) {
            CPPUNIT_ASSERT(false);
        }

        CPPUNIT_ASSERT_EQUAL(1, m_queue_sink.size());

        msg = m_queue_sink.dequeue();
        CPPUNIT_ASSERT_EQUAL(107, msg.size());
        n32 = data_reader.get_int32();
        CPPUNIT_ASSERT_EQUAL(103, n32);
        data_reader.read(buf, n32);
        CPPUNIT_ASSERT_EQUAL(string(n32, 'C'), string(buf,buf+n32));
    }

    CPPUNIT_TEST_SUITE(Message_reader_tests);
    CPPUNIT_TEST(test_empty_message);
    CPPUNIT_TEST(test_one_byte_message);
    CPPUNIT_TEST(test_multibyte_message);
    CPPUNIT_TEST(test_chained_message);
    CPPUNIT_TEST(test_chained_message_1);
    CPPUNIT_TEST(test_multiple_messages);
    CPPUNIT_TEST(test_multiple_chained_messages);
    CPPUNIT_TEST(test_overflow);
    CPPUNIT_TEST(test_overflow_1);
    CPPUNIT_TEST(test_overflow_1_with_min_packet_size);
    CPPUNIT_TEST(test_overflow_2);
    CPPUNIT_TEST(test_overflow_2_with_min_packet_size);
    CPPUNIT_TEST_SUITE_END();

  private:
    Queue_sink m_queue_sink;
    Test_sink m_sink;
};

CPPUNIT_TEST_SUITE_REGISTRATION(Message_reader_tests);
