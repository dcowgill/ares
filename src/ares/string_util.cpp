// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/string_util.hpp"
#include "ares/error.hpp"
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <memory>
#include <strings.h>    // for strcasecmp

using namespace std;

namespace
{
// tests if a character is 0-9 or a-f or A-F
inline bool is_hex_digit(char ch)
{
    return (ch >= 'a' && ch <= 'f') ||
            (ch >= 'A' && ch <= 'F') ||
            isdigit(ch);
}
}

// Function object for lowercasing a character.
struct Lower {
    char operator()(char c) {
        return tolower(c);
    }
};

// Function object for uppercasing a character.
struct Upper {
    char operator()(char c) {
        return toupper(c);
    }
};

char* ares::copy_string(char const* s)
{
    if (!s) return 0;
    int size = strlen(s) + 1;
    char* t = new char[size];
    memcpy(t, s, size);
    return t;
}

string ares::chomp(string const& s, char const* rec_sep)
{
    char const* rs  = rec_sep ? rec_sep         : "\n";
    int const rs_len = rec_sep ? strlen(rec_sep) : 1;

    rs += rs_len - 1;
    int i = s.length() - 1;
    while (i >= 0 && s[i] == *rs) i--, rs--;
    return s.substr(0, i + 1);
}

string ares::erase_trailing_chars(string const& s,
                                  char const* char_set)
{
    char const* cs = char_set ? char_set : "\r\n";

    int i = s.length() - 1;
    for (; i >= 0; i--) {
        if (strchr(cs, s[i]) == 0) {
            break;  // s[i] is not in cs
        }
    }
    return s.substr(0, i + 1);
}

int ares::compare_ignore_case(char const* s, char const* t)
{
    // FIXME: check that we have this function in our configure script
    return strcasecmp(s, t);
}

char ares::int_or_float(string const& t)
{
    enum { NOT_NUMERIC = 0, FLOAT = 'F', INT = 'I' };   // for legibility

    char const* s = t.c_str();

    // skip optional leading plus or minus sign
    if (*s == '+' || *s == '-')
        s++;

    // hexadecimal numbers begin with '0x' or '0X'
    if (*s == '0') {
        if (s[1] == 'x' || s[1] == 'X') {
            s += 2;
            while (*s) {
                if (!is_hex_digit(*s++)) {
                    return NOT_NUMERIC;
                }
            }
            return INT;
        }
    }
    else if (!isdigit(*s)) {
        // require one or more digits
        return NOT_NUMERIC;
    }

    s++;

    // require sequence of digits ending with null or decimal point
    while (*s && *s != '.') {
        if (!isdigit(*s))
            return NOT_NUMERIC;
        s++;
    }

    // if EOS, the string is an integer
    if (!*s)
        return INT;

    // require a decimal point
    if (*s != '.')
        return NOT_NUMERIC;

    s++;

    // allow zero or more digits
    while (*s && *s != 'e' && *s != 'E') {
        if (!isdigit(*s))
            return NOT_NUMERIC;
        s++;
    }

    // allow exponent (e.g. "e+42" or "E001" or "E-3")
    if (*s == 'e' || *s == 'E') {
        s++;
        if (*s == '+' || *s == '-')
            s++;
        do {
            if (!isdigit(*s))
                return NOT_NUMERIC;
            s++;
        } while (*s);
    }

    return FLOAT;
}

string ares::dequote(string const& s)
{
    int end = s.length();

    if (s[0] == '\'' && s[end - 1] == '\'') {       // single-quoted string
        string t;

        end--;
        for (int i = 1; i < end; i++) {
            if (s[i] == '\\') {
                if (++i == end) {
                    break;
                }
                if (s[i] == '\'') {
                    t += '\'';
                }
            }
            else {
                t += s[i];
            }
        }

        return t;
    }

    if (s[0] == '"' && s[end - 1] == '"') {         // double-quoted string
        string t;

        end--;
        for (int i = 1; i < end; i++) {
            if (s[i] == '\\') {
                if (++i == end) {
                    break;
                }
                switch (s[i]) {
                    case '"':
                        t += '"';
                        break;
                    case '$':
                        t += '$';
                        break;
                    case '\\':
                        t += '\\';
                        break;
                    case '\0':
                        t += '\0';
                        break;
                    case 'a':
                        t += '\a';
                        break;
                    case 'b':
                        t += '\b';
                        break;
                    case 't':
                        t += '\t';
                        break;
                    case 'n':
                        t += '\n';
                        break;
                    case 'v':
                        t += '\v';
                        break;
                    case 'f':
                        t += '\f';
                        break;
                    case 'r':
                        t += '\r';
                        break;
                    default:
                        t += s[i];  // not a valid escape sequence
                        break;
                }
            }
            else {
                t += s[i];
            }
        }

        return t;
    }

    return s;                                       // not a quoted string
}

string ares::format(char const* fmt, ...)
{
    va_list args;                       // ellided arguments
    int size = 200;                     // initial guess at size
    char* p = new char[size];

    for (;;) {
        va_start(args, fmt);
        int n = vsnprintf(p, size, fmt, args);
        va_end(args);

        if (n >= 0 && n < size)
            break;

        if (n >= 0)
            size = n+1; // we know the exact size
        else
            size *= 2;  // obsolete vsnprintf, have to guess

        delete [] p;
        p = new char[size];
    }

    string s(p);
    delete [] p;
    return s;
}

char* ares::format_string(char const* fmt, ...)
{
    va_list args;                       // ellided arguments
    int size = 200;                     // initial guess at size
    char* p = new char[size];

    for (;;) {
        va_start(args, fmt);
        int n = vsnprintf(p, size, fmt, args);
        va_end(args);

        if (n >= 0 && n < size)
            break;

        if (n >= 0)
            size = n+1; // we know the exact size
        else
            size *= 2;  // obsolete vsnprintf, have to guess

        delete [] p;
        p = new char[size];
    }

    return p;
}
