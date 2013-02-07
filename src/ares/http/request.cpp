// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/http/request.hpp"

using namespace std;
using namespace ares;

http::Request::Request(Method method,
                       string const& uri,
                       Version version,
                       Header_table const& headers,
                       vector<char> body)
        : m_method(method)
        , m_uri(uri)
        , m_version(version)
        , m_headers(headers)
        , m_body(body)
{}

http::Method http::Request::method() const
{
    return m_method;
}

string const& http::Request::uri() const
{
    return m_uri;
}

http::Version http::Request::version() const
{
    return m_version;
}

http::Header_table const& http::Request::headers() const
{
    return m_headers;
}

bool http::Request::has_message_body() const
{
    return m_body.size() > 0;
}

vector<char> const& http::Request::message_body() const
{
    return m_body;
}
