// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_job_job
#define included_ares_job_job

// This is an implementation file; do not use directly.

#include "ares/job/common.hpp"
#include "ares/job/interval.hpp"
#include "ares/thread.hpp"
#include "ares/date.hpp"
#include <string>

namespace ares { namespace job {

class Job {
  public:
    Job(int id, Task* task, Date next_date, Interval interval);
    ~Job();

    Task* task() { return m_task; }
    void set_last_date(Date d);
    void set_this_date(Date d);
    void set_next_date(Date d);
    void set_interval(Interval interval);
    void set_broken(bool is_broken);
    void add_failure(std::string const& error_string);
    void clear_failures();

    int id() const { return m_id; }
    Date last_date() const { return m_last_date; }
    Date this_date() const { return m_this_date; }
    Date next_date() const { return m_next_date; }
    Interval interval() const { return m_interval; }
    int num_failures() const { return m_num_failures; }
    std::string const& last_failure() const { return m_last_failure; }
    bool is_broken() const { return m_broken; }
    int time_running() const { return m_time_running; }
    int total_time() const { return m_total_time; }

  private:
    int const m_id;                 // unique job ID
    Task* m_task;                   // the actual task to execute
    Date m_last_date;               // last time this job ran
    Date m_this_date;               // time job started running
    Date m_next_date;               // next time job will run
    Interval m_interval;            // interval betw. last/next date
    int m_time_running;             // seconds job has been running
    int m_total_time;               // total seconds spent running
    int m_num_failures;             // consecutive failures
    std::string m_last_failure;     // error message for last failure
    bool m_broken;                  // true if job is not runnable
};

} } // namespace ares::job

#endif
