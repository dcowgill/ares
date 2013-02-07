// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_dispatcher
#define included_ares_dispatcher

// This is an implementation file; do not use directly.

#include "ares/buffer.hpp"
#include "ares/component.hpp"
#include "ares/mutex.hpp"
#include "ares/session.hpp"
#include "ares/shared_queue.hpp"
#include <list>
#include <map>
#include <vector>

namespace ares {

// TODO: replace map+list with a customized data structure that is more
// appropriate for our usage patterns

class Dispatcher_statistics;

class Dispatcher : public Component {
  public:
    Dispatcher(Server_interface& server);
    virtual ~Dispatcher();
    void dispatch(Session s, Buffer* bp);
    void cancel_dispatches(Session s);
    Dispatcher_statistics statistics();

  private:
    typedef std::pair<Session, Shared_buffer> Pending_dispatch;
    typedef Shared_queue<Pending_dispatch> Dispatch_queue;
    typedef std::vector<Pending_dispatch> Dispatch_array;
    typedef std::pair<int, Shared_buffer> Dispatch;
    typedef std::list<Dispatch> Dispatch_list;
    typedef std::map<Session, Dispatch_list> Session_map;

  private:
    void run();
    void add_dispatch(Pending_dispatch);
    void write_dispatches(Session_map::iterator& it);

  private:
    Server_interface& m_server;     // external server interface
    Session_map m_sessions;         // maps sockets to session data
    Dispatch_queue m_dispatch_queue;// queue of pending dispatches
    Dispatch_array m_dispatches;    // for efficient dequeue_all
    Mutex m_lock;                   // general sychronization

    // (statistics)
    time_t m_last_snapshot;         // time of last snapshot
    int m_num_buffers;              // current number of buffers
    int m_total_output_bytes;       // total size of data in pending buffers
    int m_total_output_bytes_left;  // total size of unsent data in buffers
    int m_buffers_added;            // outgoing buffers added
    int m_buffers_sent;             // outgoing buffers sent
    int m_writes;                   // network writes
    int m_zero_writes;              // number of failed writes
    int m_bytes_sent;               // total bytes read
};

class Dispatcher_statistics {
  public:
    int elapsed_sec() const { return m_elapsed_sec; }
    int sessions_snap() const { return m_sessions_snap; }
    int queued_dispatches_snap() const { return m_queued_dispatches_snap; }
    int buffers_snap() const { return m_buffers_snap; }
    int outbound_snap() const { return m_outbound_snap; }
    int outbound_remaining_snap() const { return m_outbound_remaining_snap; }
    int writes() const { return m_writes; }
    double writes_per_sec() const;
    int zero_writes() const { return m_zero_writes; }
    double zero_writes_per_sec() const;
    int bytes_sent() const { return m_bytes_sent; }
    double bytes_sent_per_sec() const;
    int bytes_per_write() const;
    int buffers_added() const { return m_buffers_added; }
    double buffers_added_per_sec() const;
    int buffers_sent() const { return m_buffers_sent; }
    double buffers_sent_per_sec() const;

  private:
    int m_elapsed_sec;              // seconds since last snapshot
    int m_sessions_snap;            // current number of managed sessions
    int m_queued_dispatches_snap;   // queued (unprocessed) dispatches
    int m_buffers_snap;             // pending output buffers
    int m_outbound_snap;            // total outbound bytes pending
    int m_outbound_remaining_snap;  // unsent outbound bytes pending
    int m_writes;                   // total write operations
    int m_zero_writes;              // total zero-byte write operations
    int m_bytes_sent;               // total bytes sent
    int m_buffers_added;            // outgoing buffers added
    int m_buffers_sent;             // outgoing buffers sent

    friend class Dispatcher;
};

} // namespace ares

#endif
