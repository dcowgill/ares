// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/date_util.hpp"
#include "ares/bin_util.hpp"
#include "ares/bytes.hpp"
#include "ares/date.hpp"
#include <cassert>
#include <cstdio>

using namespace std;

string ares::make_uptime_string(int seconds, int milliseconds)
{
    // Compute days, hours, minutes, seconds.
    int d = seconds / (24*60*60);
    seconds -= d*24*60*60;
    int h = seconds / (60*60);
    seconds -= h*60*60;
    int m = seconds / 60;
    seconds -= m*60;

    // 9999d 23h 59m 59.999s    <-- default format string
    // 123456789012345678901    <-- length in bytes

    char buf[22];
    snprintf(buf, sizeof(buf), "%4dd %2dh %2dm %2d.%03ds", d, h, m,
             seconds, milliseconds);
    return buf;
}

void ares::to_bytes(Bytes& bytes, Date const& date)
{
    bytes.set_min_capacity_no_copy(7);
    bytes.set_size(7);
    pack_int16(bytes.buf(), Int16(date.year()));
    bytes.buf()[2] = Int8(date.month());
    bytes.buf()[3] = Int8(date.day());
    bytes.buf()[4] = Int8(date.hour());
    bytes.buf()[5] = Int8(date.minutes());
    bytes.buf()[6] = Int8(date.seconds());
}

bool ares::from_bytes(Date& date, Bytes const& bytes)
{
    if (bytes.size() < 7)
        return false;
    date.set(unpack_int16(bytes.begin()),   // year
             bytes.begin()[2],              // mon
             bytes.begin()[3],              // day
             bytes.begin()[4],              // hour
             bytes.begin()[5],              // min
             bytes.begin()[6]);             // sec
    return true;
}
