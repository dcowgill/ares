// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_http_error
#define included_ares_http_error

#include "ares/error.hpp"

namespace ares { namespace http {

// The base class for all HTTP errors.
struct Error : public ares::Exception {
    explicit Error(int status_code);
};

// A general HTTP client error. Thrown by an HTTP server when a client request
// cannot be understood or fulfilled, typically due to an error in the client
// implementation.
struct Client_error : public Error {
    Client_error(int status_code) : Error(status_code) {}
    char const* message() const;
};

// A general HTTP server error. Thrown by an HTTP server when a server request
// cannot be understood or fulfilled, typically due to a limitation of or an
// error in the server implementation.
struct Server_error : public Error {
    Server_error(int status_code) : Error(status_code) {}
    char const* message() const;
};

} } // namespace ares::http

#endif
