// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/cmdline_arg_parser.hpp"
#include "ares/error.hpp"
#include "ares/string_util.hpp"
#include "ares/utility.hpp"

using namespace std;
using ares::Cmdline_arg_parser;

Cmdline_arg_parser::Cmdline_arg_parser(int argc,
                                       char** argv,
                                       bool ignore_key_case)
        : m_ignore_key_case(ignore_key_case)
{
    // Arguments are name=value pairs. Whitespace around the equal sign is
    // optional. The possible patterns are:
    //
    // "name=value"
    // "name = value"
    // "name= value"
    // "name =value"

    enum { NAME, EQUALS, VALUE };   // what we are currently looking for
    int state = NAME;               // the initial state

    string arg;
    string name;

    for (int i = 1; i < argc; i++) {
        arg = argv[i];

        if (state == NAME) {
            string::size_type pos = arg.find('=');

            if (pos != string::npos) {
                name = arg.substr(0, pos);
                arg  = arg.substr(pos + 1);

                if (name.empty()) {
                    throw Cmdline_empty_key_error();
                }

                if (m_ignore_key_case) {
                    name = boost::to_lower_copy(name);
                }

                if (arg.empty()) {
                    state = VALUE;          // "name= value"
                } else {
                    m_args[name] = arg;     // "name=value"
                }
            }
            else {
                name = arg;                 // "name = value" OR
                state = EQUALS;             // "name =value"
            }
        }
        else if (state == EQUALS) {
            if (arg.find('=') != 0) {
                throw Cmdline_syntax_error();
            }

            arg = arg.substr(1);
            if (arg.empty()) {
                state = VALUE;              // "name = value"
            } else {
                m_args[name] = arg;         // "name =value"
                state = NAME;               // ready for next pair
            }
        }
        else {
            m_args[name] = arg;
            state = NAME;                   // ready for next pair
        }
    }
}

int Cmdline_arg_parser::get_int(string name) const {
    return boost::lexical_cast<int>(get_string(name));
}

double Cmdline_arg_parser::get_double(string name) const {
    return boost::lexical_cast<double>(get_string(name));
}

string Cmdline_arg_parser::get_string(string name) const {
    Arg_map::const_iterator it = lookup(name);
    if (it == m_args.end()) {
        throw Cmdline_arg_not_found_error(name);
    }
    return it->second;
}

string Cmdline_arg_parser::get_string(string name, string def) const {
    Arg_map::const_iterator it = lookup(name);
    return it != m_args.end() ? it->second : def;
}

bool Cmdline_arg_parser::get_yes_or_no(string name) const {
    string s = boost::to_lower_copy(get_string(name, ""));
    return s=="y" || s=="yes";

}

bool Cmdline_arg_parser::exists(string name) const
{
    return lookup(name) != m_args.end();
}

int Cmdline_arg_parser::num_args() const
{
    return int(m_args.size());
}

Cmdline_arg_parser::Arg_map::const_iterator
Cmdline_arg_parser::lookup(string name) const
{
    return m_args.find(m_ignore_key_case ? boost::to_lower_copy(name) : name);
}
