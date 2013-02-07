// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_server
#define included_ares_server

#include "ares/component.hpp"
#include "ares/server_interface.hpp"

namespace ares {

class Service;

// Represents the server in the ares framework. A typical server program would
// instantiate exactly one of these objects and configure it to accept
// connections for various services using the add_service member function.
class Server : public Server_interface, public Component {
  public:
    Server();
    virtual ~Server();

    // Adds a service to this server. The server assumes ownership of the
    // supplied Service object. If the server is currently running, it will
    // attempt to start a listener for the service immediately; if the
    // listener cannot be started, this function raises an exception and
    // rejects the service (that is, ownership is not transferred).
    void add_service(Service* service);

    // Sets the concurrency level for the execution of Command objects. An
    // application's ideal concurrency level depends heavily on the type of
    // Command objects it creates: if they are mostly cpu-bound, having more
    // Processor objects than hardware cpus is generally not beneficial; if
    // they are mostly i/o-bound, you want as many Processor objects as your
    // system can reasonably support. Throws an Illegal_processor_count_error
    // exception if n is less than zero.
    //
    // Note: If you bounce the server, the number of processors will be reset
    // to the default (one). Avoid setting the number of processors to zero in
    // a running server unless you intend to bounce it.
    void set_num_processors(int n);

    // Returns the number of Processor objects currently managed by this
    // server. See set_num_processors for more information.
    int num_processors() const;

    // (the following functions are inherited from Server_interface; see that
    // class for documentation)
    void add_session(Session s);
    void remove_session(Session s);
    void enqueue_command(Command* c);
    void enqueue_delayed_command(Command* c, int num_seconds);
    void dispatch(Session s, Buffer* bp);
    job::Scheduler& scheduler();
    void shutdown();
    Date started() const;
    int uptime() const;

    void display_statistics();  // FIXME

  private:
    void do_startup();
    void do_shutdown();
    void stop_all_components();
    void shutdown_all_components();
    void run();

  private:
    struct Impl;
    Impl* m_impl;
};

} // namespace ares

#endif
