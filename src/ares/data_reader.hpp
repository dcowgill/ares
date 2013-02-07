// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_data_reader
#define included_ares_data_reader

#include "ares/basic_reader.hpp"
#include "ares/bytes.hpp"
#include "ares/types.hpp"
#include <string>

namespace ares {

// A buffer formatter that reads primitive binary objects. All fixed-width
// integer types are read in high (most significant) to low (least
// significant) byte order (also called "network byte order"), regardless of
// the endian-ness of the platform. Also see Data_writer.
class Data_reader : public Basic_reader {
  public:
    // Constructs a data reader.
    Data_reader(Buffer& b);

    // Destructor.
    virtual ~Data_reader();

    // Reads an 8-bit integer from the buffer. Returns 255 on error.
    Int8 get_int8();

    // Reads a 16-bit integer from the buffer. Returns 32767 on error.
    Int16 get_int16();

    // Reads a 32-bit integer from the buffer. Returns 2147483647 on error.
    Int32 get_int32();

    // Works like Data_reader::get_int8, except the buffer is not modified.
    Int8 peek_int8();

    // Works like Data_reader::get_int16, except the buffer is not modified.
    Int16 peek_int16();

    // Works like Data_reader::get_int32, except the buffer is not modified.
    Int32 peek_int32();

    // Reads a string from the buffer, storing it in s. In this context,
    // strings are not assumed to contain text. Instead, they are simply
    // treated as sequences of bytes. When written to the buffer, strings are
    // preceeded with a 32-bit integer that contains the length of the string.
    // Warning: the max_length argument is not yet implemented!
    bool get_string(std::string& s, int max_length = 0);

    // Works like Data_reader::get_string(std::string&,int), except the string
    // is returned directly instead of in a reference parameter. On error, the
    // returned string will be empty.
    std::string get_string();

    // Reads an array of bytes from the buffer, storing it in "bytes". When
    // written to the buffer, the byte array is preceeded with a 32-bit
    // integer that contains the length of the array.
    bool get_bytes(Bytes& bytes);

  private:
    bool can_read(int n);
};

} // namespace ares

#endif
