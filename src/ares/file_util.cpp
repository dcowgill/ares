// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/file_util.hpp"
#include "ares/error.hpp"
#include "ares/string_tokenizer.hpp"
#include "ares/string_util.hpp"
#include <cerrno>
#include <memory>
#include <vector>

// UNIX headers
#include <dirent.h>         // for opendir(3)
#include <limits.h>         // for PATH_MAX
#include <sys/stat.h>       // for S_xxx file mode constants and umask(2)
#include <sys/types.h>      // basic system data types
#include <unistd.h>

using namespace std;

namespace
{
// Character constant used to separate directories on this platform.
char const DIR_SEP = '/';

// Sometimes we need to use DIR_SEP as if it were a c-style string.
char const DIR_SEP_CSTR[] = "/";
}

string ares::make_full_path(string const& path)
{
    vector<char> absolute_path(PATH_MAX);

    if (!realpath(path.c_str(), &absolute_path[0]))
        throw System_error("realpath", errno);
    return string(&absolute_path[0]);
}

bool ares::is_relative_path(string const& path)
{
    return path[0] != DIR_SEP;
}

bool ares::does_directory_exist(string const& path)
{
    errno = 0;
    if (DIR* dp = opendir(path.c_str())) {
        closedir(dp);
        return true;
    }
    if (errno != ENOENT && errno != EACCES)
        throw System_error("opendir", errno);
    return false;
}

string ares::base_name(string const& path)
{
    // path might end with one or more DIR_SEP chars; remove them.
    string const s = erase_trailing_chars(path, DIR_SEP_CSTR);

    // If path was either empty or nothing but DIR_SEP chars, return "".
    if (s.empty()) return "";

    // Find the position of the last DIR_SEP char.
    string::size_type i = s.find_last_of(DIR_SEP);

    // Was the path relative? If so, s is a filename, so simply return it.
    if (i == string::npos) return s;

    // Return the substring containing characters i+1 through the end.
    return s.substr(i+1);
}

string ares::dir_name(string const& path)
{
    // path might end with one or more DIR_SEP chars; remove them.
    string const s = erase_trailing_chars(path, DIR_SEP_CSTR);

    // If path was either empty or nothing but DIR_SEP chars, return DIR_SEP.
    if (s.empty()) return DIR_SEP_CSTR;

    // Find the position of the last DIR_SEP char.
    string::size_type i = s.find_last_of(DIR_SEP);

    // Was the path relative? If so, we can only return the empty string.
    if (i == string::npos) return "";

    // Return the substring containing characters 0 through i.
    return s.substr(0, i+1);
}

void ares::make_directory(string const& path, int mode)
{
    int mkdir_errno = 0;
    string dir = (path[0] == DIR_SEP) ? DIR_SEP_CSTR : "";
    String_tokenizer tok(path, DIR_SEP_CSTR);
    while (tok.has_next()) {
        dir += tok.next();
        dir += DIR_SEP_CSTR;
        errno = 0;
        mkdir(dir.c_str(), mode);  // ignore errors
        mkdir_errno = errno;
    }
    if (!does_directory_exist(path))
        throw System_error("mkdir", mkdir_errno);
}

string ares::read_file(string const& path)
{
    FILE* fp = fopen(path.c_str(), "r");
    if (!fp)
        throw IO_error("fopen", errno);
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto_ptr<char> s(new char[file_size]);
    fread(s.get(), sizeof(char), file_size, fp);
    if (fclose(fp) != 0)
        throw IO_error("fclose", errno);
    return string(s.get(), file_size);
}
