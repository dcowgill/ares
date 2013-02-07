// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_session
#define included_ares_session

#include "ares/date.hpp"
#include "ares/shared_ptr.hpp"
#include "ares/sink.hpp"

namespace ares {

// forward declarations
class Buffer;
class Server_interface;
class Session_rep;
class Socket;

// Given a session, Session_info takes a snapshot of the session's state so
// that it may be queried afterward.
class Session_info {
  public:
    Session_info(Session_rep const& s);
    int session_id() const { return m_session_id; }
    std::string const& action() const { return m_action; }
    Sockfd socket_handle() const { return m_socket_handle; }
    bool is_socket_blocking() const { return m_is_socket_blocking; }
    Date created() const { return m_created; }
    std::string const& remote_address() const { return m_remote_address; }
    int remote_port() const { return m_remote_port; }
    int num_bytes_received() const { return m_num_bytes_received; }
    int num_bytes_sent() const { return m_num_bytes_sent; }

  private:
    int const m_session_id;
    std::string const m_action;
    Sockfd const m_socket_handle;
    bool const m_is_socket_blocking;
    Date const m_created;
    std::string const m_remote_address;
    int const m_remote_port;
    int const m_num_bytes_received;
    int const m_num_bytes_sent;
};

// +-------------+
// | Session_rep |
// +-------------+
//
// This class is the primary point for customizing the server framework. It is
// the base class for all client connections, and it provides callback hooks
// through which the derived class can respond to various system events.
//
// Note: Session_rep objects are reference-counted; always use the Session
// proxy class to store heap-allocated Session_rep objects.
class Session_rep : public Thread_safe_reference_counted,
                    public Sink,
                    boost::noncopyable {
  public:
    // Constructs a new session, using an open socket as the connection. This
    // object assumes ownership of the socket pointer.
    Session_rep(Server_interface& server, Socket* socket);

    // Destroys this session and closes its connection.
    virtual ~Session_rep();

    // Returns a reference to the server instance that created this session.
    Server_interface& server() { return m_server; }

    // Returns the socket for this session.
    Socket& socket() { return *m_socket; }

    // Returns the socket for this session.
    Socket const& socket() const { return *m_socket; }

    // Sends the contents of a byte buffer to the client to which this session
    // belongs. This function either writes directly to the session's socket
    // or passes the buffer to an i/o slave process for writing. By default,
    // it does the latter; the default behavior can be controlled by calling
    // Session_rep::use_slave_process_for_output.
    void send(Buffer const& buffer);

    // Specifies whether a slave process should be used to write data to this
    // session's socket. By default, the i/o slave process is used. See
    // Session_rep::send for more details.
    void use_slave_process_for_output(bool b) { m_use_io_slave = b; }

    // Sets the session's current action. The action should be a description
    // of the task currently being performed by the session. For more
    // information, see the documentation for #action.
    void set_action(std::string const& s) { m_action = s; }

    // Returns the session's current action. While it is usually the user's
    // responsibility to set this value using Session_rep::set_action, the
    // framework will automatically set it as follows:
    //
    //   - "initializing" before calling handle_init
    //   - "shutting down" before calling handle_shutdown
    //   - "processing input" before calling handle_input
    //   - "processing" before calling handle_processing
    //   - "idle" after any of the above functions return
    std::string const& action() const { return m_action; }

    // The input callback function. This function is called when the server
    // determines that the session has new input available and has filled the
    // session's input buffer, which is also supplied as a parameter.
    //
    // The input_buffer argument is preserved and modified between calls to
    // this function. Do not save a reference to the input buffer because it
    // is not safe to modify after this function returns.
    //
    // This function must return a boolean indicating the session state: those
    // in an error or shutdown state should return false, which insures that
    // the session will be removed from the system. This provides a direct
    // channel of communication between the session and the server system,
    // which is more efficient that communicating via the the more general
    // server interface. When possible, return false from this function to end
    // the session instead of enqueuing a command to accomplish the same
    // thing.
    //
    // Warning: because the system automatically fills the session input
    // buffer, this function should avoid reading from the socket.
    // Implementations that do so risk consuming excessive server resources.
    // Instead, input should be handled by a "reader" object.
    bool handle_input(Buffer& input_buffer);

    // The deferred processing callback function. This function is called when
    // the server executes a Process_session_command object that was linked to
    // this session; its argument indicates which processor thread has called
    // it. This callback enables session implementors to defer non-CPU-bound
    // processing until it can be done concurrently with other sessions'
    // processing. Overriding this function is optional; it does nothing by
    // default.
    void handle_processing(int pid);

    // The initialization callback function. This function is called after
    // this session is first registered with the system. Overriding this
    // function is optional; it does nothing by default.
    //
    // The input_buffer argument is preserved and modified between calls to
    // this function. Do not save a reference to the input buffer because it
    // is not safe to modify after this function returns.
    void handle_init(Buffer& input_buffer);

    // The shutdown callback function. This function is called before this
    // session is deregistered with the system. It should not attempt to read
    // from or write to the session socket. Instead, it should confine itself
    // to performing necessary per-session cleanup tasks. Overriding this
    // function is optional; it does nothing by default.
    void handle_shutdown();

    // Returns an integer identifying this session. The returned value is
    // guaranteed to be unique among sessions currently in the system.
    int id() const { return m_id; }

    // Returns a string representation of this session. The default format for
    // the string is "ID@ADDRESS:PORT" where ID is the session ID, ADDRESS is
    // the remote address, and PORT is the remote port. Derived classes may
    // override this function.
    virtual std::string to_string() const;

  private:
    virtual bool do_handle_input(Buffer& input_buffer) = 0;
    virtual void do_handle_processing(int pid) {}
    virtual void do_handle_init(Buffer& input_buffer) {}
    virtual void do_handle_shutdown() {}

  private:
    int const m_id;             // session id
    Socket* m_socket;           // client socket
    Server_interface& m_server; // reference to the server interface
    std::string m_action;       // the session's current task
    bool m_use_io_slave;        // specifies whether to use i/o slave process
};

// A shared pointer to a Session_rep instance. Session_rep objects are always
// dynamically allocated, but should never be passed around directly as
// pointers. Instead, they should be wrapped in Session objects.
//
// Assuming they are always wrapped in a stack-allocated Session object,
// Session_rep objects are automatically destroyed when their reference counts
// reach zero. Note that they should never be explicitly deleted.
typedef boost::intrusive_ptr<Session_rep> Session;

// FREE OPERATORS

bool operator==(Session const& a, Session const& b);
bool operator!=(Session const& a, Session const& b);
bool operator< (Session const& a, Session const& b);
bool operator<=(Session const& a, Session const& b);
bool operator> (Session const& a, Session const& b);
bool operator>=(Session const& a, Session const& b);


// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

inline bool operator==(Session const& a, Session const& b)
{
    return a->id() == b->id();
}

inline bool operator!=(Session const& a, Session const& b)
{
    return !(a == b);
}

inline bool operator< (Session const& a, Session const& b)
{
    return a->id() < b->id();
}

inline bool operator<=(Session const& a, Session const& b)
{
    return a->id() <= b->id();
}

inline bool operator> (Session const& a, Session const& b)
{
    return a->id() > b->id();
}

inline bool operator>=(Session const& a, Session const& b)
{
    return a->id() >= b->id();
}

} // namespace ares

#endif
