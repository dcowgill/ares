// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/exception.hpp"
#include "ares/string_tokenizer.hpp"
#include "ares/string_util.hpp"
#include "ares/utility.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

using namespace std;
using ares::Exception;

namespace
{
int const TYPE_INT = int('d');
int const TYPE_DBL = int('g');
int const TYPE_STR = int('s');
int const TYPE_PTR = int('p');

// Converts an error code to a string.
string code_to_string(int code)
{
    char buf[50];
    snprintf(buf, sizeof(buf), "%05d", code);
    return buf;
}

// Tests if a character is a valid printf conversion specifier.
bool is_conversion_specifier(char ch)
{
    switch (ch) {
        case 'c': case 'd': case 'i':
        case 'o': case 'u': case 'x': case 'X':
        case 'e': case 'E': case 'f': case 'F':
        case 'g': case 'G': case 'a': case 'A':
        case 's': case 'p':
            return true;
    }
    return false;
}

// Parses a directive such as "%d" or "%123$s"; if there is a position
// specifier (e.g. "123$"), it is extracted and its integer value is
// returned; returns -1 if there is no position specifier
int parse_directive(char* out, int* out_len, int max_out_len,
                    char const* dir, int len)
{
    assert(out != 0 && out_len != 0 && dir != 0);
    assert(0 < len && len <= max_out_len);
    assert(dir[0] == '%');

    // Parse the optional position specifier (e.g. the "123" in "%123$d").
    char pos_str[4];
    int i;
    for (i = 0; i < min(4, len-1) && isdigit(dir[i + 1]); i++)
        pos_str[i] = dir[i + 1];

    // If there was no position specifier, return the entire directive.
    if (i == 0 || dir[i + 1] != '$') {
        memcpy(out, dir, len);
        out[len] = '\0';
        *out_len = len;
        return -1;
    }

    // Extract the remainder of the directive into 'out'.
    out[0] = '%';
    char* s = out + 1;
    for (int j = i + 2; j < len; j++)
        *s++ = dir[j];
    *s = '\0';
    *out_len = s - out;

    // Convert the digits in pos_str to an integer and return it.
    int pos = 0;
    for (int power = 1; i != 0; power *= 10, i--)
        pos += power * (pos_str[i-1] - '0');

    return pos;
}

// Formats an exception a given format string, appending to string 'out'.
void format_message(string& out, Exception const& e, char const* msg)
{
    char buf[1000];             // for converting arguments
    char fmt[20];               // storage for conversion specifiers
    int fmtlen;                 // length of string stored in 'fmt'

    while (msg[0] != '\0') {
        char* next = strchr(msg, '%');  // search for the next '%'
        if (!next) {
            out += msg;                 // no more conversion specifiers
            return;
        }

        out.append(msg, next - msg);
        msg = next;

        if (msg[1] == '%') {
            out += '%';                 // "%%" translates to literal '%'
            msg++;
            continue;
        }

        // Find the conversion specifier character.
        int end;
        for (end = 1; !is_conversion_specifier(msg[end]); end++) {
            if (msg[end] == '\0') {
                out += msg;
                return;
            }
        }
        end++;

        // Insure that the format string doesn't exceed the max length.
        if (end >= int(sizeof(fmt))) {
            out += msg;
            return;
        }

        // Copy the format into 'fmt', excluding the position specifier.
        int arg_num = parse_directive(fmt, &fmtlen, sizeof(fmt),
                                      msg, end) - 1;
        msg += end;

        // If we've run out of arguments, skip the conversion.
        if (arg_num < 0 || arg_num >= e.num_arguments())
            continue;

        // Convert based on the conversion specifier.
        switch (fmt[fmtlen - 1]) {
            case 'c': case 'd': case 'i':
            case 'o': case 'u': case 'x': case 'X':
                snprintf(buf, sizeof(buf), fmt, e.arg_int_val(arg_num));
                break;

            case 'e': case 'E': case 'f': case 'F':
            case 'g': case 'G': case 'a': case 'A':
                snprintf(buf, sizeof(buf), fmt, e.arg_dbl_val(arg_num));
                break;

            case 's': {
                string s = boost::trim_copy(e.arg_str_val(arg_num));
                snprintf(buf, sizeof(buf), fmt, s.c_str());
                break;
            }

            case 'p':
                snprintf(buf, sizeof(buf), fmt, e.arg_ptr_val(arg_num));
                break;

            default:
                ARES_PANIC(("invalid specifier '%c'", fmt[fmtlen - 1]));
        }

        out += buf;
    }
}
}

Exception::Exception(char const* module, int code)
        : m_module(copy_string(module))
        , m_code(code)
        , m_message(0)
        , m_args(0)
        , m_num_args(0)
        , m_next(0)
{}

Exception::Exception(char const* module, int code, char const* arg_types, ...)
        : m_module(copy_string(module))
        , m_code(code)
        , m_message(0)
        , m_args(0)
        , m_num_args(0)
        , m_next(0)
{
    ARES_SET_EXCEPTION_VARARGS;
}

Exception::Exception(Exception const& e)
        : m_module(copy_string(e.m_module))
        , m_code(e.m_code)
        , m_message(copy_string(e.m_message ? e.m_message : e.message()))
        , m_args(0)
        , m_num_args(0)
        , m_next(e.m_next ? new /*(nothrow)*/ Exception(*e.m_next) :  0) // FIXME re-enable nothrow
{
    set_arguments(e.m_args, e.m_num_args);
}

