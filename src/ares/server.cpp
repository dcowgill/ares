// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/server.hpp"
#include "ares/command_queue.hpp"
#include "ares/date.hpp"
#include "ares/dispatcher.hpp"
#include "ares/error.hpp"
#include "ares/guard.hpp"
#include "ares/listener.hpp"
#include "ares/log.hpp"
#include "ares/platform.hpp"
#include "ares/processor.hpp"
#include "ares/receiver.hpp"
#include "ares/service.hpp"
#include "ares/string_util.hpp"
#include "ares/thread.hpp"
#include "ares/trace.hpp"
#include <list>
#include <vector>

using namespace std;
using namespace ares;

// TODO: startup and shutdown need to be restartable, and need to clean up
// after themselves in case of error. IOW we must provide the strong exception
// guarantee for those functions.

typedef list<pair<Service*, Listener*> > Service_list;

// This simple class manages a pool of integer IDs. The IDs it generates begin
// at 0 and increase up to 2^31-1. It always returns the lowest available ID.
// IDs can be returned to the pool of available IDs for subsequent reuse.
class ID_table {
  public:
    // Reserves and returns the lowest available ID.
    int get_id()
    {
        for (int i = 0; i < int(m_slots.size()); i++) {
            if (m_slots[i]) {
                m_slots[i] = false;
                return i;
            }
        }
        m_slots.push_back(false);
        return m_slots.size()-1;
    }

    // Returns an ID to the pool of available IDs.
    void release_id(int id)
    {
        m_slots.at(id) = true;
    }

  private:
    vector<bool> m_slots;
};


struct Server::Impl {
    Command_queue m_queue;              // primary command queue for components
    vector<Processor*> m_processors;    // processor components
    Receiver m_receiver;                // receiver component
    Dispatcher m_dispatcher;            // dispatcher component
    Service_list m_services;            // list of service-listener pairs
    job::Scheduler m_scheduler;         // system job scheduler
    ID_table m_pid_tab;                 // processor ID table
    ID_table m_lid_tab;                 // listener ID table

    Impl(Server_interface& server);
    ~Impl();
};


Server::Impl::Impl(Server_interface& server)
        : m_receiver(server)
        , m_dispatcher(server)
{}

Server::Impl::~Impl()
{
    for (Service_list::iterator i = m_services.begin();
         i != m_services.end(); ++i)
    {
        delete i->first;    // service
        delete i->second;   // listener
    }

    for (int i = 0; i < int(m_processors.size()); i++) {
        delete m_processors[i];
    }
}


Server::Server()
        : Component("server", "", false)
        , m_impl(new Impl(*this))
{}

Server::~Server()
{
    delete m_impl;
}

void Server::add_service(Service* service)
{
    // Create a listener for this service.
    auto_ptr<Listener> listener(
        new Listener(*service, *this, m_impl->m_lid_tab.get_id()));

    // Start the listener only if the server is currently running.
    if (is_active())
        listener->shutdown();

    // All is well; add the service.
    m_impl->m_services.push_back(make_pair(service, listener.release()));
}

void Server::set_num_processors(int n)
{
    if (n < 0) {
        throw Illegal_processor_count_error(n);
    }
    else if (num_processors() < n) {
        while (num_processors() < n) {
            auto_ptr<Processor> p(new Processor(*this, m_impl->m_queue,
                                                m_impl->m_pid_tab.get_id()));
            p->startup();
            m_impl->m_processors.push_back(p.release());
        }
    }
    else if (num_processors() > n) {
        while (num_processors() > n) {
            auto_ptr<Processor> p(m_impl->m_processors.back());
            m_impl->m_processors.pop_back();
            m_impl->m_pid_tab.release_id(p->id());
            p->shutdown();
        }
    }
}

void Server::add_session(Session s)
{
    ARES_TRACE(("adding session [%s]", s->to_string().c_str()));
    m_impl->m_receiver.add_session(s);
}

void Server::remove_session(Session s)
{
    ARES_TRACE(("removing session [%s]", s->to_string().c_str()));
    m_impl->m_receiver.remove_session(s);
}

void Server::enqueue_command(Command* c)
{
    ARES_TRACE(("enqueing command [%p]", c));
    bool success = m_impl->m_queue.enqueue(c);
    assert(success);
}

void Server::enqueue_delayed_command(Command* c, int num_seconds)
{
    // Class is private to this function
    struct Delayed_action : public job::Task {
        Delayed_action(Server_interface& server, Command* c)
                : m_server(server), m_command(c) {}

        void run() { m_server.enqueue_command(m_command); }

        Server_interface& m_server;
        Command* m_command;
    };

    if (num_seconds <= 0)
        m_impl->m_queue.enqueue(c);
    else
        scheduler().submit(new Delayed_action(*this, c),
                           Date::now().add_seconds(num_seconds));
}

