// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_date_util
#define included_ares_date_util

// This file contains a collection of utilities for date manipulation.

#include <string>

namespace ares {

class Bytes;
class Date;

// Generates a string with the format "9999d 23h 59m 59.999s" from the given
// number of seconds and milliseconds, where d represents days, h hours, m
// minutes, and s seconds and milliseconds.
std::string make_uptime_string(int seconds, int milliseconds = 0);

// Converts a date into a stream of bytes, storing them in bytes. The existing
// contents of bytes are replaced by the byte-stream representation of date.
void to_bytes(Bytes& bytes, Date const& date);

// Reads a byte-stream representation of a date from bytes. This function is
// the inverse of to_bytes. Returns true on success, false if bytes contains
// insufficient data.
bool from_bytes(Date& date, Bytes const& bytes);

} // namespace ares

#endif
