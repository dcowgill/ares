// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/condition.hpp"
#include "ares/mutex.hpp"
#include <sys/time.h>       // timeval{} for gettimeofday()

using ares::Condition;

namespace
{
// Convert relative time in milliseconds to an absolute future time.
struct timespec relative_to_absolute_time(int millis)
{
    static int const million = 1000 * 1000;

    // Convert milliseconds to seconds and microseconds, storing in t.
    struct timespec t;
    t.tv_sec = millis / 1000;
    t.tv_nsec = (millis - (t.tv_sec * 1000)) * 1000;

    // Store current time in v (seconds and microseconds).
    struct timeval v = { 0,0 };
    if (gettimeofday(&v, 0) != 0) {
        t.tv_sec += ares::current_time();  // error: fallback to time(0)
        t.tv_nsec *= 1000;
        return t;
    }

    // Add current time in v to t.
    t.tv_sec += v.tv_sec;
    t.tv_nsec += v.tv_usec;

    // Handle overflow of microseconds in t.tv_nsec.
    if (t.tv_nsec >= million) {
        int seconds = t.tv_nsec / million;
        t.tv_sec += seconds;
        t.tv_nsec -= seconds * million;
    }

    // Convert microseconds in t.tv_nsec to nanoseconds.
    t.tv_nsec *= 1000;

    return t;
}
}

Condition::Condition(Mutex& m)
        : m_mutex(m)
{
    if (pthread_cond_init(&m_cond, 0) != 0)
        throw System_error("pthread_cond_init", errno);
}

Condition::~Condition()
{
    if (pthread_cond_destroy(&m_cond) != 0)
        ;  // TODO log this somehow
}

bool Condition::wait(int millis)
{
    pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(m_mutex.handle());

    // If millis is zero, do not use a timeout.
    if (millis <= 0) {
        if (pthread_cond_wait(&m_cond, mutex) != 0)
            throw System_error("pthread_cond_wait", errno);
        return true;
    }

    // Millis is non-zero; compute the timeout and use a timed-wait.
    struct timespec timeout = relative_to_absolute_time(millis);

    errno = 0;
    if (pthread_cond_timedwait(&m_cond, mutex, &timeout) != 0) {
        if (errno != EINVAL) {
            //
            // The above test should actually be errno==ETIMEDOUT, but some
            // POSIX platforms don't set errno when pthread_cond_timedwait
            // times out.
            //
            return false;
        }
        throw System_error("pthread_cond_timedwait", errno);
    }

    return true;    // success
}

void Condition::signal()
{
    if (pthread_cond_signal(&m_cond) != 0)
        throw System_error("pthread_cond_signal", errno);
}

void Condition::broadcast()
{
    if (pthread_cond_broadcast(&m_cond) != 0)
        throw System_error("pthread_cond_broadcast", errno);
}
