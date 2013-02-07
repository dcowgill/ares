// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/http/error.hpp"
#include "ares/http/http.hpp"

ares::http::Error::Error(int status_code)
        : ares::Exception("ares.http", status_code, "s",
                          status_code_to_string(status_code).c_str())
{}

char const* ares::http::Client_error::message() const
{
    return "HTTP client error: %1$s";
}

char const* ares::http::Server_error::message() const
{
    return "HTTP server error: %1$s";
}
