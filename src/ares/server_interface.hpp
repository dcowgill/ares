// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_server_interface
#define included_ares_server_interface

#include "ares/date.hpp"
#include "ares/job/scheduler.hpp"
#include "ares/session.hpp"

namespace ares {

class Buffer;
class Command;

// Server_interface is the interface through with Command objects communicate
// with the ares framework. All member functions must be thread-safe because
// any number of Processor objects may invoke them at the same time.
class Server_interface {
  public:
    virtual ~Server_interface() {}

    // Adds a session to the server. This function is the only means by which
    // a session can be integrated with the system.
    virtual void add_session(Session s) = 0;

    // Removes a session from the server. This function is the only means by
    // which a session can be released from the system.
    virtual void remove_session(Session s) = 0;

    // Enqueues a command for eventual execution. This server assumes
    // ownership of the command and will delete it after it has been executed.
    // Therefore, c must be a pointer to a heap-allocated Command object.
    virtual void enqueue_command(Command* c) = 0;

    // Works like enqueue_command, except the command will not be enqueued for
    // execution until at least num_seconds seconds have elapsed. Depending on
    // how many processing threads are available, the delay could be greater
    // tha num_seconds.
    virtual void enqueue_delayed_command(Command* c, int num_seconds) = 0;

    // Sends a buffer to the output processor for deferred handling.
    virtual void dispatch(Session s, Buffer* bp) = 0;

    // Returns a reference to the server's central job scheduler. The job
    // facility allows users to schedule runnable objects to be run on a
    // periodic basis.
    virtual job::Scheduler& scheduler() = 0;

    // Shuts down the server.
    virtual void shutdown() = 0;

    // Returns the time the server was started.
    virtual Date started() const = 0;

    // Returns the number of seconds the server has been continuously running,
    // or zero if the server is not currently running.
    virtual int uptime() const = 0;
};

} // namespace ares

#endif
