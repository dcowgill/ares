// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/trace.hpp"
#include "ares/date.hpp"
#include "ares/error.hpp"
#include "ares/file_util.hpp"
#include "ares/string_tokenizer.hpp"
#include "ares/string_util.hpp"
#include "ares/thread.hpp"
#include <cassert>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <vector>

using namespace std;

#ifdef ARES_TRACE_ENABLED
namespace
{
// The directory into which trace files are written. It must be an
// absolute path and will be converted to one if necessary. If empty or
// not initialized, defaults to "/tmp".
string dump_dest;

// This vector contains a list of strings that identify modules to trace.
// Each string may optionally begin with '^' or end with '$' or both; the
// '^' and '$' characters are equivalent to their corresponding regular
// expression operators.
vector<string> trace_modules;

// The thread-specific name. This may be optionally set by each thread to
// make it easier to identify its trace file. A thread name may only be
// composed of letters, numbers, and underscores; invalid characters in a
// thread name will simply be removed.
ares::Thread_specific_value<char> thread_name;

// The thread-specific trace file pointer. Created on demand. The format
// for output files is "ares_{tid}.trc" where "{tid}" is replaced with an
// integer representation of the thread id. Optionally, if the thread name
// has been set (see above), the output file format is "{name}.trc" where
// "{name}" is the thread name.
ares::Thread_specific_value<FILE> trace_fp;
}
#endif

bool ares::Trace::is_enabled()
{
#ifdef ARES_TRACE_ENABLED
    return true;
#else
    return false;
#endif
}

void ares::Trace::set_dump_dest(string const& path)
        try {
#ifdef ARES_TRACE_ENABLED
            make_directory(path);
            dump_dest = make_full_path(path);
#endif
        }
        catch (Exception& cause) {
            Invalid_trace_dump_dest_error e(path);
            e.set_cause(cause);
            throw e;
        }

void ares::Trace::set_trace_modules(std::string const& trace_str)
{
#ifdef ARES_TRACE_ENABLED
    trace_modules.clear();
    String_tokenizer tok(trace_str);
    while (tok.has_next()) {
        string const& s = tok.next();
        if (s == "all") {
            trace_modules.clear();
            trace_modules.push_back(s);
            return;
        }
        trace_modules.push_back(s);
    }
#endif
}

void ares::Trace::set_thread_name(char const* name)
{
#ifdef ARES_TRACE_ENABLED
    // create a copy of name, but ignore invalid characters
    int name_len = strlen(name);
    char* s = new char[name_len+1];
    char* p = s;
    for (; *name; name++)
        if (isalnum(*name) || *name=='_')   // only alphanumeric and '_'
            *p++ = *name;
    *p = '\0';

    // if no characters were copied, raise an error
    if (s == p) {
        delete [] s;
        throw Invalid_trace_thread_name_error(name);
    }

    // delete the old thread name, if one exists
    if (thread_name.get())
        delete [] thread_name.get();

    // set the new thread name
    thread_name.reset(s);
#endif
}

bool ares::Trace::should_trace(char const* filename)
{
#ifdef ARES_TRACE_ENABLED
    // if no trace modules were specified, we never trace
    if (trace_modules.empty())
        return false;

    // if the first trace module is set to "all", we always trace
    if (trace_modules[0] == "all")
        return true;

    // we must check every trace module until we find a match
    for (int i = 0, n = trace_modules.size(); i < n; i++) {
        char const* module = trace_modules[i].c_str();
        int len = trace_modules[i].length();

        if (module[0] == '^') {
            if (module[len-1] == '$') {
                int filename_len = strlen(filename);

                // require an exact match between module and filename
                if (filename_len == len-2)
                    if (memcmp(module+1, filename, filename_len)==0)
                        return true;
            }
            else if (strncmp(module+1, filename, len-1) == 0)
                return true;
        }
        else if (module[len-1] == '$') {
            int offset = strlen(filename) - (len-1);

            // match the trailing part of the filename and the module
            if (offset >= 0 && strncmp(module, filename+offset, len-1) == 0)
                return true;
        }
        else if (strstr(filename, module) != 0)
            return true;
    }
#endif

    // none of the trace modules matched; do not trace
    return false;
}

#ifdef ARES_TRACE_ENABLED
static void real_trace_fn(char const* format, ...)
{
    if (FILE* fp = trace_fp.get()) {
        va_list ap;
        va_start(ap, format);
        vfprintf(fp, format, ap);
        fputc('\n', fp);
        fflush(fp);
        va_end(ap);
    }
}

static bool open_trace_file()
{
    using namespace ares;

    // close the existing trace file, if one exists
    if (trace_fp.get()) {
        fclose(trace_fp.get());
        trace_fp.reset();
    }

    char const* dump_dir = dump_dest.empty() ? "/tmp" : dump_dest.c_str();
    string path =
            thread_name.get() != 0
            ? format("%s/%s.trc", dump_dir, thread_name.get())
            : format("%s/ares_%u.trc", dump_dir, Thread::current_thread_id());

    FILE* fp = fopen(path.c_str(), "a");
    if (!fp) {
        fprintf(stderr, "WARNING: unable to open trace file '%s': %s\n",
                path.c_str(), strerror(errno));
        return false;
    }
    trace_fp.reset(fp);
    return true;
}

ares::Trace::Trace_fn ares::Trace::trace(char const* file, int line)
{
    // if there is no trace file and we're unable to open it, fail
    if (!trace_fp.get() && !open_trace_file())
        return no_trace;

    FILE* fp = trace_fp.get();
    assert(fp);

    // write to the trace file; if an i/o error occurs, attempt to re-open it
    Int64 time_ms = current_time_millis();
    fprintf(fp, "%s.%03d (%s:%d) ", Date(time_ms / 1000).to_string().c_str(),
            int(time_ms % 1000), file, line);
    if (feof(fp) || ferror(fp))
        if (!open_trace_file())
            return no_trace;
    return real_trace_fn;
}
#endif // ARES_TRACE_ENABLED
