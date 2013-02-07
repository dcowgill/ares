// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_file_util
#define included_ares_file_util

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdio>
#include <string>

namespace ares {

// Converts a possibly relative path to an absolute path, resolving any
// symbolic links. Throws a System_error if the directory does not exist.
std::string make_full_path(std::string const& path);

// Returns true if and only if the given path is relative.
bool is_relative_path(std::string const& path);

// Returns true if the directory exists and is readable, false if it does not
// exist or if the user doesn't have permission to read it. Throws a
// System_error exception if an error occurs.
bool does_directory_exist(std::string const& path);

// Strips the parent directory from the given path and returns the result. If
// path is relative, returns the path unadulterated. If path is empty or
// consists or nothing but directory separator characters, return an empty
// string. Trailing directory separator characters are ignored.
std::string base_name(std::string const& path);

// Similar to base_name, except this function returns the part of the path
// that base_name removes. If path is relative, returns an empty string.
std::string dir_name(std::string const& path);

// Insures that the directory specified by path exists, including any
// necessary but nonexistent parent directories. It is not an error to give
// call this function with a path to a directory that already exists. The
// permissions to set for the new directories may optionally be specified
// (they default to 0755). Throws a System_error on failure.
void make_directory(std::string const& path, int mode = 0755);

// Reads an entire file into memory and returns it as a string.
std::string read_file(std::string const& path);

} // namespace ares

#endif
