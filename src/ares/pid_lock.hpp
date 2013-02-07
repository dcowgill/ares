// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_pid_lock
#define included_ares_pid_lock

#include <boost/utility.hpp>
#include <cstdio>
#include <string>

namespace ares {

// Minimalist file-based PID lock. If two concurrently running processes
// instantiate a PID_lock and give it the same filename, only one will be able
// to successfully acquire the lock. The loser process can insepct the PID of
// the winner process via the acquire member function's return value.
class PID_lock : boost::noncopyable {
  public:
    // Constructs a PID lock with the given filename. Does not attempt to
    // acquire a lock on the file.
    PID_lock(std::string filename);

    // Releases this PID lock if it was successfully acquired. Otherwise, does
    // nothing.
    ~PID_lock();

    // Attempts to acquire this PID lock. If block is true, this function will
    // not return until the lock has been acquired. Returns a pair of values:
    // the first is a boolean indicating whether this process managed to
    // acquire the lock; the second is the PID of the process that owns the
    // lock (i.e. on success, it will be the calling process's PID; on
    // failure, it will be the PID of some other process). Has no effect if
    // the lock is already held by this process.
    std::pair<bool, std::string> acquire(bool block = false);

  private:
    std::string parse_pid_from_file();
    void raise_system_error(char const* syscall, int syscall_errno = 0);
    void reset();

    std::string const m_filename;
    FILE* m_fp;
};

} // namespace ares

#endif
