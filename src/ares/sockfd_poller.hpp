// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_sockfd_poller
#define included_ares_sockfd_poller

#include "ares/types.hpp"
#include "ares/utility.hpp"

namespace ares {

// An edge-triggered socket-i/o event polling facility. This class manages a
// set of sockets, watching for a particular type of event on each one, and
// invokes an event handler callback function (associated with the socket when
// it is added to the poller) every time the event is raised. Because the
// poller is edge-triggered, as opposed to level-triggered, it is the
// responsibility of the event handler to inform the poller whether the event
// was completely handled, or whether the handler will need to be invoked
// again the next time the system polls for events.
class Sockfd_poller : boost::noncopyable{
  public:
    // This callback object is used to handle all kinds of i/o events.
    struct Event_handler {
        enum Action {
            KEEP_EVENT,     // event was not fully handled
            DISCARD_EVENT,  // event was handled; delete it
            REMOVE_SOCKET,  // remove the socket that created the event
        };

        virtual ~Event_handler() {}

        // This function is called by Sockfd_poller whenever the right kind of
        // event occurs on the socket with which this Event_handler was
        // associated. It must return an Action indicating what the
        // Sockfd_poller should do with the event. There are three possible
        // return values: KEEP_EVENT, DISCARD_EVENT, and REMOVE_SOCKET.
        //
        // KEEP_EVENT indicates that the event hasn't been completely handled
        // and should still be considered active. As a result, this handler
        // will be invoked the next time the Sockfd_poller polls for events.
        //
        // DISCARD_EVENT indicates that the event was completely handled.
        //
        // REMOVE_SOCKET indicates that the socket associated with the event
        // should be removed immediately and the event discarded.
        virtual Action operator()() = 0;
    };

    enum Event_type {
        EVENT_WRITABLE = 1, // socket can be written to
        EVENT_READABLE = 2, // socket has incoming data
    };

    Sockfd_poller();
    ~Sockfd_poller();

    // Adds the socket s to this event multiplexer, specifying which event to
    // watch for. Note that only one type of event can be watched. Whenever
    // that event is triggered, the handler is invoked. Fails if the socket
    // was already added. Returns true on success.
    bool add(Sockfd s, Event_type event_type, Event_handler& handler);

    // Removes the socket s from this event multiplexer. Returns true if the
    // socket was removed, or false if it didn't exist.
    bool remove(Sockfd s);

    // Waits up to the specified number of milliseconds for an event to occur.
    // Returns the number of events that occurred. Specifying a negative value
    // for millis indicates that the function should wait indefinitely. Note
    // that this function doesn't cause any event handlers to be invoked; the
    // process_events function should be called whenever this function returns
    // a positive value.
    int wait_for_event(int millis);

    // Processes any events discovered by the most recent call to
    // wait_for_event, invoking the appropriate event handler for each one.
    void process_events();

    // Returns the number of sockets managed by this object.
    int num_sockets() const;

  private:
    struct Impl;
    Impl* m_impl;
};

} // namespace ares

#endif
