// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_command_queue
#define included_ares_command_queue

#include "ares/shared_queue.hpp"

namespace ares {

// forward declaration
class Command;

typedef Shared_queue<Command*> Command_queue;

} // namespace ares

#endif
