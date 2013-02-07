// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/job/scheduler.hpp"
#include "ares/job/error.hpp"
#include "ares/job/job.hpp"
#include "ares/job/job_queue.hpp"
#include "ares/guard.hpp"
#include "ares/mutex.hpp"
#include "ares/platform.hpp"
#include "ares/sequence.hpp"
#include "ares/shared_queue.hpp"
#include "ares/thread.hpp"
#include <assert.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

// Primary data structures:
//
//  JOB_QUEUE (priority queue; Scheduler dequeues, Workers enqueue)
//  RUN_QUEUE (FIFO queue; Scheduler enqueues, Workers dequeue)
//  JOBS_RUNNING (maps job ID to job, Workers add/remove)
//
// General algorithm:
//
// o Scheduler polls job_queue for ready jobs once per second; for each ready
// job in job_queue, Scheduler dequeues from job_queue and enqueues into
// run_queue
//
// o Idle workers block on run_queue.dequeue; when a new job is available,
// Worker dequeues from run_queue and inserts job into jobs_running
//
// o When worker finishes job, it computes job's next run date and adds job to
// job_queue (unless interval==0, in which case the job is deleted)

// FIXME: destroying the scheduler while a job is active will have undefined
// effects. It's possible for a concurrently running process to access the
// scheduler post-destruction. For that reason, it's a good idea to exit the
// program immediately after destroying the scheduler. (This could be fixed,
// but is it worth the effort?)

// FIXME: broken jobs are still run

enum {
    MAX_JOB_THREADS = 128,
    MAX_JOB_FAILURES = 16,
};

// Scheduler implementation
struct ares::job::Scheduler::Impl : public ares::Thread::Runnable {
    Impl();
    virtual ~Impl();

    // Starts up this scheduler.
    void startup();

    // Shuts down this scheduler.
    void shutdown();

    // Changes the number of job processes in the scheduler.
    void set_num_processes(int n);

    // Returns a job by its ID. Throws a Job_not_found_error if not found.
    // WARNING: this function is not re-entrant! The calling function must
    // hold the appropriate lock.
    Job* find_job(int job_id);

    // Grabs the next job that is ready to run. Returns null on timeout.
    Job* dequeue_ready_job(int timeout);

    // The main scheduler loop.
    void run();

    // Performs administrative tasks necessary before a job can be run.
    void run_startup(Job*);

    // Performs administrative tasks necessary after a job runs.
    void run_shutdown(Job*);

    // Removes a specific worker process and deletes it; this function is
    // typically called by a worker process itself if it aborts.
    void remove_process(Scheduler::Process* p);

    typedef vector<Scheduler::Process*> Process_array;
    typedef map<int, Job*> Job_table;
    typedef Shared_queue<Job*> Run_queue;
    typedef Sequence<Null_mutex> Job_sequence;

    Mutex m_mutex;              // protects Job_table/Job_queue
    Thread m_thread;            // main scheduler thread
    Process_array m_processes;  // array of job processes
    Job_sequence m_sequence;    // for generating job IDs
    Job_table m_jobs;           // primary data structure
    Job_queue m_job_queue;      // priority queue of scheduled jobs
    Run_queue m_run_queue;      // queue of jobs waiting for a worker
    bool m_stopped;             // true if scheduler was stopped
};


// A worker process. This object is "self-deleting," that is, it deletes
// itself immediately after its run function exits.
class ares::job::Scheduler::Process : public ares::Thread::Runnable {
  public:
    Process(Scheduler::Impl& scheduler);
    virtual ~Process() {}
    void stop();
    void run();

  private:
    Scheduler::Impl* m_scheduler;   // the parent scheduler
    Thread m_thread;                // our independent thread
    bool m_stopped;                 // true when signalled to stop
};


ares::job::Scheduler::Impl::Impl()
        : m_thread(this)
{}

