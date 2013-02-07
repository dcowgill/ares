// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/http/http.hpp"
#include "ares/string_util.hpp"
#include <cstdio>

using namespace std;
using namespace ares;

namespace
{
char const* REASON_PHRASES_1XX[] = {
    // 1xx: Informational
    "Continue",
    "Switching Protocols",
};

char const* REASON_PHRASES_2XX[] = {
    // 2xx: Successful
    "OK",
    "Created",
    "Accepted",
    "Non-Authoritative Information",
    "No Content",
    "Reset Content",
    "Partial Content",
};

char const* REASON_PHRASES_3XX[] = {
    // 3xx: Redirection
    "Multiple Choices",
    "Moved Permanently",
    "Found",
    "See Other",
    "Not Modified",
    "Use Proxy",
    "Temporary Redirect",
};

char const* REASON_PHRASES_4XX[] = {
    // 4xx: Client Error
    "Bad Request",
    "Unauthorized",
    "Payment Required",
    "Forbidden",
    "Not Found",
    "Method Not Allowed",
    "Not Acceptable",
    "Proxy Authentication Required",
    "Request Time-out",
    "Conflict",
    "Gone",
    "Length Required",
    "Precondition Failed",
    "Request Entity Too Large",
    "Request-URI Too Large",
    "Unsupported Media Type",
    "Requested range not satisfiable",
    "Expectation Failed",
};

char const* REASON_PHRASES_5XX[] = {
    // 5xx: Server Error
    "Internal Server Error",
    "Not Implemented",
    "Bad Gateway",
    "Service Unavailable",
    "Gateway Time-out",
    "HTTP Version not supported",
};

#define NELEMS(a) (sizeof(a) / sizeof(a[0]))
int const MAX_1xx = 100 + NELEMS(REASON_PHRASES_1XX) - 1;
int const MAX_2xx = 200 + NELEMS(REASON_PHRASES_2XX) - 1;
int const MAX_3xx = 300 + NELEMS(REASON_PHRASES_3XX) - 1;
int const MAX_4xx = 400 + NELEMS(REASON_PHRASES_4XX) - 1;
int const MAX_5xx = 500 + NELEMS(REASON_PHRASES_5XX) - 1;
#undef NELEMS
}

// General headers
string const http::Headers::CACHE_CONTROL = "cache-control";
string const http::Headers::CONNECTION = "connection";
string const http::Headers::DATE = "date";
string const http::Headers::PRAGMA = "pragma";
string const http::Headers::TRANSFER_ENCODING = "transfer-encoding";
string const http::Headers::UPGRADE = "upgrade";
string const http::Headers::VIA = "via";

// Request headers
string const http::Headers::ACCEPT = "accept";
string const http::Headers::ACCEPT_CHARSET = "accept-charset";
string const http::Headers::ACCEPT_ENCODING = "accept-encoding";
string const http::Headers::ACCEPT_LANGUAGE = "accept-language";
string const http::Headers::AUTHORIZATION = "authorization";
string const http::Headers::FROM = "from";
string const http::Headers::HOST = "host";
string const http::Headers::IF_MODIFIED_SINCE = "if-modified-since";
string const http::Headers::IF_MATCH = "if-match";
string const http::Headers::IF_NONE_MATCH = "if-none-match";
string const http::Headers::IF_RANGE = "if-range";
string const http::Headers::IF_UNMODIFIED_SINCE = "if-unmodified-since";
string const http::Headers::MAX_FORWARDS = "max-forwards";
string const http::Headers::PROXY_AUTHORIZATION = "proxy-authorization";
string const http::Headers::RANGE = "range";
string const http::Headers::REFERER = "referer";
string const http::Headers::USER_AGENT = "user-agent";

// Entity headers
string const http::Headers::ALLOW = "allow";
string const http::Headers::CONTENT_BASE = "content-base";
string const http::Headers::CONTENT_ENCODING = "content-encoding";
string const http::Headers::CONTENT_LANGUAGE = "content-language";
string const http::Headers::CONTENT_LENGTH = "content-length";
string const http::Headers::CONTENT_LOCATION = "content-location";
string const http::Headers::CONTENT_MD5 = "content-md5";
string const http::Headers::CONTENT_RANGE = "content-range";
string const http::Headers::CONTENT_TYPE = "content-type";
string const http::Headers::ETAG = "etag";
string const http::Headers::EXPIRES = "expires";
string const http::Headers::LAST_MODIFIED = "last-modified";

string ares::http::status_code_to_string(int status_code)
{
    static string const UNKNOWN = "[Unknown Status Code]";

    if (status_code >= 100 && status_code <= MAX_1xx) {
        return REASON_PHRASES_1XX[status_code - 100];
    }
    if (status_code >= 200 && status_code <= MAX_2xx) {
        return REASON_PHRASES_2XX[status_code - 200];
    }
    if (status_code >= 300 && status_code <= MAX_3xx) {
        return REASON_PHRASES_3XX[status_code - 300];
    }
    if (status_code >= 400 && status_code <= MAX_4xx) {
        return REASON_PHRASES_4XX[status_code - 400];
    }
    if (status_code >= 500 && status_code <= MAX_5xx) {
        return REASON_PHRASES_5XX[status_code - 500];
    }
    return UNKNOWN;
}

http::Method ares::http::find_method(string const& s)
{
    // Because the two most common HTTP methods are GET and POST, we check for
    // those first before converting the string to uppercase and testing for
    // the less common methods.

    char const* t = s.c_str();

    if ((t[0] == 'G' || t[0] == 'g') &&
        (t[1] == 'E' || t[1] == 'e') &&
        (t[2] == 'T' || t[2] == 't') &&
        t[3] =='\0')
    {
        return METHOD_GET;
    }

    if ((t[0] == 'P' || t[0] == 'p') &&
        (t[1] == 'O' || t[1] == 'o') &&
        (t[2] == 'S' || t[2] == 's') &&
        (t[3] == 'T' || t[3] == 't') &&
        t[4] =='\0')
    {
        return METHOD_POST;
    }

    // The method is neither GET nor POST; check for the others.

    string m = boost::to_upper_copy(s);

    if (m == "PUT")
        return METHOD_PUT;
    if (m == "HEAD")
        return METHOD_HEAD;
    if (m == "OPTIONS")
        return METHOD_OPTIONS;
    if (m == "TRACE")
        return METHOD_TRACE;

    return METHOD_UNKNOWN;
}

http::Version ares::http::determine_version(string const& s)
{
    return (s == "HTTP/1.1") ? VERSION_1_1 : VERSION_1_0;
}
