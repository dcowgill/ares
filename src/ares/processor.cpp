// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/command.hpp"
#include "ares/error.hpp"
#include "ares/log.hpp"
#include "ares/platform.hpp"
#include "ares/processor.hpp"
#include "ares/socket.hpp"
#include "ares/string_util.hpp"
#include "ares/trace.hpp"
#include <memory>

using namespace std;
using ares::Processor;

Processor::Processor(Server_interface& server,
                     Command_queue& queue,
                     int id)
        : Component("processor", boost::lexical_cast<string>(id))
        , m_server(server)
        , m_queue(queue)
        , m_id(id)
        , m_last_snapshot(current_time())
        , m_commands_executed(0)
{}

Processor::~Processor()
{}

ares::Processor_statistics Processor::statistics()
{
    Processor_statistics stats;

    time_t current_time = ares::current_time();
    stats.m_elapsed_sec = current_time - m_last_snapshot;
    m_last_snapshot = current_time;

    stats.m_commands_executed = m_commands_executed;

    m_commands_executed = 0;

    return stats;
}

// (This ugly macro is used to simplify an ARES_TRACE statement below)
#define COMMAND_SESSION_NAME(p)                                         \
    (dynamic_cast<Session_command*>(p)                                  \
     ? dynamic_cast<Session_command*>(p)->session()->to_string().c_str() \
     : "none")

void Processor::run()
{
    int const DEQUEUE_TIMEOUT = 200;

    Trace::set_thread_name(format("processor_%d", m_id).c_str());

    while (!is_stopped()) {
        try {
            Command* cmdp;
            if (m_queue.dequeue(cmdp, DEQUEUE_TIMEOUT)) {
                auto_ptr<Command> cmd(cmdp);    // insure cleanup
                ARES_TRACE(("processing command [%p] for [%s]",
                            cmdp, COMMAND_SESSION_NAME(cmdp)));
                cmdp->execute(m_server, m_id);
                ARES_TRACE(("finished processing command"));
                m_commands_executed++;
            }
        }
        catch (Exception& e) {
            Log::writef(Log::WARNING, "processor (%d): unhandled exception: %s",
                        m_id, e.to_string().c_str());
        }
        catch (...) {
            Log::writef(Log::WARNING, "processor (%d): unhandled exception", m_id);
        }
    }
}

#undef COMMAND_SESSION_NAME
