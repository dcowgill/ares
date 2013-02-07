// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/log.hpp"
#include "ares/date.hpp"
#include "ares/error.hpp"
#include "ares/guard.hpp"
#include "ares/mutex.hpp"
#include "ares/shared_queue.hpp"
#include "ares/socket.hpp"
#include "ares/string_util.hpp"
#include "ares/thread.hpp"
#include "ares/trace.hpp"
#include <cstdio>
#include <cstring>
#include <map>

using namespace std;
using namespace ares;

// This thread is responsible for actual i/o of the log messages.
class Log_writer : public Thread::Runnable {
    void run();
    void write(const pair<int,char*>& p);
};


typedef Shared_queue<pair<int, char*> > Msg_queue;
typedef map<string, pair<int, FILE*> > File_tab;
typedef File_tab::iterator File_tab_iter;
typedef map<Socket*, int> Socket_tab;
typedef Socket_tab::iterator Socket_tab_iter;

// Logging data structures.
struct Log_internal {
    Mutex       m_mutex;            // global lock
    Msg_queue   m_queue;            // queue of pending log entries
    File_tab    m_files;            // maps filenames to handles
    Socket_tab  m_sockets;          // maps socket ptrs to level
    int         m_min_level;        // minimum level of attached outputs
    Log_writer  m_lgwr;             // log writer
    Thread      m_lgwr_thread;      // log writer thread
    bool        m_lgwr_stopped;     // lgwr stopped?
    string      m_level_text[Log::NUM_LEVELS];

    Log_internal();
    ~Log_internal();
    void reset_min_level();
};


// private definitions
namespace
{
Log_internal* s_log=0;

const string& level_to_string(int level)
{
    if (level < 0 || level >= Log::NUM_LEVELS)
        throw Invalid_log_level_error();
    return s_log->m_level_text[level];
}

// Formats and writes a log message to a file output.
void write_msg_to_file(FILE* fp,
                       const char* msg,
                       const string& now,
                       int level)
{
    fputs(now.c_str(), fp);
    fputc(' ', fp);
    fputc('(', fp);
    fputs(level_to_string(level).c_str(), fp);
    fputc(')', fp);
    fputc(' ', fp);
    fputs(msg, fp);
    fputc('\n', fp);
}

// Formats and writes a log message to a file output.
void write_message_to_socket(Socket* s,
                             const char* msg,
                             const string& now,
                             int level)
{
    const string& lvl_str = level_to_string(level);

    s->write((const Byte*) now.data(), now.length());
    s->write((const Byte*) " (", 2);
    s->write((const Byte*) lvl_str.data(), lvl_str.length());
    s->write((const Byte*) ") ", 2);
    s->write((const Byte*) msg, strlen(msg));
    s->write((const Byte*) "\n", 1);
}
}


// The Log_writer main loop.
void Log_writer::run()
{
    pair<int, char*> item;

    Trace::set_thread_name("log_writer");

    while (!s_log->m_lgwr_stopped)
        if (s_log->m_queue.dequeue(item, 1000))
            write(item);

    // Write all remaining log messages before shutting down.
    while (s_log->m_queue.dequeue(item))
        write(item);
}

// Writes a single log message to all valid destinations.
void Log_writer::write(pair<int, char*> const& p)
{
    string const now = Date::now().to_string();
    Guard guard(s_log->m_mutex);

    // Write to attached files:

    for (File_tab_iter it(s_log->m_files.begin());
         it != s_log->m_files.end(); ++it)
    {
        pair<int, FILE*> file = it->second;
        if (p.first >= file.first) {
            write_msg_to_file(file.second, p.second, now, p.first);
            if (fflush(file.second) != 0) {
                Log::writef(Log::ERROR,
                            "i/o error writing to log file (%s): %s",
                            it->first.c_str(),
                            strerror(errno));
                s_log->m_files.erase(it++);  // erase as we go
            }
        }
    }

    // Write to attached sockets:

    for (Socket_tab_iter it(s_log->m_sockets.begin());
         it != s_log->m_sockets.end(); ++it)
    {
        if (p.first >= it->second) {
            Socket* socket = it->first;
            try {
                write_message_to_socket(socket, p.second, now, p.first);
            }
            catch (Exception& e) {
                Log::writef(Log::ERROR,
                            "i/o error writing to socket (%s): %s",
                            socket->to_string().c_str(),
                            e.to_string().c_str());
                s_log->m_sockets.erase(it++);  // erase as we go
            }
        }
    }

    delete [] p.second;  // free the message string
}

Log_internal::Log_internal()
        : m_min_level(Log::FATAL)
        , m_lgwr_thread(&m_lgwr)
        , m_lgwr_stopped(true)
{
    m_level_text[Log::FATAL]   = "fatal";
    m_level_text[Log::ERROR]   = "error";
    m_level_text[Log::WARNING] = "warning";
    m_level_text[Log::NOTICE]  = "notice";
    m_level_text[Log::DEBUG]   = "debug";
}

