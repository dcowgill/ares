// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_command
#define included_ares_command

#include "ares/buffer.hpp"
#include "ares/session.hpp"

namespace ares {

class Server_interface;

// A server command. Pass one of these to Server_interface::enqueue_command
// and it will be executed asynchronously in one of several parallel processor
// threads. Each processor thread has a unique id, which is passed to the
// command's execute member function via the pid argument.
class Command {
  public:
    virtual ~Command();
    virtual void execute(Server_interface& server, int pid) = 0;
};

// A command that requires a session on which it should operate. Any command
// that requires a session should either derive from this class or from
// Typed_session_command (see below).
class Session_command : public Command {
  public:
    Session_command(Session s);
    virtual ~Session_command();
    Session session() { return m_session; }

  private:
    Session m_session;
};

// Asynchronously adds the session s to the system. Adding a session should
// always be done via this command.
class Add_session_command : public Session_command {
  public:
    Add_session_command(Session s);
    void execute(Server_interface& server, int pid);
};

// Asynchronously removes the session s from the system. Removing a session
// should always be done via this command.
class Remove_session_command : public Session_command {
  public:
    Remove_session_command(Session s);
    void execute(Server_interface& server, int pid);
};

// Asynchronously sends the buffer b to the session s. This is the primary
// means by which asynch-i/o occurs in the server framework.
class Dispatch_command : public Session_command {
  public:
    Dispatch_command(Session s, Buffer const& b);
    void execute(Server_interface& server, int pid);

  private:
    Buffer* m_buffer;
};

// Instructs the server to invoke the given session's handle_processing member
// function, passing it the id of the calling processor thread. Useful for
// performing asynchronous processing in a session.
class Process_session_command : public Session_command {
  public:
    Process_session_command(Session s);
    void execute(Server_interface& server, int pid);
};

// Instructs the server to delete the specified object. If an object can be
// expensive to delete (e.g. because it manages resources that are very
// time-consuming to clean up), it should be deleted asynchronously in a
// processor thread using this command.
template<typename T>
struct Delete_object_command : public Command {
    Delete_object_command(T* ptr) : m_ptr(ptr) {}
    void execute(Server_interface&, int) { delete m_ptr; }
  private:
    T* m_ptr;
};

// Utility template for creating commands that accept sessions of a specific
// derived type. If you need to define a command class that should only
// operate on sessions of a specific type, derive from this class instead of
// Session_command.
template<class S>
struct Typed_session_command : public Session_command {
    Typed_session_command(S* s) : Session_command(s) {}
    S* session_rep() { return static_cast<S*>(session().get()); }
};

} // namespace ares

#endif
