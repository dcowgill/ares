// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/sockfd_selector.hpp"
#include "ares/error.hpp"
#include "ares/net_tk.hpp"
#include "ares/network_common.hpp"

// Choose which file descriptor-polling facility to use on this operating
// system. The following are supported: epoll, poll, and select. The best
// of the available options is chosen, where "best" means whichever comes
// first in the following list:
//
//      - kqueue (BSD only; not yet implemented)
//      - epoll
//      - poll
//      - select
//
// Note: epoll is used as a level-triggered (*not* edge-triggered) polling
// mechanism, which means it is semantically similar to poll and select.

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
using ares::Sockfd_selector;


struct Sockfd_selector::Impl {
    Impl();
    ~Impl();
    void watch(Sockfd s, int event);
    bool unwatch(Sockfd s, int event);
    void wait_for_event(int millis, vector<Sockfd_selector::Event>& set);

#if defined(USE_EPOLL)
    struct Sock_pair {
        int m_fd;           // -1 when uninitialized, else file descriptor
        int m_events;       // event bit mask
        Sock_pair() : m_fd(-1), m_events(0) {}
    };
    int m_epfd;                         // epoll file descriptor
    int m_num_sockets;                  // number of managed sockets
    vector<Sock_pair> m_sockets;        // socket "map", indexed by fd
    vector<struct epoll_event> m_events;// array of output epoll events
#elif defined(USE_POLL)
    vector<struct pollfd> m_poll_vec;   // array of poll structs, 1 per socket
    int m_last_index;                   // greatest filled index in poll_vec
#elif defined(USE_SELECT)
    fd_set m_rfdset;                    // sockets watched for reading
    fd_set m_wfdset;                    // sockets watched for writing
    fd_set m_temp_rfdset;               // temp. fd_set modified by select
    fd_set m_temp_wfdset;               // temp. fd_set modified by select
    int m_max_fd;                       // max file descriptor in either set
    int m_num_sockets;                  // number of managed sockets
#endif
};

Sockfd_selector::Impl::Impl()
#if defined(USE_EPOLL)
        : m_epfd(-1)
        , m_num_sockets(0)
#elif defined(USE_POLL)
          : m_last_index(-1)
#elif defined(USE_SELECT)
          : m_max_fd(-1)
        , m_num_sockets(0)
#endif
{
#if defined(USE_EPOLL)
    if ((m_epfd = epoll_create(100)) < 0) {
        throw Network_io_error("epoll_create", errno);
    }
#elif defined(USE_SELECT)
    FD_ZERO(&m_rfdset);
    FD_ZERO(&m_wfdset);
#endif
}

Sockfd_selector::Impl::~Impl()
{
#if defined(USE_EPOLL)
    close(m_epfd);
#endif
}

void Sockfd_selector::Impl::watch(Sockfd s, int events)
{
    if (s == net_tk::null_socket())
        throw Invalid_socket_error(int(s));

#if defined(USE_EPOLL)
    int op = EPOLL_CTL_MOD;
    if (s >= int(m_sockets.size())) {
        m_sockets.resize(s + 1);
        op = EPOLL_CTL_ADD;
    }
    else if (m_sockets[s].m_fd < 0) {
        op = EPOLL_CTL_ADD;
    }

    struct epoll_event evt;
    evt.data.fd = s;
    evt.events = events;

    if (epoll_ctl(m_epfd, op, s, &evt) < 0) {
        throw Network_io_error("epoll_ctl", errno);
    }

    if (op == EPOLL_CTL_ADD) {
        m_sockets[s].m_fd = s;
        m_sockets[s].m_events = events;
        m_num_sockets++;
        if (m_num_sockets > int(m_events.size())) {
            m_events.resize(m_num_sockets);
        }
    }
    else {
        assert(m_sockets[s].m_fd == s);
        m_sockets[s].m_events |= events;
    }
#elif defined(USE_POLL)
    // Search for the socket in our existing poll structures.
    for (int i = 0; i <= m_last_index; i++) {
        if (m_poll_vec[i].fd == s) {
            m_poll_vec[i].events |= events;
            return;
        }
    }

    // Not found: add the socket to the poll vector.
    m_last_index++;
    if (m_last_index >= int(m_poll_vec.size())) {   // make room for socket
        struct pollfd p;
        p.fd = s;
        p.events = events;
        m_poll_vec.push_back(p);
    }
    else {                                          // vector already has room
        m_poll_vec[m_last_index].fd = s;
        m_poll_vec[m_last_index].events = events;
    }
#elif defined(USE_SELECT)
    if (s > FD_SETSIZE) {
        throw Socket_not_selectable_error(int(s));
    }

    bool was_added = false;

    if ((events & Sockfd_selector::EVENT_READ) &&
        (events & Sockfd_selector::EVENT_WRITE))
    {
        if (!FD_ISSET(s, &m_rfdset) || !FD_ISSET(s, &m_wfdset)) {
            was_added = true;
        }
        FD_SET(s, &m_rfdset);
        FD_SET(s, &m_wfdset);
    }
    else {
        fd_set* watch_set;  // the set to which we are adding a socket
        fd_set* other_set;  // the other set

        if (events & Sockfd_selector::EVENT_READ) {
            watch_set = &m_rfdset;
            other_set = &m_wfdset;
        }
        else {
            watch_set = &m_wfdset;
            other_set = &m_rfdset;
        }

        if (!FD_ISSET(s, watch_set)) {
            FD_SET(s, watch_set);
            if (!FD_ISSET(s, other_set)) {
                was_added = true;
            }
        }
    }

    if (was_added) {
        if (m_max_fd < s) {
            m_max_fd = s;
        }
        ++m_num_sockets;
    }
#endif
}

