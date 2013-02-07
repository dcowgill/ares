// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/thread.hpp"
#include "ares/error.hpp"
#include "ares/platform.hpp"
#include <cassert>
#include <cerrno>

using namespace std;
using ares::Thread;

Thread::Thread(Runnable* r)
        : m_runnable(r)
        , m_running(false)
{
    pthread_attr_init(&m_attr);
    pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_DETACHED);
}

Thread::~Thread()
{
    pthread_attr_destroy(&m_attr);
}

void Thread::start()
{
    // Do not start a thread that is already running.
    if (is_running()) {
        throw Thread_already_running_error();
    }

    if (pthread_create(&m_thread, &m_attr, thread_wrapper, this) != 0) {
        throw System_error("pthread_create", errno);
    }
}

void Thread::wait_for_exit(int millis)
{
    // If millis is not valid, set it to the maximum reasonable wait time.
    if (millis <= 0) {
        millis = 60*1000;
    }

    // Use an exponential busy-wait strategy.
    int wait = 2;
    while (is_running() && wait <= millis) {
        milli_sleep(wait);
        wait *= 2;
    }

    // Raise an error if we timed out.
    if (wait >= millis && is_running()) {
        throw Thread_exit_timeout_error();
    }
}

void Thread::join()
{
    if (!is_running()) {
        throw Thread_not_running_error();
    }
    if (pthread_join(m_thread, 0)) {
        throw System_error("pthread_join", errno);
    }
}

void Thread::suspend()
{
    throw Not_implemented_error("suspend thread");
}

void Thread::resume()
{
    throw Not_implemented_error("resume thread");
}

bool Thread::is_running() const
{
    return m_running;
}

ares::Thread_id Thread::id() const
{
    if (!is_running()) {
        throw Thread_not_running_error();
    }
    return m_thread;
}

ares::Thread_id Thread::current_thread_id()
{
    return pthread_self();
}

void* Thread::thread_wrapper(void* self)
{
    // Call the run method of the thread's runnable object.
    Thread* t = static_cast<Thread*>(self);
    t->m_running = true;
    try { t->m_runnable->run(); } catch (...) {}    // enforce no throw
    t->m_running = false;
    return 0;
}
