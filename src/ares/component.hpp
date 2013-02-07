// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_component
#define included_ares_component

// This is an implementation file; do not use directly.

#include "ares/date.hpp"
#include "ares/thread.hpp"
#include <string>

namespace ares {

// Represents an active server component. All components can be started and
// shut down, remember their startup time, and can compute their uptime. Each
// class derived from Component contains an independent thread of execution
// that executes the run function inhereted from Thread::Runnable (unless
// otherwise specified; see the constructor function below).
class Component : public Thread::Runnable {
  public:
    // Constructs the component, given its name. The name is used by the
    // component to identify itself in log messages. The caller may optionally
    // provide a more specific identifying string in addition to the name.
    // Typically, each component represents an independent thread of execution
    // (the component itself provides the thread's "main" function), but the
    // thread can be disabled by setting is_thread to false.
    Component(std::string name, std::string id = "", bool is_thread = true);

    // Destructor. Note that it does not call Component::shutdown, though it
    // will log a warning message if it was not shutdown beforehand.
    virtual ~Component();

    // Starts this component. May throw an arbitrary exception on failure.
    // This method provides the strong exception guarantee.
    void startup();

    // Shuts down this component. May throw an arbitrary exception on failure.
    // This method provides the strong exception guarantee.
    void shutdown();

    // Signals this component to initiate a graceful shutdown. This function
    // is useful for starting the shutdown process for several components
    // without waiting for any one of them to shut down completely; therefore,
    // this function will not throw an exception or block waiting for the
    // component to shut down. In order to actually shut down the component,
    // the shutdown function must still be called, and calling this function
    // prior to shutdown is completely optional.
    void stop();

    // Returns true if startup was called successfully, and the component has
    // not been shut down since.
    bool is_active() const { return m_active; }

    // Returns true if shutdown was called successfully, and the component has
    // not be started since.
    bool is_idle() const { return !is_active(); }

    // Returns the date when startup was last called successfully.
    Date started() const { return m_started; }

    // Returns the number of seconds since this component was last
    // successfully started. If the component is not active, returns zero.
    int uptime() const;

  protected:
    // Returns true if this component's internal thread has been stopped
    // (which doesn't imply that it has exited). Derived classes that
    // implement the run method inhereted from Thread::Runnable should use
    // this method to test whether the thread should exit.
    bool is_stopped() const { return m_stopped; }

  private:
    // Performs startup tasks specific to the derived class. By default, this
    // method does nothing. It must throw an exception to indicate that an
    // error has occurred, but it should also provide the strong exception
    // guarantee in case of failure.
    virtual void do_startup() {}

    // Performs shutdown and cleanup tasks specific to the derived class. By
    // default, this method does nothing. It must throw an exception to
    // indicate that an error has occurred, but it should also provide the
    // strong exception guarantee in case of failure.
    virtual void do_shutdown() {}

    // Returns the number of milliseconds to wait for the component's internal
    // thread to exit when shutting down. The default wait time is several
    // seconds. If a derived class expects its internal thread to take longer
    // to exit, it may override this function to return a larger value.
    virtual int thread_wait_time() const;

  private:
    std::string const m_name;   // component name
    std::string const m_id;     // component id (optional)
    bool const m_is_thread;     // does this component require its own thread?
    bool m_active;              // status of this component
    Thread m_thread;            // internal thread of execution
    bool m_stopped;             // true if thread has been stopped
    Date m_started;             // date component was started
};

} // namespace ares

#endif
