// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_http_http
#define included_ares_http_http

#include <string>

namespace ares { namespace http {

// Supported HTTP request methods.
enum Method {
    METHOD_UNKNOWN,
    METHOD_GET,
    METHOD_HEAD,
    METHOD_OPTIONS,
    METHOD_POST,
    METHOD_PUT,
    METHOD_TRACE,
    NUM_METHODS
};

// Supported HTTP versions.
enum Version {
    VERSION_1_0,
    VERSION_1_1
};

// Standard HTTP headers.
struct Headers {
    // General headers
    static std::string const CACHE_CONTROL;
    static std::string const CONNECTION;
    static std::string const DATE;
    static std::string const PRAGMA;
    static std::string const TRANSFER_ENCODING;
    static std::string const UPGRADE;
    static std::string const VIA;

    // Request headers
    static std::string const ACCEPT;
    static std::string const ACCEPT_CHARSET;
    static std::string const ACCEPT_ENCODING;
    static std::string const ACCEPT_LANGUAGE;
    static std::string const AUTHORIZATION;
    static std::string const FROM;
    static std::string const HOST;
    static std::string const IF_MODIFIED_SINCE;
    static std::string const IF_MATCH;
    static std::string const IF_NONE_MATCH;
    static std::string const IF_RANGE;
    static std::string const IF_UNMODIFIED_SINCE;
    static std::string const MAX_FORWARDS;
    static std::string const PROXY_AUTHORIZATION;
    static std::string const RANGE;
    static std::string const REFERER;
    static std::string const USER_AGENT;

    // Entity headers
    static std::string const ALLOW;
    static std::string const CONTENT_BASE;
    static std::string const CONTENT_ENCODING;
    static std::string const CONTENT_LANGUAGE;
    static std::string const CONTENT_LENGTH;
    static std::string const CONTENT_LOCATION;
    static std::string const CONTENT_MD5;
    static std::string const CONTENT_RANGE;
    static std::string const CONTENT_TYPE;
    static std::string const ETAG;
    static std::string const EXPIRES;
    static std::string const LAST_MODIFIED;
};

// HTTP status codes.
struct Codes {
    enum {
        // 1xx: Informational
        CONTINUE                            = 100,
        SWITCHING_PROTOCOLS                 = 101,

        // 2xx: Successful
        OK                                  = 200,
        CREATED                             = 201,
        ACCEPTED                            = 202,
        NON_AUTHORITATIVE_INFORMATION       = 203,
        NO_CONTENT                          = 204,
        RESET_CONTENT                       = 205,
        PARTIAL_CONTENT                     = 206,

        // 3xx: Redirection
        MULTIPLE_CHOICES                    = 300,
        MOVED_PERMANENTLY                   = 301,
        FOUND                               = 302,
        SEE_OTHER                           = 303,
        NOT_MODIFIED                        = 304,
        USE_PROXY                           = 305,
        TEMPORARY_REDIRECT                  = 307,

        // 4xx: Client Error
        BAD_REQUEST                         = 400,
        UNAUTHORIZED                        = 401,
        PAYMENT_REQUIRED                    = 402,
        FORBIDDEN                           = 403,
        NOT_FOUND                           = 404,
        METHOD_NOT_ALLOWED                  = 405,
        NOT_ACCEPTABLE                      = 406,
        PROXY_AUTHENTICATION_REQUIRED       = 407,
        REQUEST_TIME_OUT                    = 408,
        CONFLICT                            = 409,
        GONE                                = 410,
        LENGTH_REQUIRED                     = 411,
        PRECONDITION_FAILED                 = 412,
        REQUEST_ENTITY_TOO_LARGE            = 413,
        REQUEST_URI_TOO_LARGE               = 414,
        UNSUPPORTED_MEDIA_TYPE              = 415,
        REQUESTED_RANGE_NOT_SATISFIABLE     = 416,
        EXPECTATION_FAILED                  = 417,

        // 5xx: Server Error
        INTERNAL_SERVER_ERROR               = 500,
        NOT_IMPLEMENTED                     = 501,
        BAD_GATEWAY                         = 502,
        SERVICE_UNAVAILABLE                 = 503,
        GATEWAY_TIME_OUT                    = 504,
        HTTP_VERSION_NOT_SUPPORTED          = 505
    };
};

// Converts an integer status code to its textual equivalent. The code
// must be one of the values enumerated in Http::Codes.
std::string status_code_to_string(int status_code);

// Finds a request method by name and returns the corresponding
// Http::Method value. If no matching method can be found, returns
// Http::METHOD_UNKNOWN.
Method find_method(std::string const& s);

// Determines the HTTP protocol version based on a version string. If the
// version cannot be determined, defaults to HTTP/1.0.
Version determine_version(std::string const& s);

} } // namespace ares::http

#endif
