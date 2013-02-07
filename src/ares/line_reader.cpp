// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/line_reader.hpp"
#include "ares/error.hpp"
#include "ares/buffer.hpp"
#include <algorithm>

using namespace std;

namespace
{
// Returns a pointer to the first newline character in the range (begin, end].
// If no newline is found, returns end. Both '\r' and '\n' are accepted as
// newline characters.
ares::Byte const* find_newline(ares::Byte const* begin,
                               ares::Byte const* end)
{
    while (begin < end) {
        if (*begin == '\r' || *begin == '\n')
            return begin;
        begin++;
    }
    return end;
}
}

ares::Line_reader::Line_reader(Buffer& b)
        : Buffer_formatter(b)
        , m_discard(false)
{}

ares::Line_reader::~Line_reader()
{}

bool ares::Line_reader::get_line(string& s)
{
    // Abort the operation if we're in the failed state.
    if (!*this)
        return false;

    Byte const* newline;  // position of newline in input stream

    // Search for a newline in the input stream; if not found, return.

    newline = find_newline(buffer().begin(), buffer().end());
    if (newline == buffer().end()) {
        fail();
        return false;  // failed
    }

    // We found a newline in the read buffer. Copy the line (including the
    // terminating newline characters) into the destination string and skip
    // the appropriate number of characters in the read buffer.

    int length = newline - buffer().begin();
    int newline_length = 1;

    // If we are not at the end of the input and the current character is the
    // '\r', then the next character in the stream could be a '\n'. If it is,
    // skip past it and add it to the read string.

    if (newline[0] == '\r' && newline != buffer().end() - 1) {
        if (newline[1] == '\n') {
            newline_length++;
        }
    }

    const char* str = (char const*) buffer().begin();
    s.assign(str, length + (m_discard ? 0 : newline_length));
    buffer().consume(length + newline_length);
    return true;
}
