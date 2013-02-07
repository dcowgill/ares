// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/rwlock.hpp"
#include "ares/platform.hpp"

ares::Rwlock::Rwlock()
{
    if (pthread_rwlock_init(&m_rwlock, 0) != 0)
        throw System_error("pthread_rwlock_init", errno);
}

ares::Rwlock::~Rwlock()
{
    if (pthread_rwlock_destroy(&m_rwlock) != 0)
        ;  // is there any way to log this?
}

void ares::Rwlock::acquire_read()
{
    if (pthread_rwlock_rdlock(&m_rwlock) != 0)
        throw System_error("pthread_rwlock_rdlock", errno);
}

void ares::Rwlock::acquire_write()
{
    if (pthread_rwlock_wrlock(&m_rwlock) != 0)
        throw System_error("pthread_rwlock_wrlock", errno);
}

void ares::Rwlock::release()
{
    if (pthread_rwlock_unlock(&m_rwlock) != 0)
        throw System_error("pthread_rwlock_unlock", errno);
}
