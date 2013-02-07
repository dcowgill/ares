// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/job/job.hpp"
#include <cassert>

using namespace std;
using namespace ares;

ares::job::Job::Job(int id, Task* task, Date next_date, Interval interval)
        : m_id(id)
        , m_task(task)
        , m_next_date(next_date)
        , m_interval(interval)
        , m_time_running(0)
        , m_total_time(0)
        , m_broken(false)
{
    assert(m_task != 0);
}

ares::job::Job::~Job()
{
    delete m_task;
}

void ares::job::Job::set_last_date(Date d)
{
    m_last_date = d;
}

void ares::job::Job::set_this_date(Date d)
{
    // this function represents a "transition" state
    m_this_date = d;
    m_total_time += m_time_running;
    m_time_running = 0;
}

void ares::job::Job::set_next_date(Date d)
{
    m_next_date = d;
}

void ares::job::Job::set_interval(Interval interval)
{
    m_interval = interval;
}

void ares::job::Job::set_broken(bool is_broken)
{
    m_broken = is_broken;
}

void ares::job::Job::add_failure(string const& error_string)
{
    ++m_num_failures;
    m_last_failure = error_string;
}

void ares::job::Job::clear_failures()
{
    m_num_failures = 0;
    m_last_failure.clear();
}
