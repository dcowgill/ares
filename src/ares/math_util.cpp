// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/math_util.hpp"
#include <math.h>

bool ares::is_prime(int v)
{
    // If computing large primes were important to us, we'd use Rabin's
    // algorithm (see below).

    if (v <= 1)
        return false;       // 1, 0, -1, ... are not prime

    if (v%2 == 0)
        return (v == 2);    // 2 is the only even prime

    for (int i = 3, last = int(sqrt(v)); i <= last; i += 2)
        if (v % i == 0)
            return false;

    return true;
}

// This function is brain-dead; caveat emptor
int ares::next_prime(int v)
{
    if (v%2 == 0)
        v++;
    while (!is_prime(v))
        v += 2;
    return v;
}
