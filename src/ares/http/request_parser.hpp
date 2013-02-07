// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_http_request_parser
#define included_ares_http_request_parser

#include "ares/http/http.hpp"
#include "ares/http/header_table.hpp"
#include "ares/string_tokenizer.hpp"
#include <string>
#include <vector>

namespace ares { namespace http {

class Request;

// A parser for HTTP/1.0 and HTTP/1.1 requests. Fully supports the HTTP/1.0
// standard. Supports the basics of the HTTP/1.1 standard and can handle all
// web requests sent by modern browsers, with a few exceptions. The parser
// interface is specifically intended to be used as a state machine, enabling
// a single thread of control to service multiple clients. Thus, each client
// should should generally be given a dedicated parser instance.
// The only transfer-encoding supported by this parser is "chunked." A request
// containing any other encoding will raise a Server_error exception.
// Assume that any member function of this class may raise either a
// Client_error or a Server_error exception.
class Request_parser {
  public:
    // Default constructor.
    Request_parser();

    // Adds a line to the request. Raises a Server_error if called when
    // wants_line would return false. The string passed to this function need
    // not be terminated by a newline; that is, the caller may strip the
    // newline if desired.
    void add_line(std::string const& s);

    // Adds a range of bytes to the request. Raises a Server_error if called
    // when wants_data would return false or if the range of bytes is longer
    // than the value returned by num_bytes_wanted.
    void add_data(char const* begin, char const* end);

    // Creates a new Request object from the parse results. Only returns a
    // valid object if is_done is true; otherwise, returns null.
    Request* make_request();

    // Resets the parser, preparing it to parse another request.
    void reset();

    // Returns true if the next input to the parser should be a line of text.
    // Also see add_line.
    bool wants_line() const;

    // Returns true if the parser wants num_bytes_wanted bytes of input.
    bool wants_data() const;

    // Returns the number of raw input bytes needed by the parser. Returns 0
    // if wants_data would return false.
    int num_bytes_wanted() const;

    // Returns true if the parser has completed parsing a request. If so, a
    // Request object may safely be obtained by calling make_request.
    bool is_done() const;

    Request* parse_string(std::string const& s);
    Request* parse_file(std::string const& filename);

  private:
    void parse_request_line(std::string const& s);
    void parse_header(std::string const& s);
    void parse_chunk(std::string const& s);
    void transition_from_header_state();
    void add_current_header();

  private:
    // The parser is a state machine; the following are its possible states.
    enum Parser_state {
        STATE_REQUEST_LINE, // expects request line
        STATE_HEADER,       // expects a header or an empty line
        STATE_HEADER_CONT,  // STATE_HEADER or line with leading whitespace
        STATE_CHUNK_SIZE,   // expects a part of a chunked message body
        STATE_CHUNK_DATA,   // expects a part of a chunked message body
        STATE_ENTITY_BODY,  // expects a message body of known length
        STATE_DONE          // done parsing (request complete)
    };

    Parser_state m_state;           // current parse state
    Method m_method;                // request method
    std::string m_uri;              // request URI
    Version m_version;              // HTTP version of request
    Header_table m_headers;         // table of headers
    int m_content_length;           // number of bytes needed for body
    std::vector<char> m_body;       // message body (if any)
    std::string m_curr_header;      // current (incomplete) hdr
    std::string m_curr_value;       // current (incomplete) hdr value
    String_tokenizer m_tok;         // for tokenizing request lines
};


inline
bool Request_parser::wants_line() const
{
    return !wants_data() && !is_done();
}

inline
bool Request_parser::wants_data() const
{
    return m_state == STATE_ENTITY_BODY;
}

inline
int Request_parser::num_bytes_wanted() const
{
    return wants_data() ? m_content_length - m_body.size() : 0;
}

inline
bool Request_parser::is_done() const
{
    return m_state == STATE_DONE;
}

} } // namespace ares::http

#endif
