// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ares/bin_util.hpp"

using namespace std;
using namespace ares;

#define nelems(a) int(sizeof(a)/sizeof((a)[0]))

class Bin_util_tests : public CppUnit::TestFixture {
  public:
    void setUp() {}

    void tearDown() {}

    void test_unpack_int16()
    {
        Byte buf[2];
        Int16 n = -32768;
        for (;;) {
            buf[0] = buf[1] = 0;
            pack_int16(buf, n);
            CPPUNIT_ASSERT_EQUAL(n, unpack_int16(buf));
            if (n++ == 32767) break;
        }
    }

    void test_unpack_uint16()
    {
        Byte buf[2];
        Uint16 n = 0;
        for (;;) {
            buf[0] = buf[1] = 0;
            pack_uint16(buf, n);
            CPPUNIT_ASSERT_EQUAL(n, unpack_uint16(buf));
            if (n++ == 65535) break;
        }
    }

    void test_pack_int32()
    {
        const Int32 test_values[] = {
            -2147483648,
            -2147483647,
            -2139062144,
            -2130706175,
            -1000000000,
            -65536,
            -32768,
            -1,
            0,
            1,
            32767,
            65535,
            8421504,
            16777473,
            16843009,
            1000000000,
            2147483646,
            2147483647,
        };
        const int test_bytes[][4] = {
            { 128,   0,   0,   0 },
            { 128,   0,   0,   1 },
            { 128, 128, 128, 128 },
            { 129,   0,   1,   1 },
            { 196, 101,  54,   0 },
            { 255, 255,   0,   0 },
            { 255, 255, 128,   0 },
            { 255, 255, 255, 255 },
            {   0,   0,   0,   0 },
            {   0,   0,   0,   1 },
            {   0,   0, 127, 255 },
            {   0,   0, 255, 255 },
            {   0, 128, 128, 128 },
            {   1,   0,   1,   1 },
            {   1,   1,   1,   1 },
            {  59, 154, 202,   0 },
            { 127, 255, 255, 254 },
            { 127, 255, 255, 255 },
        };


        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0};
            pack_int32(buf, test_values[i]);
            for (int j = 0; j < 4; j++) {
                //printf("DEBUG: i=%d, j=%d\n", i, j);
                CPPUNIT_ASSERT_EQUAL(test_bytes[i][j], int(buf[j]));
            }
        }
    }

    void test_pack_uint32()
    {
        const Uint32 test_values[] = {
            0,
            1,
            32767,
            65535,
            8421504,
            16777473,
            16843009,
            1000000000,
            2147483646,
            2147483647,
        };
        const int test_bytes[][4] = {
            {   0,   0,   0,   0 },
            {   0,   0,   0,   1 },
            {   0,   0, 127, 255 },
            {   0,   0, 255, 255 },
            {   0, 128, 128, 128 },
            {   1,   0,   1,   1 },
            {   1,   1,   1,   1 },
            {  59, 154, 202,   0 },
            { 127, 255, 255, 254 },
            { 127, 255, 255, 255 },
        };


        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0};
            pack_uint32(buf, test_values[i]);
            for (int j = 0; j < 4; j++) {
                //printf("DEBUG: i=%d, j=%d\n", i, j);
                CPPUNIT_ASSERT_EQUAL(test_bytes[i][j], int(buf[j]));
            }
        }
    }

    void test_unpack_int32()
    {
        const Int32 test_values[] = {
            -2147483648,
            -2147483647,
            -2139062144,
            -2130706175,
            -1000000000,
            -65536,
            -32768,
            -1,
            0,
            1,
            32767,
            65535,
            8421504,
            16777473,
            16843009,
            1000000000,
            2147483646,
            2147483647,
        };

        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0};
            pack_int32(buf, test_values[i]);
            CPPUNIT_ASSERT_EQUAL(test_values[i], unpack_int32(buf));
        }
    }

    void test_unpack_uint32()
    {
        const Uint32 test_values[] = {
            0,
            1,
            32767,
            65535,
            8421504,
            16777473,
            16843009,
            1000000000,
            2147483646,
            2147483647,
            2164261121,
            4294967294,
            4294967295,
        };

        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0};
            pack_uint32(buf, test_values[i]);
            CPPUNIT_ASSERT_EQUAL(test_values[i], unpack_uint32(buf));
        }
    }

    void test_compress_uint32()
    {
        const Uint32 test_values[][2] = {
            {          0, 1 },
            {          1, 1 },
            {        200, 1 },
            {        253, 1 },
            {        254, 3 },
            {        255, 3 },
            {      32767, 3 },
            {      65533, 3 },
            {      65534, 3 },
            {      65535, 5 },
            {      65536, 5 },
            { 2147483646, 5 },
            { 2147483647, 5 },
        };
        const int test_bytes[][5] = {
            {   0,   0,   0,   0,   0 },
            {   1,   0,   0,   0,   0 },
            { 200,   0,   0,   0,   0 },
            { 253,   0,   0,   0,   0 },
            { 254,   0, 254,   0,   0 },
            { 254,   0, 255,   0,   0 },
            { 254, 127, 255,   0,   0 },
            { 254, 255, 253,   0,   0 },
            { 254, 255, 254,   0,   0 },
            { 255,   0,   0, 255, 255 },
            { 255,   0,   1,   0,   0 },
            { 255, 127, 255, 255, 254 },
            { 255, 127, 255, 255, 255 },
        };

        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0,0};
            int n = compress_uint32(buf, test_values[i][0]);
            CPPUNIT_ASSERT_EQUAL(int(test_values[i][1]), n);
            for (int j = 0; j < 5; j++) {
                //printf("DEBUG: i=%d, j=%d\n", i, j);
                CPPUNIT_ASSERT_EQUAL(test_bytes[i][j], int(buf[j]));
            }
        }
    }

    void test_decompress_uint32_0()
    {
        const Uint32 test_values[][2] = {
            {          0, 1 },
            {          1, 1 },
            {        200, 1 },
            {        253, 1 },
            {        254, 3 },
            {        255, 3 },
            {      32767, 3 },
            {      65533, 3 },
            {      65534, 3 },
            {      65535, 5 },
            {      65536, 5 },
            { 2147483646, 5 },
            { 2147483647, 5 },
        };

        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0,0};
            compress_uint32(buf, test_values[i][0]);
            Uint32 value;
            int n = decompress_uint32(buf, value);
            CPPUNIT_ASSERT_EQUAL(int(test_values[i][1]), n);
            CPPUNIT_ASSERT_EQUAL(test_values[i][0], value);
        }
    }

    void test_decompress_uint32_1()
    {
        const Uint32 test_values[][2] = {
            {          0, 1 },
            {          1, 1 },
            {        200, 1 },
            {        253, 1 },
            {        254, 3 },
            {        255, 3 },
            {      32767, 3 },
            {      65533, 3 },
            {      65534, 3 },
            {      65535, 5 },
            {      65536, 5 },
            { 2147483646, 5 },
            { 2147483647, 5 },
        };

        for (int i = 0; i < nelems(test_values); i++) {
            Byte buf[] = {0,0,0,0,0};
            compress_uint32(buf, test_values[i][0]);
            Uint32 value;
            for (int j = 0; j < int(test_values[i][1]); j++) {
                int n = decompress_uint32(buf, j, value);
                CPPUNIT_ASSERT_EQUAL(0, n);
            }
            int n = decompress_uint32(buf, test_values[i][1], value);
            CPPUNIT_ASSERT_EQUAL(int(test_values[i][1]), n);
            CPPUNIT_ASSERT_EQUAL(test_values[i][0], value);
        }
    }

    CPPUNIT_TEST_SUITE(Bin_util_tests);
    CPPUNIT_TEST(test_unpack_int16);
    CPPUNIT_TEST(test_unpack_uint16);
    CPPUNIT_TEST(test_pack_int32);
    CPPUNIT_TEST(test_pack_uint32);
    CPPUNIT_TEST(test_unpack_int32);
    CPPUNIT_TEST(test_unpack_uint32);
    CPPUNIT_TEST(test_compress_uint32);
    CPPUNIT_TEST(test_decompress_uint32_0);
    CPPUNIT_TEST(test_decompress_uint32_1);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Bin_util_tests);
