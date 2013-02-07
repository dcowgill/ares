// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/random.hpp"
#include <cassert>
#include <cstdlib>

using namespace std;
using ares::Random;

void Random::seed(int n)
{
    srandom(unsigned(n));
}

bool Random::next_bool()
{
    return random() >= (RAND_MAX+1)/2;
}

double Random::next_double(double n)
{
    return n * random()/(RAND_MAX+1.0);
}
