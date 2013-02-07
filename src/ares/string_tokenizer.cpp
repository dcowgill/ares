// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/string_tokenizer.hpp"

using namespace std;
using ares::String_tokenizer;

namespace
{
string const DEFAULT_DELIM = " \t\n\r\f";

inline bool is_delim(char c, string const& delim) {
    return delim.find(c) != string::npos;
}
}

String_tokenizer::String_tokenizer(string const& s)
        : m_string(s)
        , m_delim(DEFAULT_DELIM)
        , m_cur_pos(0)
        , m_max_pos(s.length() - 1)
{}

String_tokenizer::String_tokenizer(string const& s, string const& delim)
        : m_string(s)
        , m_delim(delim)
        , m_cur_pos(0)
        , m_max_pos(s.length() - 1)
{}

String_tokenizer::~String_tokenizer()
{}

void String_tokenizer::reset(string const& s)
{
    m_string = s;
    m_cur_pos = 0;
    m_max_pos = s.length() - 1;
}

void String_tokenizer::set_delim(string const& delim)
{
    has_next();  // move past any delimiter characters
    m_delim = delim;
}

string const& String_tokenizer::next()
{
    // If there are no more tokens, throw an exception
    if (!has_next()) {
        throw String_tokenizer_underflow();
    }

    // 'has_next' left us on a non-delimiter; find the next delimiter
    int next_delim = m_string.find_first_of(m_delim, m_cur_pos + 1);
    if (next_delim == (int) string::npos) {
        next_delim = m_max_pos + 1;
    }

    // Store the token and advance our position
    m_token.assign(m_string, m_cur_pos, next_delim - m_cur_pos);
    m_cur_pos = next_delim + 1;
    return m_token;
}

bool String_tokenizer::has_next() const
{
    // If we're past the end of the string, there can't be more tokens
    if (m_cur_pos > m_max_pos) {
        return false;
    }

    // If we're on a delimiter, find the next non-delimiter
    if (is_delim(m_string[m_cur_pos], m_delim)) {
        m_cur_pos = m_string.find_first_not_of(m_delim, m_cur_pos + 1);
        if (m_cur_pos == (int) string::npos) {
            m_cur_pos = m_max_pos + 1;
            return false;
        }
    }

    // We found a non-delimiter; there are still tokens
    return true;
}

string String_tokenizer::rest() const
{
    return has_next() ? m_string.substr(m_cur_pos) : "";
}
