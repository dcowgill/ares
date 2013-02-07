// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/service.hpp"
#include "ares/listener_strategy.hpp"

using ares::Service;

Service::Service(std::string const& name,
                 std::string const& address,
                 std::string const& port,
                 Listener_strategy* strategy)
        : m_name(name)
        , m_address(address)
        , m_port(port)
        , m_strategy(strategy)
{}

Service::~Service()
{
    delete m_strategy;
}
