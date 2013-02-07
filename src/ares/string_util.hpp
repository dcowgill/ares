// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_string_util
#define included_ares_string_util

#include "ares/types.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <cstdlib>
#include <string>

namespace ares {

// Duplicates a C-style string using operator new[].
char* copy_string(char const* s);

// Removes a trailing string from another string. For example, chomp(s,"abc")
// will remove "abc" exactly once from the end of s, if s ends in "abc".
std::string chomp(std::string const& s, char const* rec_sep = 0);

// Removes from the end of s any character in appearing in char_set. If
// char_set is not given, it defaults to "\r\n".
std::string erase_trailing_chars(std::string const& s, char const* char_set=0);

// Performs a case-insensitive lexicographic comparison of two null-terminated
// strings. Returns an integer less than, equal to, or greater than zero if s
// is less than, equal to, or greater than t.
int compare_ignore_case(char const* s, char const* t);

// A case-insensitive string comparator (computes less-than). This function
// object can be used wherever case-insensitive string comparison is needed.
// It is also suitable for use with STL associative containers.
struct Compare_string_ignore_case {
    inline bool operator()(std::string const& a, std::string const& b) const {
        return compare_ignore_case(a.c_str(), b.c_str()) < 0;
    }
};

// A hash functor for std::string, suitable for use with the non-standard
// unique associative hash-table containers (e.g., std::hash_map).
//
// Note: This hash function was created by Robert J. Jenkins Jr. See
// http://burtleburtle.net/bob/hash/evahash.html for more information.
struct Hash_string {
    inline ares::Uint32 operator()(std::string const& s) const
    {
        typedef ares::Uint32 u32;  // for readability

#define mix(a, b, c) {                          \
            a -= b; a -= c; a ^= (c>>13);       \
            b -= c; b -= a; b ^= (a<< 8);       \
            c -= a; c -= b; c ^= (b>>13);       \
            a -= b; a -= c; a ^= (c>>12);       \
            b -= c; b -= a; b ^= (a<<16);       \
            c -= a; c -= b; c ^= (b>> 5);       \
            a -= b; a -= c; a ^= (c>> 3);       \
            b -= c; b -= a; b ^= (a<<10);       \
            c -= a; c -= b; c ^= (b>>15);       \
        }

        unsigned char const* k = (unsigned char const*) s.data();
        u32 a, b, c, len;

        len = s.length();
        a = b = 0x9e3779b9;  // the golden ratio; an arbitrary value
        c = 0;

        // Handle most of the key:
        while (len >= 12) {
            a += k[0] + u32(k[1]<<8) + u32(k[ 2]<<16) + u32(k[ 3]<<24);
            b += k[4] + u32(k[5]<<8) + u32(k[ 6]<<16) + u32(k[ 7]<<24);
            c += k[8] + u32(k[9]<<8) + u32(k[10]<<16) + u32(k[11]<<24);
            mix(a, b, c);
            k += 12; len -= 12;
        }

        // Handle the last 11 bytes:
        c += len;
        switch (len) {
            case 11: c += u32(k[10]<<24);
            case 10: c += u32(k[9]<<16);
            case  9: c += u32(k[8]<<8);
            case  8: b += u32(k[7]<<24);
            case  7: b += u32(k[6]<<16);
            case  6: b += u32(k[5]<<8);
            case  5: b += k[4];
            case  4: a += u32(k[3]<<24);
            case  3: a += u32(k[2]<<16);
            case  2: a += u32(k[1]<<8);
            case  1: a += k[0];
        }

        mix(a, b, c);
        return c;

#undef mix
    }
};

// Determines whether a string contains a textual representation of an integer
// or a floating-point number or neither. Returns char('I') if the string
// contains an integer, char('F') if it contains a floating-point number, or
// zero ('\0') if it contains neither. Numbers may be preceded by a single
// plus or minus sign, and floating-point numbers may contain a single decimal
// point and may end with an exponent (e.g. 'e+37' or 'E-001').
//
// Note: This function doesn't care about the size of the number it parses.
// For example, it may return 'I' even if the integer it finds wouldn't fit in
// any native machine integer type.
char int_or_float(std::string const& t);

// If a given string begins and ends with either single or double quotes,
// converts the escape sequences in the string to ASCII characters, removes
// the surrounding quote marks, and returns the new string. If the string
// doesn't begin and end with the same quote marks, returns the original.
//
// If the first character is a single quote, the string is considered
// single-quoted, and only the escaped single quote (\') is unescaped.
// Otherwise, the string is considered double-quoted, and the following
// conversions take place:
//
//      \"   ->  "
//      \$   ->  $
//      \\\\ ->  \\ (ASCII 92)
//      \0   ->  null (ASCII 0)
//      \a   ->  bell (ASCII 7)
//      \b   ->  backspace (ASCII 8)
//      \t   ->  tab (ASCII 9)
//      \n   ->  newline (ASCII 10)
//      \v   ->  vertical tab (ASCII 11)
//      \f   ->  form feed (ASCII 12)
//      \r   ->  carriage return (ASCII 13)
std::string dequote(std::string const& s);

// Similar to printf, this function returns a std::string object instead of a
// character pointer. This is basically a better snprintf in many situations.
std::string format(char const* fmt, ...);

// Similar to printf, except the result is returned in a dynamically allocated
// C-style string. The caller assumes responsibility for the returned pointer.
char* format_string(char const* fmt, ...);

} // namespace ares

#endif
