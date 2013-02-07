// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/job/job_queue.hpp"
#include <cassert>
#include <cstdio>   // XXX for debugging only!

using namespace std;

void ares::job::Job_queue::insert(Job* job)
{
    bool was_inserted = m_set.insert(job).second;
    assert(was_inserted);
}

bool ares::job::Job_queue::remove_top()
{
    Set_type::iterator it = m_set.begin();
    if (it == m_set.end()) {
        return false;
    }
    m_set.erase(it);
    return true;
}

bool ares::job::Job_queue::remove(Job* job)
{
    return m_set.erase(job);
}

void ares::job::Job_queue::set_next_date(Job* job, Date next_date)
{
    // Priority changes are implemented as remove-insert
    if (remove(job)) {
        job->set_next_date(next_date);
        insert(job);
    }
}

ares::job::Job* ares::job::Job_queue::top() const
{
    Set_type::const_iterator it = m_set.begin();
    return it != m_set.end() ? *it : 0;
}

void ares::job::Job_queue::dump() const
{
    printf("**********************\n");
    for (Set_type::const_iterator i = m_set.begin(); i != m_set.end(); ++i) {
        printf("JOB %d: %s\n", (*i)->id(),
               (*i)->next_date().to_string().c_str());
    }
    printf("**********************\n");
}
