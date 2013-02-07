// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/error.hpp"
#include <cstring>

using namespace std;

char const* ares::Not_implemented_error::message() const
{
    return "operation \"%1$s\" is not implemented";
}

char const* ares::Range_error::message() const
{
    return "value out of range";
}

char const* ares::Timeout_error::message() const
{
    return "operation timed out";
}

char const* ares::Protocol_error::message() const
{
    return "protocol error: %1$s";
}

char const* ares::Char_to_num_conversion_error::message() const
{
    return "character to number conversion failed for \"%1$s\"";
}

char const* ares::Internal_error::message() const
{
    return "internal exception, arguments: [%1$s],[%2$s],[%3$s],[%4$s],[%5$s],[%6$s],[%7$s],[%8$s]";
}

ares::System_error::System_error(char const* syscall, int errnum)
        : Error(Errors::SYSTEM, "sds", syscall, errnum, strerror(errnum))
{}

char const* ares::System_error::message() const
{
    return "syscall \"%1$s\" failed [%2$d]: %3$s";
}

ares::IO_error::IO_error(char const* syscall, int errnum)
        : Error(Errors::INPUT_OUTPUT, "sds", syscall, errnum, strerror(errnum))
        , System_error(0,0)
{}

char const* ares::IO_error::message() const
{
    return "i/o syscall \"%1$s\" failed [%2$d]: %3$s";
}

ares::Network_error::Network_error(char const* syscall, int errnum)
        : Error(Errors::NETWORK, "sds", syscall, errnum, strerror(errnum))
        , System_error(0,0)
{}

char const* ares::Network_error::message() const
{
    return "network syscall \"%1$s\" failed [%2$d]: %3$s";
}

ares::Network_io_error::Network_io_error(char const* syscall, int errnum)
        : Error(Errors::NETWORK_IO, "sds", syscall, errnum, strerror(errnum))
        , Network_error(0,0)
        , IO_error(0,0)
{}

char const* ares::Network_io_error::message() const
{
    return "network i/o syscall \"%1$s\" failed [%2$d]: %3$s";
}

char const* ares::Invalid_socket_error::message() const
{
    return "invalid socket descriptor: %1$d";
}

char const* ares::Socket_not_selectable_error::message() const
{
    return "socket descriptor out of range: %1$d";
}

char const* ares::Thread_already_running_error::message() const
{
    return "thread is already running";
}

char const* ares::Thread_not_running_error::message() const
{
    return "thread is not running";
}

char const* ares::Thread_exit_timeout_error::message() const
{
    return "timed out waiting for thread exit";
}

char const* ares::Cmdline_arg_parser_error::message() const
{
    return "command line parser exception";
}

char const* ares::Cmdline_arg_not_found_error::message() const
{
    return "command line argument \"%1$s\" not found";
}

char const* ares::Cmdline_empty_key_error::message() const
{
    return "command line syntax error; empty key in pair";
}

char const* ares::Cmdline_syntax_error::message() const
{
    return "command line syntax error";
}

char const* ares::String_tokenizer_underflow::message() const
{
    return "no more tokens in stream";
}

char const* ares::Invalid_trace_dump_dest_error::message() const
{
    return "invalid trace dump dest: \"%1$s\"";
}

char const* ares::Invalid_trace_thread_name_error::message() const
{
    return "invalid trace thread name: \"%1$s\"";
}

char const* ares::Buffer_formatter_error::message() const
{
    return "buffer formatter exception";
}

char const* ares::Packet_size_exceeded_error::message() const
{
    return "packet size %1$d exceeds maximum of %2$d";
}

char const* ares::Message_size_exceeded_error::message() const
{
    return "message size %1$d exceeds maximum of %2$d";
}

char const* ares::Packet_sequence_error::message() const
{
    return "packet read out of sequence";
}

char const* ares::Socket_acceptor_error::message() const
{
    return "socket acceptor exception";
}

char const* ares::Listen_socket_already_bound_error::message() const
{
    return "listen socket already bound";
}

char const* ares::Listen_socket_not_bound_error::message() const
{
    return "listen socket not bound";
}

char const* ares::Bind_failed_error::message() const
{
    return "could not bind listen socket to address \"%1$s\" on port \"%2$s\"";
}

char const* ares::Illegal_processor_count_error::message() const
{
    return "illegal processor count specified: %1$d";
}

char const* ares::Invalid_log_level_error::message() const
{
    return "illegal log level specified";
}

char const* ares::Log_file_attach_error::message() const
{
    return "could not attach to log file \"%1$s\"";
}

char const* ares::Input_buffer_too_small_error::message() const
{
    return "input buffer capacity %1$d is insufficient";
}
