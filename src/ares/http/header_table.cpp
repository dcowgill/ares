// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/http/header_table.hpp"

using namespace std;
using namespace ares;

http::Header_table::Header_table()
{}

http::Header_table::Header_table(Header_table const& t)
        : m_headers(t.m_headers)
{}

http::Header_table& http::Header_table::operator=(Header_table const& t)
{
    if (this != &t) {
        m_headers = t.m_headers;
    }
    return *this;
}

void http::Header_table::add(string const& name, string const& value)
{
    // FIXME: this is a serious bug in the HTTP implementation. It IS legal
    // for an HTTP message to contains duplicate headers; the values should be
    // formed into a comma-separated list. Unfortunately, this wreaks havoc
    // with the "dequoting" behavior of our request parser, and therefore we
    // conveniently ignore this requirement for now, overwriting previous
    // entries when duplicate header arrive. In practice, duplicate header
    // values are rarely seen.

    m_headers[name] = value;
}

void http::Header_table::remove(string const& name)
{
    m_headers.erase(name);
}

void http::Header_table::clear()
{
    m_headers.clear();
}

bool http::Header_table::exists(string const& name) const
{
    return m_headers.find(name) != m_headers.end();
}

string http::Header_table::operator[](string const& name) const
{
    Table_type::const_iterator it = m_headers.find(name);
    return it != m_headers.end() ? it->second : "";
}

http::Header_table::Table_type const& http::Header_table::table() const
{
    return m_headers;
}

int http::Header_table::size() const
{
    return m_headers.size();
}
