// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_job_error
#define included_ares_job_error

#include "ares/error.hpp"

namespace ares { namespace job {

struct Errors {
    enum {
        JOB_NOT_FOUND = 1,
    };
};

struct Error : public ares::Exception {
    Error(int error_code)
            : Exception("ares.job", error_code) {}

    Error(int error_code, char const* arg_types, ...)
            : Exception("ares.job", error_code) { ARES_SET_EXCEPTION_VARARGS }
};

struct Job_not_found_error : public Error {
    Job_not_found_error(int job_id)
            : Error(Errors::JOB_NOT_FOUND, "d", job_id) {}

    char const* message() const;
};

} } // namespace ares::job

#endif