ares::job::Scheduler::Impl::~Impl()
try
{
    // Shutdown all running processes.
    set_num_processes(0);   // will not throw

    Guard guard(m_mutex);

    // Delete jobs that are ready to run but haven't been started.
    vector<Job*> jobs;
    if (m_run_queue.dequeue_all(jobs))
        for (int i = 0; i < int(jobs.size()); i++)
            delete jobs[i];

    // Delete jobs that are not currently running.
    while (Job* job = m_job_queue.top()) {
        delete job;
        m_job_queue.remove_top();
    }

    // Empty our master table of jobs.
    m_jobs.clear();
}
catch (...) {}

void ares::job::Scheduler::Impl::startup()
{
    m_thread.start();
}

void ares::job::Scheduler::Impl::shutdown()
{
    m_stopped = true;
    set_num_processes(0);   // will not throw
    m_thread.wait_for_exit(1000);
}

void ares::job::Scheduler::Impl::set_num_processes(int n)
{
    Guard guard(m_mutex);

    // Make sure n is in the valid range.
    if (n < 0)
        n = 0;
    else if (n > MAX_JOB_THREADS)
        n = MAX_JOB_THREADS;

    if (int(m_processes.size()) < n) {
        while (int(m_processes.size()) < n) {
            auto_ptr<Process> process(new Process(*this));
            m_processes.push_back(process.release());
        }
    }
    else if (int(m_processes.size()) > n) {
        while (int(m_processes.size()) > n) {
            // We don't need to delete the Process objects here because they
            // self-delete immediately after exiting their main loops. Thus,
            // we don't have to block while the processes finish running their
            // current jobs, because they will clean up after themselves.
            Process* process = m_processes.back();
            m_processes.pop_back();
            process->stop();
        }
    }
}

ares::job::Job* ares::job::Scheduler::Impl::find_job(int job_id)
{
    Job_table::iterator it(m_jobs.find(job_id));
    if (it == m_jobs.end()) throw Job_not_found_error(job_id);
    return it->second;
}

ares::job::Job* ares::job::Scheduler::Impl::dequeue_ready_job(int timeout)
{
    // This function does not acquire a lock because the underlying queue
    // object, m_run_queue, is reentrant.

    Job* job = 0;
    if (m_run_queue.dequeue(job, timeout))
        return job;
    return 0;
}

void ares::job::Scheduler::Impl::run()
{
    m_stopped = false;
    while (!m_stopped) {
        milli_sleep(1000);
        Guard guard(m_mutex);
        try {
            Date now(Date::now());
            Job* job;

            // While job queue is non-empty and top job is ready to run:
            while ((job = m_job_queue.top()) && (job->next_date() <= now)) {
                m_job_queue.remove_top();   // remove job from priority queue
                m_run_queue.enqueue(job);   // add job to run queue
            }
        }
        catch (Timeout_error&) {
            ARES_PANIC(("unexpected timeout"));
        }
    }
}

void ares::job::Scheduler::Impl::run_startup(Job* job)
{
    Guard guard(m_mutex);
    job->set_this_date(Date::now());
    job->set_broken(false);
}

void ares::job::Scheduler::Impl::run_shutdown(Job* job)
{
    if (job->interval().is_null()) {
        delete job;
        return;
    }

    job->set_last_date(job->this_date());
    job->set_this_date(Date::now());
    job->set_next_date(job->interval().next_date(Date::now()));

    if (job->num_failures() >= MAX_JOB_FAILURES)
        job->set_broken(true);

    // Insert the job into the priority queue only if it is still in our
    // master table. If it is not, that means it was removed while it was
    // being run. (In the latter case, we delete the job instead.)

    Guard guard(m_mutex);

    if (m_jobs.find(job->id()) != m_jobs.end())
        m_job_queue.insert(job);     // reschedules job
    else
        delete job;                  // job was removed
}

void ares::job::Scheduler::Impl::remove_process(Scheduler::Process* process)
{
    Guard guard(m_mutex);
    vector<Scheduler::Process*>::iterator i(m_processes.begin());
    for (; i != m_processes.end(); ++i) {
        if (*i == process) {
            m_processes.erase(i);
            return;
        }
    }
}


ares::job::Scheduler::Process::Process(Scheduler::Impl& scheduler)
        : m_scheduler(&scheduler)
        , m_thread(this)
{
    m_thread.start();
}