Exception::~Exception()
{
    delete [] m_module;
    m_module = 0;
    delete [] m_message;
    m_message = 0;
    unset_arguments();
    delete m_next;
    m_next = 0;
}

void Exception::set_cause(Exception const& e)
{
    delete m_next;
    m_next = new /*(nothrow)*/ Exception(e); // FIXME re-enable nothrow
}

char const* Exception::module() const
{
    return m_module;
}

int Exception::code() const
{
    return m_code;
}

int Exception::argument_type(int n) const
{
    assert(n >= 0 && n < m_num_args);
    return m_args[n].first;
}

int Exception::arg_int_val(int n) const
{
    assert(n >= 0 && n < m_num_args);
    return m_args[n].second.int_val;    // implicit conversion
}

double Exception::arg_dbl_val(int n) const
{
    assert(n >= 0 && n < m_num_args);
    return m_args[n].second.dbl_val;    // implicit conversion
}

string Exception::arg_str_val(int n) const
{
    assert(n >= 0 && n < m_num_args);
    if (m_args[n].first == TYPE_STR)
        return m_args[n].second.str_val;  // TODO: escape meta-characters

    // The following types require manual conversion.

    char buf[50];
    if (m_args[n].first == TYPE_INT)
        snprintf(buf, sizeof(buf), "%d", m_args[n].second.int_val);
    else if (m_args[n].first == TYPE_DBL)
        snprintf(buf, sizeof(buf), "%g", m_args[n].second.dbl_val);
    else if (m_args[n].first == TYPE_PTR)
        snprintf(buf, sizeof(buf), "%p", m_args[n].second.ptr_val);
    return buf;
}

void const* Exception::arg_ptr_val(int n) const
{
    assert(n >= 0 && n < m_num_args);
    return m_args[n].second.ptr_val;    // implicit conversion
}

int Exception::num_arguments() const
{
    return m_num_args;
}

Exception const& Exception::next_exception() const
{
    assert(has_next_exception());
    return *m_next;
}

bool Exception::has_next_exception() const
{
    return m_next != 0;
}

string Exception::to_string() const
{
    string s(module());
    s += '-';
    s += code_to_string(code());
    s += ": ";

    format_message(s, *this, m_message ? m_message : message());

    if (has_next_exception()) {
        s += " (";
        s += next_exception().to_string();
        s += ')';
    }

    return s;
}

char const* Exception::message() const
{
    return "error while retrieving text for error";
}

void Exception::set_arguments(char const* arg_types, ...)
{
    ARES_SET_EXCEPTION_VARARGS;
}

void Exception::set_arguments(char const* arg_types, va_list args)
{
    assert(m_num_args == 0 && m_args == 0);
    m_num_args = strlen(arg_types);
    m_args = new /*(nothrow)*/ Arg[m_num_args]; // FIXME re-enable nothrow
    if (!m_args) {
        m_num_args = 0;
        return;
    }
    for (int i = 0; i < m_num_args; i++) {
        m_args[i].first = arg_types[i];
        switch (arg_types[i]) {
            case TYPE_INT:
                m_args[i].second.int_val = va_arg(args, int);
                break;
            case TYPE_DBL:
                m_args[i].second.dbl_val = va_arg(args, double);
                break;
            case TYPE_STR:
                m_args[i].second.str_val = copy_string(va_arg(args, char const*));
                break;
            case TYPE_PTR:
                m_args[i].second.ptr_val = va_arg(args, void const*);
                break;
            default:
                ARES_PANIC(("'%c' is not a valid arg-type", arg_types[i]));
        }
    }
}

void Exception::set_arguments(Arg* args, int num_args)
{
    assert(m_num_args == 0 && m_args == 0);
    m_num_args = num_args;
    m_args = new /*(nothrow)*/ Arg[m_num_args]; // FIXME re-enable nothrow
    if (!m_args) {
        m_num_args = 0;
        return;
    }
    for (int i = 0; i < m_num_args; i++) {
        m_args[i].first = args[i].first;
        switch (args[i].first) {
            case TYPE_INT:
                m_args[i].second.int_val = args[i].second.int_val;
                break;
            case TYPE_DBL:
                m_args[i].second.dbl_val = args[i].second.dbl_val;
                break;
            case TYPE_STR:
                m_args[i].second.str_val = copy_string(args[i].second.str_val);
                break;
            case TYPE_PTR:
                m_args[i].second.ptr_val = args[i].second.ptr_val;
                break;
            default:
                ARES_PANIC(("'%c' is not a valid arg-type", args[i].first));
        }
    }
}

void Exception::unset_arguments()
{
    if (m_args) {
        for (int i = 0; i < m_num_args; i++)
            if (m_args[i].first == TYPE_STR && m_args[i].second.str_val != 0)
                delete [] m_args[i].second.str_val;
        delete [] m_args;
        m_args = 0;
        m_num_args = 0;
    }
}

//void Exception::swap(Exception& e)
//{
//    swap(m_module, e.m_module);
//    swap(m_code, e.m_code);
//    swap(m_args, e.m_args);
//    swap(m_num_args, e.m_num_args);
//    swap(m_next, e.m_next);
//}
