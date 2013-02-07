// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_job_common
#define included_ares_job_common

namespace ares { namespace job {

// Represents the actual task that a job must perform. This class is analogous
// to the job's "main" function.
class Task {
  public:
    virtual ~Task() {}
    virtual void run() = 0;
};

} } // namespace ares::job

#endif
