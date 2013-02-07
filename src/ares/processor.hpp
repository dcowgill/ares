// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_processor
#define included_ares_processor

// This is an implementation file; do not use directly.

#include "ares/command_queue.hpp"
#include "ares/component.hpp"

namespace ares {

struct Processor_statistics;
class Server_interface;

class Processor : public Component {
  public:
    Processor(Server_interface& server, Command_queue& queue, int id);
    ~Processor();
    Processor_statistics statistics();
    int id() const { return m_id; }

  private:
    void run();

    Server_interface& m_server;     // server context to pass to commands
    Command_queue& m_queue;         // shared command queue
    int const m_id;                 // unique ID assigned to this processor

    // (for statistics)
    time_t m_last_snapshot;
    int m_commands_executed;
};

struct Processor_statistics {
    int m_elapsed_sec;          // seconds since last snapshot
    int m_commands_executed;    // number of commands executed
};

} // namespace ares

#endif
