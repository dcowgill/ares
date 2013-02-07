// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/mutex.hpp"

using ares::Mutex;

Mutex::Mutex()
{
    if (pthread_mutex_init(&m_mutex, 0) != 0)  // use default attributes
        throw System_error("pthread_mutex_init", errno);
}

Mutex::~Mutex()
{
    if (pthread_mutex_destroy(&m_mutex) != 0)
        ; // TODO log this somehow
}

void Mutex::acquire()
{
    if (pthread_mutex_lock(&m_mutex) != 0)
        throw System_error("pthread_mutex_lock", errno);
}

bool Mutex::acquire_no_wait()
{
    int n = pthread_mutex_trylock(&m_mutex);
    if (n != 0 && n != EBUSY)
        throw System_error("pthread_mutex_trylock", errno);
    return !n;  // true when n is zero
}

void Mutex::release()
{
    if (pthread_mutex_unlock(&m_mutex) != 0)
        throw System_error("pthread_mutex_unlock", errno);
}

void* Mutex::handle()
{
    return &m_mutex;
}
