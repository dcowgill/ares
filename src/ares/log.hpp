// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_log
#define included_ares_log

#include "ares/socket.hpp"
#include <string>

namespace ares {

// TODO: syslog support
// TODO: automatically roll logfiles

// A thread-safe and scalable logging facility. Conceptually, there is only
// one log per server, but there may be multiple backends (places for the log
// output to go). Output backends may be files, which are specified by a
// filesystem path, or TCP sockets, which are specified by a connected socket.
// All log messages are associated with a log level, which determines which
// backends will receive the message.
struct Log {
    // The logging levels, which indicate the severity or importance of the
    // log entry.
    enum {
        DEBUG,      // for debugging purposes only
        NOTICE,     // informational message
        WARNING,    // minor error; may be ignored
        ERROR,      // non-fatal error
        FATAL,      // fatal error; causes program to abort
        NUM_LEVELS  // (must be last entry)
    };

    // Starts the log writer. Until this function is called, no messages will
    // actually be written to output backends. Has no effect if the log writer
    // is already running.
    static void startup();

    // Shuts down the log writer. Waits for the log writer thread to stop
    // before returning. Has no effect if the log writer is not running.
    static void shutdown();

    // Attaches a file backend to the log. The file is specified by its path,
    // and if it can't be opened, this function will raise an error. If
    // successful, all log messages with a log level of at least the specified
    // level will be written to the file.
    static void attach(std::string filename, int level = NOTICE);

    // Detaches and closes a previously attached file. Returns true if the
    // filename was valid (it was being logged to), false otherwise.
    static bool detach(std::string filename);

    // Attaches a socket to the log, causing all log messages with a log level
    // of at least the specified level to be sent to it. The logger does NOT
    // assume ownership of the supplied socket.
    //
    // Note: any socket attached to the log will be set to non-blocking mode
    // to prevent a broken connection from blocking the log writer thread.
    //
    // Note: delivery of log messages to sockets is not guaranteed; it's
    // possible for a message to be sent in part or not at all.
    static void attach(Socket& socket, int level = NOTICE);

    // Detaches a previously attached socket. The socket is not closed.
    static bool detach(Socket& socket);

    // Writes a log message to the output backends that accept messages with
    // the given level.
    static void write(int level, const std::string& msg);

    // Exactly like Log::write(int,const std::string&) but more efficient when
    // a temporary string object does not need to be constructed.
    static void write(int level, const char* msg);

    // Similar to write(level,std::string const&), except this function
    // accepts a printf-style format string and multiple arguments.
    static void writef(int level, char const* fmt, ...);
};

// module initializer
struct Log_init { Log_init(); ~Log_init(); };
static Log_init s_ares_log_init;

} // namespace ares

#endif
