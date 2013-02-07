// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/utility.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace std;

namespace
{
void panic_printf(char const* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fputc('\n', stderr);
    abort();
}
}

void (*ares::panic(char const* file,
                   int line,
                   char const* func))(char const*, ...)
{
    fprintf(stderr, "PANIC: %s:%d: %s: ", file, line, func);
    return panic_printf;
}