Log_internal::~Log_internal()
{
    for (File_tab_iter it(m_files.begin()); it != m_files.end(); ++it) {
        fclose(it->second.second);
    }
}

// Reset m_min_level to the lowest log level of all attached outputs.
void Log_internal::reset_min_level()
{
    m_min_level = Log::FATAL;

    for (File_tab_iter it(m_files.begin()); it != m_files.end(); ++it)
        if (m_min_level > it->second.first)
            m_min_level = it->second.first;

    for (Socket_tab_iter it(m_sockets.begin()); it != m_sockets.end(); ++it)
        if (m_min_level > it->second)
            m_min_level = it->second;
}


void Log::startup()
{
    if (!s_log->m_lgwr_stopped)
        return;

    s_log->m_lgwr_stopped = false;
    s_log->m_lgwr_thread.start();
}

void Log::shutdown()
{
    if (s_log->m_lgwr_stopped)
        return;

    s_log->m_lgwr_stopped = true;
    s_log->m_lgwr_thread.wait_for_exit(10*1000);
}

void Log::attach(string filename, int level) try
{
    filename = boost::trim_copy(filename);

    Guard guard(s_log->m_mutex);       // lock the file table
    File_tab_iter it(s_log->m_files.find(filename));
    if (it == s_log->m_files.end()) {
        FILE* fp = fopen(filename.c_str(), "a");
        if (!fp) throw IO_error("fopen", errno);
        s_log->m_files[filename] = make_pair(level, fp);
    }
    else {
        int const old_level = it->second.first;
        it->second.first = level;   // already attached, just change level
        if (s_log->m_min_level == old_level && old_level < level)
            s_log->reset_min_level();
    }

    if (s_log->m_min_level > level)
        s_log->m_min_level = level;
}
catch (Exception& cause) {
    Log_file_attach_error e(filename);
    e.set_cause(cause);
    throw e;
}

bool Log::detach(string filename)
{
    filename = boost::trim_copy(filename);

    Guard guard(s_log->m_mutex);       // lock the file table
    File_tab_iter it(s_log->m_files.find(filename));
    if (it != s_log->m_files.end()) {
        if (fclose(it->second.second) != 0) {
            Log::writef(Log::WARNING, "i/o error closing log file (%s)",
                        it->first.c_str());
        }
        if (s_log->m_min_level == it->second.first) {
            s_log->reset_min_level();
        }
        s_log->m_files.erase(it);
        return true;
    }
    return false;   // file doesn't exist
}

void Log::attach(Socket& socket, int level)
{
    Guard guard(s_log->m_mutex);       // lock the socket table
    Socket_tab_iter it(s_log->m_sockets.find(&socket));
    if (it == s_log->m_sockets.end()) {
        socket.set_blocking(false);
        s_log->m_sockets[&socket] = level;
    }
    else {
        int const old_level = it->second;
        it->second = level;   // already attached, just change level
        if (s_log->m_min_level == old_level && old_level < level)
            s_log->reset_min_level();
    }

    if (s_log->m_min_level > level)
        s_log->m_min_level = level;
}

bool Log::detach(Socket& socket)
{
    Guard guard(s_log->m_mutex);       // lock the socket table
    Socket_tab_iter it(s_log->m_sockets.find(&socket));
    if (it != s_log->m_sockets.end()) {
        if (s_log->m_min_level == it->second)
            s_log->reset_min_level();
        s_log->m_sockets.erase(it);
        return true;
    }
    return false;   // socket doesn't exist
}

void Log::write(int level, const string& msg)
{
    write(level, msg.c_str());
}

void Log::write(int level, const char* msg)
{
    // Don't bother sending this message if no one will receive it.
    if (s_log->m_min_level > level)
        return;

    s_log->m_queue.enqueue(make_pair(level, copy_string(msg)));
}

void Log::writef(int level, const char* fmt, ...)
{
    // Don't bother sending this message if no one will receive it.
    if (s_log->m_min_level > level)
        return;

    va_list args;
    int size = 200;
    char* p = new char[size];

    for (;;) {
        va_start(args, fmt);
        int n = vsnprintf(p, size, fmt, args);
        va_end(args);

        if (n >= 0 && n < size)
            break;

        if (n >= 0)
            size = n+1;
        else
            size *= 2;

        delete [] p;
        p = new char[size];
    }

    ARES_TRACE((p));
    s_log->m_queue.enqueue(make_pair(level, p));
}


// module initialization

namespace { int nifty_counter = 0; }

Log_init::Log_init()
{
    if (0 == nifty_counter++) {
        s_log = new Log_internal;
    }
}

Log_init::~Log_init()
{
    if (0 == --nifty_counter) {
        try { Log::shutdown(); } catch (Timeout_error&) {}
        delete s_log;
    }
}