bool Sockfd_selector::Impl::unwatch(Sockfd s, int events)
{
    if (s == net_tk::null_socket())
        throw Invalid_socket_error(int(s));

    if (!events)
        events = Sockfd_selector::EVENT_READ | Sockfd_selector::EVENT_WRITE;

#if defined(USE_EPOLL)
    if (s >= int(m_sockets.size()) || m_sockets[s].m_fd < 0) {
        return false;  // we don't own this socket
    }
    assert(m_sockets[s].m_fd == s);
    m_sockets[s].m_events &= ~events;  // turn off the event bits

    int op = EPOLL_CTL_MOD;
    if (m_sockets[s].m_events == 0) {
        op = EPOLL_CTL_DEL;
        m_sockets[s].m_fd = -1;
        m_num_sockets--;
    }

    struct epoll_event evt;
    evt.data.fd = s;
    evt.events = m_sockets[s].m_events;

    if (epoll_ctl(m_epfd, op, s, &evt) < 0) {
        throw Network_io_error("epoll_ctl", errno);
    }

    return true;
#elif defined(USE_POLL)
    // Search for socket to remove.
    for (int i = 0; i <= m_last_index; i++) {
        if (m_poll_vec[i].fd == s) {
            if ((m_poll_vec[i].events &= ~events) != 0) {
                return false;
            }
            m_poll_vec[i].fd = m_poll_vec[m_last_index].fd;
            m_poll_vec[i].events = m_poll_vec[m_last_index].events;
            m_last_index--;
            return true;
        }
    }
#elif defined(USE_SELECT)
    if (s > FD_SETSIZE) {
        throw Socket_not_selectable_error(int(s));
    }

    bool was_removed = false;

    if ((events & Sockfd_selector::EVENT_READ) &&
        (events & Sockfd_selector::EVENT_WRITE))
    {
        if (FD_ISSET(s, &m_rfdset) || FD_ISSET(s, &m_wfdset)) {
            FD_CLR(s, &m_rfdset);
            FD_CLR(s, &m_wfdset);
            was_removed = true;
        }
    }
    else {
        fd_set* watch_set;  // the set to which we are adding a socket
        fd_set* other_set;  // the other set

        if (events & Sockfd_selector::EVENT_READ) {
            watch_set = &m_rfdset;
            other_set = &m_wfdset;
        }
        else {
            watch_set = &m_wfdset;
            other_set = &m_rfdset;
        }

        if (FD_ISSET(s, watch_set)) {
            FD_CLR(s, watch_set);
            if (!FD_ISSET(s, other_set)) {
                was_removed = true;
            }
        }
    }

    if (was_removed) {
        if (s == m_max_fd) {
            while (--m_max_fd >= 0) {
                if (FD_ISSET(m_max_fd, &m_rfdset) ||
                    FD_ISSET(m_max_fd, &m_wfdset))
                {
                    break;
                }
            }
        }
        --m_num_sockets;
        return true;
    }
#endif

    return false;   // not removed
}

