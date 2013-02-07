// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_math_util
#define included_ares_math_util

namespace ares {

// Tests if a number has no factors other than 1 and itself.
bool is_prime(int v);

// Returns the smallest prime number that is greater than or equal to v and
// less than 2^31-1.
int next_prime(int v);

} // namespace ares

#endif
