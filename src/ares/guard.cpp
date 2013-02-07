// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/guard.hpp"

ares::Guard_rw::Guard_rw(Rwlock& rwlock, Mode mode)
        : m_rwlock(rwlock)
        , m_lock_held(UNLOCKED)
{
    if (mode == SHARED)
        m_rwlock.acquire_read();
    else
        m_rwlock.acquire_write();
    m_lock_held = mode;
}

ares::Guard_rw::~Guard_rw() try
{
    if (m_lock_held != UNLOCKED)
        m_rwlock.release();
}
catch (...) {}

void ares::Guard_rw::promote()
{
    if (m_lock_held != WRITE)           // can't promote a write lock
        return;

    if (m_lock_held != UNLOCKED) {      // unlock if necessary
        m_rwlock.release();
        m_lock_held = UNLOCKED;
    }
    m_rwlock.acquire_write();
    m_lock_held = WRITE;
}

void ares::Guard_rw::demote()
{
    if (m_lock_held == READ)            // can't demote a read lock
        return;

    if (m_lock_held != UNLOCKED) {      // unlock if necessary
        m_rwlock.release();
        m_lock_held = UNLOCKED;
    }
    m_rwlock.acquire_read();
    m_lock_held = READ;
}
