// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/component.hpp"
#include "ares/error.hpp"
#include "ares/log.hpp"
#include "ares/string_util.hpp"
#include <cassert>

using namespace std;
using ares::Component;

Component::Component(string name, string id, bool is_thread)
        : m_name(name)
        , m_id(id)
        , m_is_thread(is_thread)
        , m_active(false)
        , m_thread(this)
        , m_stopped(false)
{}

Component::~Component()
{
    if (is_active()) {
        Log::writef(Log::WARNING, "%s not cleanly shutdown", m_name.c_str());
    }
}

void Component::startup() try
{
    if (is_active()) {
        Log::writef(Log::WARNING, "cannot start already-running %s - "
                    "shut it down first", m_name.c_str());
        return;
    }

    Log::writef(Log::DEBUG, "%s starting up", m_name.c_str());

    // Call the derived class's startup function.

    try {
        do_startup();
    }
    catch (Exception& e) {
        Log::writef(Log::ERROR, "%s startup failed: %s",
                    m_name.c_str(), e.to_string().c_str());
        throw;
    }

    // Start the internal thread.

    try {
        if (m_is_thread) {
            assert(!m_thread.is_running());
            m_stopped = false;
            m_thread.start();
        }
    }
    catch (Exception& e) {
        Log::writef(Log::ERROR, "%s startup failed - thread not started: %s",
                    m_name.c_str(), e.to_string().c_str());

        try {
            do_shutdown();
        }
        catch (...) {
            Log::writef(Log::ERROR, "%s - could not rollback aborted startup",
                        m_name.c_str());
            throw;
        }
        throw;
    }

    // Startup complete.

    m_active = true;
    m_started = Date::now();

    Log::writef(Log::DEBUG, "%s [%s] startup complete",
                m_name.c_str(), m_id.c_str());
}
catch (...) {
    Log::writef(Log::ERROR, "%s startup failed - unhandled exception",
                m_name.c_str());
    throw;
}

void Component::shutdown() try
{
    if (is_idle())
        return;

    Log::writef(Log::DEBUG, "%s starting shutdown", m_name.c_str());

    // Stop the internal thread.

    try {
        if (m_is_thread) {
            m_stopped = true;
            m_thread.wait_for_exit(thread_wait_time());
            assert(!m_thread.is_running());
        }
    }
    catch (Timeout_error) {
        Log::writef(Log::WARNING, "%s thread exit timed out", m_name.c_str());
    }
    catch (System_error& e) {
        Log::writef(Log::WARNING, "could not stop %s thread: %s",
                    m_name.c_str(), e.to_string().c_str());
    }

    // Call the derived class's shutdown function.

    try {
        do_shutdown();
    }
    catch (Exception& e) {
        Log::writef(Log::ERROR, "%s shutdown failed: %s",
                    m_name.c_str(), e.to_string().c_str());
        throw;
    }

    // Shutdown complete.

    m_active = false;
    Log::writef(Log::DEBUG, "%s [%s] shutdown complete",
                m_name.c_str(), m_id.c_str());
}
catch (...) {
    Log::writef(Log::ERROR, "%s shutdown failed - unhandled exception",
                m_name.c_str());
    throw;
}

void Component::stop()
{
    m_stopped = true;
}

int Component::uptime() const
{
    if (is_idle())
        return 0;

    return int((Date::now() - started()) * 24 * 60 * 60);
}

int Component::thread_wait_time() const
{
    return 2000; // milliseconds
}
