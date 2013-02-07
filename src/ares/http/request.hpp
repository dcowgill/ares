// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_http_request
#define included_ares_http_request

#include "ares/http/http.hpp"
#include "ares/http/header_table.hpp"
#include <string>
#include <vector>

namespace ares { namespace http {

class Request {
  public:
    Request(Method method,
            std::string const& uri,
            Version version,
            Header_table const& headers,
            std::vector<char> body);

    Method method() const;
    std::string const& uri() const;
    Version version() const;
    Header_table const& headers() const;
    bool has_message_body() const;
    std::vector<char> const& message_body() const;

  private:
    Method              m_method;   // request method
    std::string         m_uri;      // request URI
    Version             m_version;  // HTTP version of request
    Header_table        m_headers;  // table of headers
    std::vector<char>   m_body;     // message body (if any)
};

} } // namespace ares::http

#endif
