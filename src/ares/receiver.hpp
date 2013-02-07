// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_receiver
#define included_ares_receiver

// This is an implementation file; do not use directly.

#include "ares/command_queue.hpp"
#include "ares/component.hpp"
#include "ares/mutex.hpp"
#include "ares/server_interface.hpp"
#include "ares/session.hpp"
#include "ares/shared_queue.hpp"
#include "ares/sockfd_poller.hpp"
#include <map>
#include <vector>

namespace ares {

class Receiver_statistics;

// Receiver is the framework component responsible for reading input from
// sessions and invoking the session input handlers when input is ready. A
// typical server will use only one receiver instance.
class Receiver : public Component {
  public:
    Receiver(Server_interface& server);
    virtual ~Receiver();

    // Requests that a session be added to this receiver. Only one receiver
    // object should manage a session at a time, and while the session is
    // being managed by a receiver, no other threads should attempt to read
    // from the session's socket (concurrent writes are safe). Note that the
    // session may not be immediately added.
    void add_session(Session s);

    // Requests that a session be removed from this receiver. Has no effect if
    // the receiver is not currently managing the given session. Note that the
    // session may not be immediately removed.
    void remove_session(Session s);

    // Returns the number of sessions currently being managed by this
    // receiver. Note that this may not include recent additions or removals.
    int num_sessions() const;

    Receiver_statistics statistics();

  private:
    struct Socket_event_handler : public Sockfd_poller::Event_handler {
        Receiver& m_receiver;   // reference to the parent class
        Session m_session;      // session to associate with events
        Buffer* m_buffer;       // the session input buffer

        Socket_event_handler(Receiver& r, Session s, Buffer* b)
                : m_receiver(r)
                , m_session(s)
                , m_buffer(b)
        {}

        virtual ~Socket_event_handler();
        Action operator()();  // the event handler callback
    };

    typedef std::pair<bool, Session> Pending_update;
    typedef Shared_queue<Pending_update> Update_queue;
    typedef std::vector<Pending_update> Update_array;
    typedef std::map<Sockfd, Socket_event_handler*> Session_map;

  private:
    // Implements Thread::Runnable::run.
    void run();

    // Waits very briefly for an incoming session update.
    void briefly_wait_for_update();

    // Performs the actual work involved in handling an update.
    void process(Pending_update);

    Server_interface& m_server;     // external server interface
    Session_map m_sessions;         // maps sockets to session data
    Sockfd_poller m_poller;         // socket I/O event poller
    Update_queue m_update_queue;    // queued added/removed sessions
    Update_array m_updates;         // for efficient dequeue_all
    mutable Mutex m_lock;           // general sychronization

    // (statistics)
    time_t m_last_snapshot;
    int m_reads;
    int m_bytes_read;

    friend struct Socket_event_handler;
};

// Encapsulates statistics about a Receiver object. Objects of this type are
// created by calling the Receiver::statistics function.
class Receiver_statistics {
  public:
    // The number of seconds since the last call to Receiver::statistics. All
    // rate-based statistics are relative to the time period returned by this
    // function.
    int elapsed_sec() const { return m_elapsed_sec; }

    // A snapshot of the number of sessions managed by the receiver when the
    // statistics were collected.
    int sessions_snap() const { return m_sessions_snap; }

    // A snapshot of the number of queued, unprocessed updates to the receiver
    // when the statistics were collected. Updates are additions and removals
    // of sessions from the system.
    int queued_updates_snap() const { return m_queued_updates_snap; }

    // The number of network read operations executed by the receiver.
    int reads() const { return m_reads; }

    // The mean network reads per second during the statistics window.
    double reads_per_sec() const;

    // The number of bytes read by the receiver.
    int bytes_read() const { return m_bytes_read; }

    // The mean bytes read per second during the statistics window.
    double bytes_read_per_sec() const;

    // The mean bytes read per network read operation during the statistics
    // window.
    int bytes_per_read() const;

  private:
    int m_elapsed_sec;              // seconds since last snapshot
    int m_sessions_snap;            // current number of managed sessions
    int m_queued_updates_snap;      // queued (unprocessed) session updates
    int m_reads;                    // total read operations
    int m_bytes_read;               // total bytes read

    friend class Receiver;
};

} // namespace ares

#endif