void ares::job::Scheduler::Process::stop()
{
    m_stopped = true;
}

void ares::job::Scheduler::Process::run()
        try
        {
            enum { TIMEOUT = 1000 };    // millis to wait for a ready job

            m_stopped = false;

            while (!m_stopped) {
                Job* job = m_scheduler->dequeue_ready_job(TIMEOUT);
                if (job) {
                    m_scheduler->run_startup(job);

                    // Run the job, and if it completes without throwing an
                    // exception, clear its error record. If it does throw,
                    // however, increment its failure count.

                    try {
                        job->task()->run();
                        job->clear_failures();
                    }
                    catch (Exception& e) {
                        job->add_failure(e.to_string());
                    }
                    catch (...) {
                        job->add_failure("unknown exception");
                    }

                    m_scheduler->run_shutdown(job);
                }
            }

            delete this;    // self-destruct!
        }
        catch (...) {
            m_scheduler->remove_process(this);
            delete this;    // self-destruct!
        }


ares::job::Scheduler::Scheduler()
        : m_impl(new Impl)
{}

ares::job::Scheduler::~Scheduler()
{
    m_impl->shutdown();
    delete m_impl;
}

void ares::job::Scheduler::startup()
{
    m_impl->startup();
}

void ares::job::Scheduler::shutdown()
{
    m_impl->shutdown();
}

int ares::job::Scheduler::submit(Task* task, Date next_date, Interval interval)
{
    Guard guard(m_impl->m_mutex);
    assert(task != 0);

    // Create a Job object to represent the submitted job.
    int job_id = m_impl->m_sequence.next_val();
    Job* job = new Job(job_id, task, next_date, interval);

    // Insert the Job into our master table.
    bool was_inserted = m_impl->m_jobs.insert(make_pair(job_id, job)).second;
    assert(was_inserted);

    // Add the Job to our priority queue of runnable jobs.
    m_impl->m_job_queue.insert(job);
    return job_id;
}

bool ares::job::Scheduler::remove(int job_id)
{
    Guard guard(m_impl->m_mutex);

    // Find the job in our table.
    Impl::Job_table::iterator it = m_impl->m_jobs.find(job_id);
    if (it == m_impl->m_jobs.end())
        return false;

    // Remove the job from our table.
    Job* job = it->second;
    m_impl->m_jobs.erase(it);

    // Try to remove from the priority queue. If the job was in the queue, we
    // can safely delete it now. Otherwise, it is currently being executed, in
    // which case the scheduler will delete it upon completion.

    if (m_impl->m_job_queue.remove(job))
        delete job;

    return true;
}

void ares::job::Scheduler::set_num_processes(int n)
{
    m_impl->set_num_processes(n);
}

void ares::job::Scheduler::set_next_date(int job_id, Date next_date)
{
    Guard guard(m_impl->m_mutex);
    m_impl->m_job_queue.set_next_date(m_impl->find_job(job_id), next_date);
}

void ares::job::Scheduler::set_interval(int job_id, Interval interval)
{
    Guard guard(m_impl->m_mutex);
    m_impl->find_job(job_id)->set_interval(interval);
}

void ares::job::Scheduler::set_broken(int job_id, bool broken)
{
    Guard guard(m_impl->m_mutex);
    m_impl->find_job(job_id)->set_broken(broken);
}

int ares::job::Scheduler::num_processes() const
{
    Guard guard(m_impl->m_mutex);
    return m_impl->m_processes.size();
}

ares::Date ares::job::Scheduler::next_date(int job_id) const
{
    Guard guard(m_impl->m_mutex);
    return m_impl->find_job(job_id)->next_date();
}

ares::job::Interval ares::job::Scheduler::interval(int job_id) const
{
    Guard guard(m_impl->m_mutex);
    return m_impl->find_job(job_id)->interval();
}

bool ares::job::Scheduler::is_broken(int job_id) const
{
    Guard guard(m_impl->m_mutex);
    return m_impl->find_job(job_id)->is_broken();
}
