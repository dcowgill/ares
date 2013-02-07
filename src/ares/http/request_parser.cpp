// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/http/request_parser.hpp"
#include "ares/http/error.hpp"
#include "ares/http/http.hpp"
#include "ares/http/request.hpp"
#include "ares/string_util.hpp"
#include <cassert>
#include <cerrno>
#include <cstdio>       // used by Request_parser::parse_file

using namespace std;
using namespace ares;

namespace
{
// A whitespace string (for String_tokenizer).
string const& WS = " ";

// We accept only so many headers in a single request.
int const MAX_HEADERS = 100;

// We must also put some maximum on the length of an entity body.
int const MAX_CONTENT_LENGTH = 1024*1024;

// Chunks are limited in size by the maximum content length.
int const MAX_CHUNKS = 10;
int const MAX_CHUNK_SIZE = MAX_CONTENT_LENGTH / MAX_CHUNKS;

// used by Request_parser::parse_file
void read_error(FILE* fp)
{
    if (fclose(fp) != 0)
        throw IO_error("fclose", errno);
    else
        throw http::Error(http::Codes::BAD_REQUEST);
}
}


http::Request_parser::Request_parser()
        : m_state(STATE_REQUEST_LINE)
        , m_tok("", WS)
{}

void http::Request_parser::add_line(string const& s)
{
    if (!wants_line())
        throw Server_error(Codes::INTERNAL_SERVER_ERROR);

    // Explanation of why we erase all trailing whitespace from the line:
    //
    // "Clients SHOULD be tolerant in parsing the Status-Line and servers
    // tolerant when parsing the Request-Line. In particular, they SHOULD
    // accept any amount of SP or HT characters between fields, even though
    // only a single SP is required." (RFC-2616)
    //
    // "The line terminator for message-header fields is the sequence CRLF.
    // However, we recommend that applications, when parsing such headers,
    // recognize a single LF as a line terminator and ignore the leading CR."
    // (RFC-2616)

    string line = boost::trim_right_copy(s);

    switch (m_state) {
        case STATE_REQUEST_LINE:
            parse_request_line(line);
            break;

        case STATE_HEADER:
        case STATE_HEADER_CONT:
            parse_header(line);
            break;

        case STATE_CHUNK_SIZE:
        case STATE_CHUNK_DATA:
            parse_chunk(line);

        default:
            throw Server_error(Codes::INTERNAL_SERVER_ERROR);
    }
}

void http::Request_parser::add_data(char const* begin, char const* end)
{
    if (!wants_data())
        throw Server_error(Codes::INTERNAL_SERVER_ERROR);

    // Make sure we haven't been given more data than we need. If we have,
    // it's our error (i.e., a bug in the parser or a bug in the caller).

    if (end - begin > num_bytes_wanted())
        throw Server_error(Codes::INTERNAL_SERVER_ERROR);

    // Append to the message body. If we have all we need, transition state.

    m_body.insert(m_body.end(), begin, end);
    if (num_bytes_wanted() == 0) {
        m_state = STATE_DONE;
    }
}

http::Request* http::Request_parser::make_request()
{
    if (!is_done())
        return 0;
    return new Request(m_method, m_uri, m_version, m_headers, m_body);
}

void http::Request_parser::reset()
{
    m_state = STATE_REQUEST_LINE;
    m_headers.clear();
    m_body.clear();
}

void http::Request_parser::parse_request_line(string const& s)
{
    // The first line should be the request line. The general format of a
    // request line is "Method SP Request-URI SP HTTP-Version". A robust
    // server should also ignore empty leading lines.

    if (s.empty())      // ignore leading CRLFs
        return;

    m_tok.reset(s);

    if (!m_tok.has_next())
        throw Client_error(Codes::BAD_REQUEST);
    m_method = find_method(m_tok.next());

    if (!m_tok.has_next())
        throw Client_error(Codes::BAD_REQUEST);
    m_uri = m_tok.next();

    // HTTP/1.0 clients somtimes omit the version. If the version is missing,
    // therefore, we assume we are dealing with an HTTP/1.0 client.

    if (m_tok.has_next())
        m_version = determine_version(m_tok.next());
    else
        m_version = VERSION_1_0;

    m_state = STATE_HEADER;     // advance the parse state
}

