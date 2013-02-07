// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/pid_lock.hpp"
#include "ares/error.hpp"
#include "ares/platform.hpp"
#include "ares/string_util.hpp"
#include <cassert>
#include <cstdio>
#include <cerrno>
#include <sys/file.h>

using namespace std;
using ares::PID_lock;

PID_lock::PID_lock(string filename)
        : m_filename(filename)
        , m_fp(0)
{}

PID_lock::~PID_lock()
{
    reset();
}

pair<bool, string> PID_lock::acquire(bool block)
{
    if (m_fp)
        return make_pair(true, parse_pid_from_file());

    if (!(m_fp = fopen(m_filename.c_str(), "a")))
        throw System_error("fopen", errno);

    if (flock(fileno(m_fp), LOCK_EX | (block ? 0 : LOCK_NB)) != 0) {
        int const flock_errno = errno;
        reset();
        if (flock_errno == EWOULDBLOCK)
            return make_pair(false, parse_pid_from_file());
        throw System_error("flock", flock_errno);
    }

    string const pid = system_process_id();
    if (fseek(m_fp, 0, SEEK_SET) != 0)
        raise_system_error("fseek");
    if (ftruncate(fileno(m_fp), 0) != 0)
        raise_system_error("ftruncate");
    if (fputs((pid + "\n").c_str(), m_fp) == EOF)
        raise_system_error("fputs");
    if (fflush(m_fp) != 0)
        raise_system_error("fflush");

    return make_pair(true, pid);
}

string PID_lock::parse_pid_from_file()
{
    FILE* fp = fopen(m_filename.c_str(), "r");
    if (!fp)
        throw IO_error("fopen", errno);
    char buf[20];
    if (!fgets(buf, sizeof(buf), fp))
        return "";
    return boost::trim_right_copy(string(buf));
}

void PID_lock::raise_system_error(char const* syscall, int syscall_errno)
{
    int const saved_errno = syscall_errno==0 ? errno : syscall_errno;
    reset();
    throw System_error(syscall, saved_errno);
}

void PID_lock::reset()
{
    if (m_fp) {
        fclose(m_fp);
        m_fp = 0;
    }
}
