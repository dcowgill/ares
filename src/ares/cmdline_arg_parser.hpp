// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_cmdline_arg_parser
#define included_ares_cmdline_arg_parser

#include <map>
#include <string>

namespace ares {

// Parses command-line arguments consisting of "name=value" pairs. This format
// is different from one expected by the typical UNIX command line program
// (namely, switches preceeded by one or two hyphens). For example, the UNIX
// `ls` program can be run as follows:
//
//      ls -SR --width=40
//
// which means "sort by file size, list subdirectories recursively, and use 40
// columns for display." If `ls` were to use this class for argument parsing,
// the command line above might instead look like this:
//
//      ls sort=size recurse=yes width=40
//
// Although more verbose, the second syntax is simple and easy to understand
// without consulting a manual. For programs that are rarely run by hand (such
// as servers), the extra typing is only a small inconvenience.
//
// Note: A program using this class should not apply const specifiers to the
// main function's argv parameter, instead declaring main using the classic
// C-style signature: main(int argc, char** argv).
class Cmdline_arg_parser {
  public:
    // Constructs a parser given the argc and argv values passed to the main
    // function. The ignore_key_case parameter specifies whether the get_int,
    // get_double, and get_string functions will ignore the case of the key
    // name when looking for key=value pairs. Throws a Cmdline_syntax_error
    // exception if there is a syntax error in the command line format. Throws
    // a Cmdline_empty_key_error if a key=value pair on the command line has
    // an empty key.
    Cmdline_arg_parser(int argc, char** argv, bool ignore_key_case=true);

    // Retrieves the string value associated with the specified name. Throws a
    // Cmdline_arg_not_found_error exception if no argument with the specified
    // name was provided on the command line.
    std::string get_string(std::string name) const;

    // Similar to get_string, this function converts its return vaule to an
    // int before returning it. Throws a boost::lexical_cast exception if the
    // argument value could not be converted to an int.
    int get_int(std::string name) const;

    // Similar to get_int, this function returns a double instead of an int.
    double get_double(std::string name) const;

    // Similar to get_string(std::string) but doesn't raise an error if the
    // key wasn't found, returning the def argument instead.
    std::string get_string(std::string name, std::string def) const;

    // Returns true if an argument with the specified name was given and was
    // set to either "yes" or "y" (case is ignored). Returns false otherwise.
    bool get_yes_or_no(std::string name) const;

    // Returns true if a key=value pair with key equal to name was specified
    // on the command line.
    bool exists(std::string name) const;

    // Returns the number of key=value pairs on the command line.
    int num_args() const;

  private:
    typedef std::map<std::string, std::string> Arg_map;
    Arg_map::const_iterator lookup(std::string name) const;

    Arg_map m_args;                 // maps keys to values
    bool const m_ignore_key_case;   // true if keys are case-insensitive
};

} // namespace ares

#endif
