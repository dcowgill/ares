// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_sockfd_selector
#define included_ares_sockfd_selector

#include "ares/types.hpp"
#include "ares/utility.hpp"
#include <vector>

namespace ares {

class Sockfd_selector : boost::noncopyable {
  public:
    typedef std::pair<Sockfd, char> Event;

    static int const WAIT_FOREVER;
    static int const EVENT_READ;
    static int const EVENT_WRITE;

    Sockfd_selector();
    ~Sockfd_selector();
    void watch(Sockfd s, int events);
    bool unwatch(Sockfd s, int events = 0);
    void wait_for_event(int millis, std::vector<Event>& set);
    int num_sockets() const;

  private:
    struct Impl;
    Impl* m_impl;
};

} // namespace ares

#endif