void Sockfd_selector::Impl::wait_for_event(int millis,
                                           vector<Sockfd_selector::Event>& set)
{
    set.resize(0);  // fast; doesn't reallocate

#if defined(USE_EPOLL)
    if (m_num_sockets == 0) {
        return;
    }
    if (millis < -1) {
        millis = -1;
    }

    // Call epoll_wait to get the count of "interesting" sockets.
    int count = epoll_wait(m_epfd, &m_events[0], m_num_sockets, millis);
    if (count < 0)
        throw Network_io_error("epoll", errno);

    // Search for the ready sockets and store them in the vector.
    for (int i = 0; i < count; i++) {
        set.push_back(make_pair(m_events[i].data.fd, m_events[i].events));
    }
#elif defined(USE_POLL)
    if (m_last_index < 0) {
        return;
    }

    // Call poll to get the count of "interesting" sockets.
    int count = poll(&m_poll_vec[0], m_last_index + 1, millis);
    if (count < 0)
        throw Network_io_error("poll", errno);

    // Search for the ready sockets and store them in the vector.
    for (int i = 0; (count > 0) && (i <= m_last_index); i++) {
        if (m_poll_vec[i].revents != NO_EVENT) {
            set.push_back(make_pair(m_poll_vec[i].fd, m_poll_vec[i].revents));
            --count;
        }
    }
#elif defined(USE_SELECT)
    if (m_max_fd < 0) {
        return;
    }

    // Copy the fd_sets because select modifies the sets passed to it.
    m_temp_rfdset = m_rfdset;
    m_temp_wfdset = m_wfdset;

    int count;      // the count of ready sockets

    if (millis < 0) {
        // Poll forever (don't need a timeval struct).
        count = select(m_max_fd + 1, &m_temp_rfdset, &m_temp_wfdset, 0, 0);
    }
    else {
        // Convert milliseconds to seconds and microseconds.
        struct timeval tv;
        tv.tv_sec = millis / 1000;
        tv.tv_usec = (millis - (tv.tv_sec * 1000)) * 1000;
        count = select(m_max_fd + 1, &m_temp_rfdset, &m_temp_wfdset, 0, &tv);
    }

    if (count < 0) {
        throw Network_io_error("select", errno);
    }

    // Search for the ready sockets and store them in the vector.
    for (int i = 0; (count > 0) && (i <= m_max_fd); i++) {
        int events = 0;

        if (FD_ISSET(i, &m_temp_rfdset))
            events |= Sockfd_selector::EVENT_READ;
        if (FD_ISSET(i, &m_temp_wfdset))
            events |= Sockfd_selector::EVENT_WRITE;

        if (events) {
            set.push_back(make_pair(i, events));
            --count;
        }
    }
#endif
}

//
// Sockfd_selector
//

int const Sockfd_selector::WAIT_FOREVER  = -1;

#if defined(USE_EPOLL)
int const Sockfd_selector::EVENT_READ  = EPOLLIN|EPOLLPRI|EPOLLERR|EPOLLHUP;
int const Sockfd_selector::EVENT_WRITE = EPOLLOUT;
#elif defined(USE_POLL)
int const Sockfd_selector::EVENT_READ  = POLLIN|POLLPRI|POLLERR|POLLHUP;
int const Sockfd_selector::EVENT_WRITE = POLLOUT;
#elif defined(USE_SELECT)
int const Sockfd_selector::EVENT_READ  = 1;
int const Sockfd_selector::EVENT_WRITE = 2;
#endif

Sockfd_selector::Sockfd_selector()
        : m_impl(new Impl)
{}

Sockfd_selector::~Sockfd_selector()
{
    delete m_impl;
}

void Sockfd_selector::watch(Sockfd s, int event)
{
    m_impl->watch(s, event);
}

bool Sockfd_selector::unwatch(Sockfd s, int event)
{
    return m_impl->unwatch(s, event);
}

void Sockfd_selector::wait_for_event(int millis, vector<Event>& set)
{
    m_impl->wait_for_event(millis, set);
}

int Sockfd_selector::num_sockets() const
{
#if defined(USE_EPOLL)
    return m_impl->m_num_sockets;
#elif defined(USE_POLL)
    return m_impl->m_last_index + 1;
#elif defined(USE_SELECT)
    return m_impl->m_num_sockets;
#endif
}
