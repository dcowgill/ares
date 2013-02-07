// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"
#include "ares/error.hpp"
#include "ares/random.hpp"
#include <cstdlib>
#include <ctime>

int main(int argc, char** argv) try
{
    using namespace std;

    // Seed the random number generator (used by some tests).
    ares::Random::seed(argc > 1 ? atoi(argv[1]) : time(0));

    // Register and run the unit tests.
    CppUnit::TestFactoryRegistry &registry =
            CppUnit::TestFactoryRegistry::getRegistry();
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(registry.makeTest());
    runner.run();
}
catch (ares::Exception& e) {
    fprintf(stderr, "\nERROR at %s:%d\n  in %s:\n%s\n",
            __FILE__, __LINE__, __PRETTY_FUNCTION__,
            e.to_string().c_str());
}
