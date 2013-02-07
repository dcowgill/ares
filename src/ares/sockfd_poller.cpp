// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/sockfd_poller.hpp"
#include "ares/error.hpp"
#include "ares/net_tk.hpp"
#include "ares/network_common.hpp"
#include <list>
#include <vector>

#if 0 && defined(HAVE_SYS_EPOLL_H) && defined(HAVE_EPOLL_CREATE)
# include <sys/epoll.h>
# define USE_EPOLL 1
#elif defined(HAVE_POLL)
# define USE_POLL 1
#elif defined(HAVE_SELECT)
# include <sys/time.h>   // for struct timeval
# define USE_SELECT 1
#else
# error "Either epoll, poll, or select is required!"
#endif

using namespace std;
using ares::Sockfd_poller;

struct Sockfd_poller::Impl {
    Impl();
    ~Impl();
    bool add(Sockfd s, Event_type event_type, Event_handler& handler);
    bool remove(Sockfd s);
    int wait_for_event(int millis);
    void process_events();

    struct Sock_info {
        Sockfd m_fd;
        Event_type m_event_type;
        Event_handler* m_handler;
        bool m_on_edge;
        Sock_info() : m_fd(-1) {}
    };
    int m_num_sockets;                  // no. of managed sockets
    vector<Sock_info> m_sockets;        // socket "map", indexed by fd
    int m_num_events;                   // no. of events in last poll

#if defined(USE_EPOLL)
    int m_epfd;                         // epoll file descriptor
    vector<struct epoll_event> m_events;// array of output epoll events
    list<Sockfd> m_edge_list;           // sockets with pending events
#elif defined(USE_POLL)
    vector<struct pollfd> m_poll_vec;   // array of poll structs, 1 per socket
#endif
};

Sockfd_poller::Impl::Impl()
        : m_num_sockets(0)
        , m_num_events(0)
#if defined(USE_EPOLL)
        , m_epfd(-1)
#endif
{
#if defined(USE_EPOLL)
    if ((m_epfd = epoll_create(100)) < 0)
        throw Network_io_error("epoll_create", errno);
#endif
}

Sockfd_poller::Impl::~Impl()
{
#if defined(USE_EPOLL)
    close(m_epfd);
#endif
}

bool Sockfd_poller::Impl::add(Sockfd s, Event_type event_type,
                              Event_handler& handler)
{
    if (s == net_tk::null_socket())
        throw Invalid_socket_error(int(s));

    if (s >= int(m_sockets.size()))
        m_sockets.resize(s + 1);    // expand socket map if necessary
    else if (m_sockets[s].m_fd >= 0)
        return false;               // this socket was already added

#if defined(USE_EPOLL)
    // Add the socket to the kernel queue.
    struct epoll_event evt;
    evt.data.fd = s;
    evt.events = (event_type == Sockfd_poller::EVENT_READABLE)
                 ? EPOLLET | EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP
                 : EPOLLET | EPOLLOUT;

    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, s, &evt) < 0)
        throw Network_io_error("epoll_ctl", errno);

    // Expand the event vector (passed to epoll_wait) if necessary.
    if (m_num_sockets > int(m_events.size()))
        m_events.resize(m_num_sockets);
#elif defined(USE_POLL)
    // Add the socket to the kernel queue.
    int events = (event_type == Sockfd_poller::EVENT_READABLE)
                 ? POLLIN | POLLPRI | POLLERR | POLLHUP
                 : POLLOUT;

    if (m_num_sockets >= int(m_poll_vec.size())) {  // make room for socket
        struct pollfd p;
        p.fd = s;
        p.events = events;
        m_poll_vec.push_back(p);
    }
    else {                                          // vector already has room
        m_poll_vec.at(m_num_sockets).fd = s;
        m_poll_vec.at(m_num_sockets).events = events;
    }
#endif

    // Add the socket to our data structure.
    m_sockets[s].m_fd = s;
    m_sockets[s].m_event_type = event_type;
    m_sockets[s].m_handler = &handler;
    m_sockets[s].m_on_edge = false;
    m_num_sockets++;

    return true;
}

bool Sockfd_poller::Impl::remove(Sockfd s)
{
    if (s >= int(m_sockets.size()) || m_sockets[s].m_fd < 0)
        return false;  // we don't own this socket

    // Remove the socket from our data structure.
    assert(m_sockets[s].m_fd == s);
    m_sockets[s].m_fd = -1;
    m_num_sockets--;

#if defined(USE_EPOLL)
    // If the socket was on the edge list, remove it.
    if (m_sockets[s].m_on_edge)
        m_edge_list.remove(s);

    // Remove the socket from the kernel queue.
    struct epoll_event evt;
    evt.data.fd = s;
    evt.events = 0;
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, s, &evt) < 0)
        throw Network_io_error("epoll_ctl", errno);
    return true;
