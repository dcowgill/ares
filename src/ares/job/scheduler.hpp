// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_job_scheduler
#define included_ares_job_scheduler

#include "ares/job/common.hpp"
#include "ares/job/interval.hpp"
#include "ares/utility.hpp"

namespace ares { namespace job {

// A class that allows programs to schedule user jobs for periodic execution.
// Each job is represented by a Thread::Runnable object, which corresponds to
// the "main," or top-level, function for that job. Jobs can be run exactly
// once or can be scheduler for recurring execution using an Interval object
// (see that class's documentation for a thorough explanation of the
// scheduling algorithm). Any member function that takes a job_id will throw a
// Job_Not_Found_Error if no job with the specified id exists.
//
// NOTE: All scheduler member functions are thread-safe.
class Scheduler : boost::noncopyable {
  public:
    Scheduler();
    ~Scheduler();

    // Starts up this scheduler. Until this function is called, the scheduler
    // will not run any scheduled jobs.
    void startup();

    // Shuts down this scheduler. The scheduler will cease running scheduled
    // jobs after this function is called, but any jobs that are currently
    // running will not be interrupted.
    void shutdown();

    // Submits a job to the scheduler, returning the job's unique ID. The job
    // will be executed at next_date, or immediately if next_date is in the
    // past. After the job runs, interval will be applied to the current time
    // to calculate the job's next run date; if interval is a null interval,
    // the job will be run exactly once. The scheduler assumes ownership of
    // the what pointer.
    int submit(Task* task, Date next_date = Date(),
               Interval interval = Interval());

    // Removes the job with ID job_id. If the job is currently running, it
    // will not be interrupted.
    bool remove(int job_id);

    // Sets the number of job processes that can concurrently execute
    // scheduled jobs. Note that jobs may not run when scheduled if there are
    // no available job processes to run them. The maximum number of job
    // processes is 128, and the minimum is zero.
    void set_num_processes(int n);

    // Changes the next run date for a specified job.
    void set_next_date(int job_id, Date next_date);

    // Changes the run interval for a specified job.
    void set_interval(int job_id, Interval interval);

    // Changes the broken status of a specified job. A broken job will not be
    // run by the scheduler. The scheduler will automatically break a job if
    // it fails 16 consecutive times.
    void set_broken(int job_id, bool broken);

    // Returns the number of threads the scheduler uses to run concurrent
    // jobs. If there are not enough job processes, some jobs may not run
    // according to their defined schedules.
    int num_processes() const;

    // Returns the next run date for the job with ID job_id.
    Date next_date(int job_id) const;

    // Returns the interval for the job with ID job_id.
    Interval interval(int job_id) const;

    // Returns the status of the job with ID job_id.
    bool is_broken(int job_id) const;

  private:
    struct Impl;
    class Process;

    Impl* m_impl;
};

} } // namespace ares::job

#endif