void Server::dispatch(Session c, Buffer* bp)
{
    m_impl->m_dispatcher.dispatch(c, bp);
}

ares::job::Scheduler& Server::scheduler()
{
    return m_impl->m_scheduler;
}

void Server::shutdown()
{
    Component::shutdown();
}

void Server::do_startup()
{
    // Create Processor instances.
    set_num_processors(1);

    // Start up our Receiver and Dispatcher.
    m_impl->m_receiver.startup();
    m_impl->m_dispatcher.startup();

    // Create a listener for each registered service.
    for (Service_list::iterator i = m_impl->m_services.begin();
         i != m_impl->m_services.end(); ++i)
    {
        i->second->startup();
    }
}

void Server::do_shutdown()
{
    stop_all_components();
    shutdown_all_components();
}

void Server::stop_all_components()
{
    m_impl->m_receiver.stop();
    m_impl->m_dispatcher.stop();

    for (int i = 0; i < num_processors(); i++)
        m_impl->m_processors[i]->stop();

    for (Service_list::iterator i = m_impl->m_services.begin();
         i != m_impl->m_services.end(); ++i)
    {
        i->second->stop();
    }
}

void Server::shutdown_all_components()
{
    m_impl->m_receiver.shutdown();
    m_impl->m_dispatcher.shutdown();

    set_num_processors(0);

    for (Service_list::iterator i = m_impl->m_services.begin();
         i != m_impl->m_services.end(); ++i)
    {
        i->second->shutdown();
    }
}

void Server::run()
{
    // the server has no independent thread of execution
}

void Server::display_statistics()    // FIXME
{
    fprintf(stderr, "==================================\n\n");

    Receiver_statistics rs = m_impl->m_receiver.statistics();

    fprintf(stderr, "RCVR.interval                    %d s\n", rs.elapsed_sec());
    fprintf(stderr, "RCVR.sessions_snap               %d\n", rs.sessions_snap());
    fprintf(stderr, "RCVR.queued_updates_snap         %d\n", rs.queued_updates_snap());
    fprintf(stderr, "RCVR.reads                       %d (%.2f/s)\n", rs.reads(), rs.reads_per_sec());
    fprintf(stderr, "RCVR.bytes_read                  %d (%.2f/s)\n", rs.bytes_read(), rs.bytes_read_per_sec());
    fprintf(stderr, "RCVR.bytes_per_read              %d\n", rs.bytes_per_read());

    Dispatcher_statistics ds = m_impl->m_dispatcher.statistics();

    fprintf(stderr, "DSPR.interval                    %d s\n", ds.elapsed_sec());
    fprintf(stderr, "DSPR.sessions_snap               %d\n", ds.sessions_snap());
    fprintf(stderr, "DSPR.queued_dispatches_snap      %d\n", ds.queued_dispatches_snap());
    fprintf(stderr, "DSPR.buffers_snap                %d\n", ds.buffers_snap());
    fprintf(stderr, "DSPR.outbound_snap               %d\n", ds.outbound_snap());
    fprintf(stderr, "DSPR.outbound_remaining_snap     %d\n", ds.outbound_remaining_snap());
    fprintf(stderr, "DSPR.writes                      %d (%.2f/s)\n", ds.writes(), ds.writes_per_sec());
    fprintf(stderr, "DSPR.zero_writes                 %d (%.2f/s)\n", ds.zero_writes(), ds.zero_writes_per_sec());
    fprintf(stderr, "DSPR.bytes_sent                  %d (%.2f/s)\n", ds.bytes_sent(), ds.bytes_sent_per_sec());
    fprintf(stderr, "DSPR.bytes_per_write             %d\n", ds.bytes_per_write());
    fprintf(stderr, "DSPR.buffers_added               %d (%.2f/s)\n", ds.buffers_added(), ds.buffers_added_per_sec());
    fprintf(stderr, "DSPR.buffers_sent                %d (%.2f/s)\n", ds.buffers_sent(), ds.buffers_sent_per_sec());

    for (int i = 0; i < int(m_impl->m_processors.size()); i++) {
        Processor_statistics ps = m_impl->m_processors[i]->statistics();
        fprintf(stderr, "PRCR-%03d.interval                %d s\n", i, ps.m_elapsed_sec);
        fprintf(stderr, "PRCR-%03d.commands_executed       %d (%.2f/s)\n", i, ps.m_commands_executed, 1.0*ps.m_commands_executed/ps.m_elapsed_sec);
    }

    fprintf(stderr, "\n");
}

ares::Date Server::started() const
{
    return Component::started();
}

int Server::uptime() const
{
    return Component::uptime();
}

int Server::num_processors() const
{
    return m_impl->m_processors.size();
}