#elif defined(USE_POLL)
    // Find the socket to remove and swap in the last element to fill the gap.
    // Note that we must use m_num_sockets (and not m_num_sockets-1) to find
    // the last element because m_num_sockets was already decremented above!

    for (int i = 0, n = m_poll_vec.size(); i < n; i++)
        if (m_poll_vec[i].fd == s) {
            m_poll_vec[i].fd = m_poll_vec[m_num_sockets].fd;
            m_poll_vec[i].events = m_poll_vec[m_num_sockets].events;
            return true;
        }

    ARES_PANIC(("expected to find socket fd %d", s));
    return false;  // not reached
#endif
}

int Sockfd_poller::Impl::wait_for_event(int millis)
{
    if (m_num_sockets == 0)
        return 0;

    if (millis < -1)
        millis = -1;

#if defined(USE_EPOLL)
    // Call epoll_wait to get the list of socket events.
    m_num_events = epoll_wait(m_epfd, &m_events[0], m_num_sockets, millis);
    if (m_num_events < 0)
        throw Network_io_error("epoll", errno);
#elif defined(USE_POLL)
    // Call poll to get the count of "interesting" sockets.
    m_num_events = poll(&m_poll_vec[0], m_num_sockets, millis);
    if (m_num_events < 0)
        throw Network_io_error("poll", errno);
#endif
    return m_num_events;
}

void Sockfd_poller::Impl::process_events()
{
#if defined(USE_EPOLL)
    // Invoke the appropriate handler for each new event.
    while (--m_num_events >= 0) {
        Sockfd s = m_events[m_num_events].data.fd;
        if (m_sockets[s].m_on_edge)
            continue;  // this shouldn't happen

        Event_handler::Action const action = (*m_sockets[s].m_handler)();

        if (action == Event_handler::KEEP_EVENT) {
            m_edge_list.push_front(s);
            m_sockets[s].m_on_edge = true;
        }
        else if (action == Event_handler::REMOVE_SOCKET)
            remove(s);
        // else DISCARD_EVENT -> do nothing
    }

    // After handling new events, send "kept" events to handlers.
    for (list<Sockfd>::iterator i(m_edge_list.begin()),
                 end(m_edge_list.end()); i != end; )
    {
        Event_handler::Action const action = (*m_sockets[*i].m_handler)();

        if (action ==  Event_handler::DISCARD_EVENT) {
            Sockfd s = *i;
            i = m_edge_list.erase(i);
            m_sockets[s].m_on_edge = false;
        }
        else if (action == Event_handler::REMOVE_SOCKET) {
            Sockfd s = *i;
            i = m_edge_list.erase(i);
            remove(s);
        }
        else    // KEEP_EVENT
            ++i;
    }
#elif defined(USE_POLL)
    // Search for the ready sockets and invoke their event handlers.
    for (int i = 0; (m_num_events > 0) && (i < m_num_sockets); i++) {
        if (m_poll_vec[i].revents != NO_EVENT) {
            Sockfd const s = m_poll_vec[i].fd;
            Event_handler::Action const action = (*m_sockets[s].m_handler)();

            // Because poll(2) is a level-triggered polling interface, we
            // always discard events (because they will recur in the next
            // poll). Thus, we only recognize the REMOVE_SOCKET action.

            if (action == Event_handler::REMOVE_SOCKET)
                remove(s);

            --m_num_events;
        }
    }
#endif
}


Sockfd_poller::Sockfd_poller()
        : m_impl(new Impl)
{}

Sockfd_poller::~Sockfd_poller()
{
    delete m_impl;
}

bool Sockfd_poller::add(Sockfd socket, Event_type event_type,
                        Event_handler& handler)
{
    return m_impl->add(socket, event_type, handler);
}

bool Sockfd_poller::remove(Sockfd socket)
{
    return m_impl->remove(socket);
}

int Sockfd_poller::wait_for_event(int millis)
{
    return m_impl->wait_for_event(millis);
}

void Sockfd_poller::process_events()
{
    m_impl->process_events();
}

int Sockfd_poller::num_sockets() const
{
    return m_impl->m_num_sockets;
}