void http::Request_parser::parse_header(string const& s)
{
    // If the header line is empty, we must transition state. (The logic used
    // to determine our next state is sufficient complicated to warrant its
    // own function.)

    if (s.empty()) {
        add_current_header();
        transition_from_header_state();
        return;
    }

    // If the line begins with a space or tab and the parse state is
    // STATE_HEADER_CONT state, this is a continuation of the preceeding
    // header line.

    if (m_state == STATE_HEADER_CONT) {
        if (s[0] == ' ' || s[0] == '\t') {
            m_curr_value += " ";
            m_curr_value += boost::trim_left_copy(s);
            return; // we're done with this line
        }
        else {
            // The fact that the current line doesn't begin with whitespace
            // tells us that the header we weren't sure was complete is indeed
            // complete. We must add it to the header table now.

            if (m_state == STATE_HEADER_CONT)
                add_current_header();
        }
    }

    m_state = STATE_HEADER;     // make sure we're in the right parse state

    // Enforce the maximum number of headers per request.

    if (m_headers.size() >= MAX_HEADERS)
        throw Client_error(Codes::BAD_REQUEST);

    // A header line resembles this: "Header-Name: Header-Value". Because this
    // is so straightforward to parse, we do it without the help of a string
    // tokenizer. But there is one wrinkle: header lines can span multiple
    // lines, as long as the succeeding lines begin with spaces or tabs.

    string::size_type pos = s.find(':');
    if (pos == string::npos)
        throw Client_error(Codes::BAD_REQUEST);

    m_curr_header = s.substr(0, pos);
    m_curr_value = boost::trim_copy(s.substr(pos + 1));

    if (m_curr_header == "")
        throw Client_error(Codes::BAD_REQUEST);

    // Advance to the "continued-header" state, indicating that we have a
    // header, but we can't be sure that it has been terminated.

    m_state = STATE_HEADER_CONT;
}

void http::Request_parser::parse_chunk(string const& s)
{
    // length := 0
    // read chunk-size, chunk-ext (if any) and CRLF
    // while (chunk-size > 0) {
    //     read chunk-data and CRLF
    //     append chunk-data to entity-body
    //     length := length + chunk-size
    //     read chunk-size and CRLF
    // }
    // read entity-header
    // while (entity-header not empty) {
    //     append entity-header to existing header fields
    //     read entity-header
    // }
    // Content-Length := length
    // Remove "chunked" from Transfer-Encoding

    throw Server_error(Codes::NOT_IMPLEMENTED);
}

void http::Request_parser::transition_from_header_state()
{
    bool has_chunk_encoding = false;
    bool has_content_length = false;

    // Check for a Transfer-Encoding header.

    if (m_headers.exists(Headers::TRANSFER_ENCODING)) {
        string transfer_encoding =
                boost::to_lower_copy(m_headers[Headers::TRANSFER_ENCODING]);

        if (transfer_encoding == "chunked") {
            has_chunk_encoding = true;
        }
        else {
            // "A server which receives an entity-body with a transfer-coding
            // it does not understand SHOULD return 501 (Unimplemented), and
            // close the connection." (RFC-2616)

            throw Server_error(Codes::NOT_IMPLEMENTED);
        }
    }

    // Check for a Content-Length header.

    if (m_headers.exists(Headers::CONTENT_LENGTH)) {

        // Convert the Content-Length header to an integer and throw an
        // appropriate client error exception if underflow or overflow occurs.

        try {
            m_content_length =
                    boost::lexical_cast<int>(
                        m_headers[Headers::CONTENT_LENGTH]);
        }
        catch (boost::bad_lexical_cast& cause) {
            throw Client_error(Codes::REQUEST_ENTITY_TOO_LARGE);
        }

        has_content_length = true;
    }

    // Make sure the client is not ambiguous about the entity body.

    if (has_chunk_encoding && has_content_length)
        throw Client_error(Codes::BAD_REQUEST);

    // Transition state based on the presence of the message body.

    if (has_chunk_encoding)
        m_state = STATE_CHUNK_SIZE;
    else if (has_content_length)
        m_state = STATE_ENTITY_BODY;
    else
        m_state = STATE_DONE;
}

void http::Request_parser::add_current_header()
{
    if (m_state == STATE_HEADER_CONT) {
        assert(!m_curr_header.empty());
        m_headers.add(m_curr_header, dequote(m_curr_value));
        m_curr_header = m_curr_value = "";
    }
}

http::Request* http::Request_parser::parse_string(std::string const& s)
{
    reset();

    string line;
    char const* p = s.data();
    char const* end = p + s.length();
    char* q;

    while (!is_done()) {
        if (wants_line()) {
            if (!(q = strchr(p, '\n')))
                throw Client_error(Codes::BAD_REQUEST);
            line.assign(p, q - p + 1);
            add_line(line);
            p = q + 1;
        }
        else {
            if (num_bytes_wanted() > end - p)
                throw Client_error(Codes::BAD_REQUEST);
            add_data(p, p + num_bytes_wanted());
            p += num_bytes_wanted();
        }
    }

    return make_request();
}

http::Request* http::Request_parser::parse_file(std::string const& filename)
{
    reset();

    // Try to open the file.

    FILE* fp = fopen(filename.c_str(), "r");
    if (!fp)
        throw IO_error("fopen", errno);

    // Read either newline terminated-strings or chunks of data from the file,
    // as ordered by the parser, until the parse is complete and we encounter
    // an error.

    char buf[2028];
    string line;

    while (!is_done()) {
        if (wants_line()) {
            if (!fgets(buf, sizeof(buf), fp))
                read_error(fp);
            line.assign(buf);
            add_line(line);
        }
        else {
            int n = min(num_bytes_wanted(), int(sizeof(buf)));
            if (fread(buf, sizeof(char), n, fp) <= 0)
                read_error(fp);
            add_data(buf, buf + n);
        }
    }

    fclose(fp);
    return make_request();
}
