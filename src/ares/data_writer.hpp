// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_data_writer
#define included_ares_data_writer

#include "ares/buffer_formatter.hpp"
#include "ares/bytes.hpp"
#include "ares/types.hpp"
#include <cstring>
#include <string>

namespace ares {

// A buffer formatter that writes primitive binary objects. All fixed-width
// integer types are written in high-to-low byte order (most to least
// significant), also called "network byte order," regardless of the
// endian-ness of the platform. Also see Data_reader.
class Data_writer : public Buffer_formatter {
  public:
    // Constructs a data writer.
    Data_writer(Buffer& b);

    // Destructor.
    virtual ~Data_writer();

    // Writes an 8-bit integer to the buffer.
    bool put_int8(Int8 n);

    // Writes a 16-bit integer to the buffer.
    bool put_int16(Int16 n);

    // Writes a 32-bit integer to the buffer.
    bool put_int32(Int32 n);

    // Writes a null-terminated string to the buffer. This function is
    // semantically equivalent to put_string(s,strlen(s)).
    bool put_string(const char* s);

    // Writes a string to the buffer. This function is semantically equivalent
    // to put_string(s.data(),s.length()).
    bool put_string(const std::string& s);

    // Writes the value of "len" to the buffer exactly as
    // Data_writer::put_int32 would, then writes "len" bytes from "data" to
    // the buffer.
    bool put_bytes(Byte const* data, Int32 len);

    // Writes a byte array to the buffer. This function is semantically
    // equivalent to put_string(bytes.data(),bytes.length()).
    bool put_bytes(Bytes const& bytes);

  private:
    bool write(Byte const* data, int count);
};

// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

inline bool Data_writer::put_string(char const* s)
{
    return put_bytes((Byte const*) s, std::strlen(s));
}

inline bool Data_writer::put_string(std::string const& s)
{
    return put_bytes((Byte const*) s.data(), s.length());
}

inline bool Data_writer::put_bytes(Bytes const& bytes)
{
    return put_bytes(bytes.begin(), bytes.size());
}

} // namespace ares

#endif
