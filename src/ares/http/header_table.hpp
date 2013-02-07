// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_http_header_table
#define included_ares_http_header_table

#include "ares/http/http.hpp"
#include "ares/string_util.hpp"
#include <map>
#include <string>

namespace ares { namespace http {

class Header_table {
  public:
    typedef Compare_string_ignore_case Compare_type;
    typedef std::map<std::string, std::string, Compare_type> Table_type;

  public:
    Header_table();
    Header_table(Header_table const& t);
    Header_table& operator=(Header_table const& t);
    void add(std::string const& name, std::string const& value);
    void remove(std::string const& name);
    void clear();
    bool exists(std::string const& name) const;
    std::string operator[](std::string const& name) const;
    Table_type const& table() const;
    int size() const;

  private:
    Table_type m_headers;
};

} } // namespace ares::http

#endif
