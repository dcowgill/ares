// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_job_job_queue
#define included_ares_job_job_queue

// This is an implementation file; do not use directly.

#include "ares/job/job.hpp"
#include <set>

namespace ares { namespace job {

class Job_queue {
  public:
    // Adds a job to the queue.
    void insert(Job* job);

    // Removes the highest priority job from the queue. Returns true if
    // a job was removed, else false (meaning the queue was empty).
    bool remove_top();

    // Removes a specific job from the priority queue.
    bool remove(Job* job);

    // Changes the next run date for a specific job.
    void set_next_date(Job* job, Date next_date);

    // Returns the highest priority job in the queue, or null if there are
    // no jobs in the queue.
    Job* top() const;

    // For debugging.
    void dump() const;

  private:
    // Orders Job pointers by next_date ("less" is having a later next_date).
    // Jobs with equivalent next_date values are ordered by ID.
    struct Job_ptr_less {
        bool operator()(Job* a, Job* b) {
            if (a->next_date() == b->next_date())
                return a->id() < b->id();
            return a->next_date() < b->next_date();
        }
    };

    typedef std::set<Job*, Job_ptr_less> Set_type;
    Set_type m_set;     // priority queue implementation
};

} } // namespace ares::job

#endif
