// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_line_reader
#define included_ares_line_reader

#include "ares/buffer_formatter.hpp"
#include <string>

namespace ares {

// A buffer formatter that reads sequences of newline-terminated strings. This
// object considers a newline to consist of any of the following byte
// sequences: "\r" (ASCII 13), "\n" (ASCII 10), or "\r\n" (ASCII 13 followed
// by ASCII 10).
class Line_reader : public Buffer_formatter {
  public:
    // Constructs a line reader.
    Line_reader(Buffer& b);

    // Destructor.
    virtual ~Line_reader();

    // Reads a line from from the wrapped buffer, storing it in s. If a line
    // could not be read, the wrapped buffer is not modified.
    virtual bool get_line(std::string& s);

    // Specifies whether the terminating newline characters should be stored
    // in the string returned by Line_reader::get_line. By default, the
    // newline characters are preserved.
    void set_discard_newline(bool b) { m_discard = b; }

  private:
    bool m_discard;  // should newlines be discarded?
};

} // namespace ares

#endif
