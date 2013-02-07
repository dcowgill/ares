// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_error
#define included_ares_error

#include "ares/exception.hpp"

namespace ares {

struct Errors {
    enum {
        // general errors
        NOT_IMPLEMENTED               = 1,
        RANGE                         = 2,
        TIMEOUT                       = 3,
        PROTOCOL                      = 4,
        CHAR_TO_NUM_CONVERSION        = 5,
        INTERNAL                      = 600,

        // system
        SYSTEM                        = 800,
        INPUT_OUTPUT                  = 801,
        NETWORK                       = 802,
        NETWORK_IO                    = 803,
        INVALID_SOCKET                = 900,
        SOCKET_NOT_SELECTABLE         = 901,
        THREAD_ALREADY_RUNNING        = 910,
        THREAD_NOT_RUNNING            = 911,
        THREAD_EXIT_TIMEOUT           = 912,

        // utility
        CMDLINE_ARG_PARSER            = 2000,
        CMDLINE_ARG_NOT_FOUND         = 2001,
        CMDLINE_EMPTY_KEY             = 2002,
        CMDLINE_SYNTAX                = 2003,
        STRING_TOKENIZER_UNDERFLOW    = 2100,
        INVALID_TRACE_DUMP_DEST       = 2900,
        INVALID_TRACE_THREAD_NAME     = 2901,

        // i/o
        BUFFER_FORMATTER              = 3000,
        PACKET_SIZE_EXCEEDED          = 3001,
        MESSAGE_SIZE_EXCEEDED         = 3002,
        PACKET_SEQUENCE               = 3003,

        // network
        SOCKET_ACCEPTOR               = 4000,
        LISTEN_SOCKET_ALREADY_BOUND   = 4001,
        LISTEN_SOCKET_NOT_BOUND       = 4002,
        BIND_FAILED                   = 4003,

        // server framework
        ILLEGAL_PROCESSOR_COUNT       = 5100,
        INVALID_LOG_LEVEL             = 5200,
        LOG_FILE_ATTACH               = 5201,
        INPUT_BUFFER_TOO_SMALL        = 5500,
    };
};

// The error base class.
struct Error : public Exception {
    explicit Error(int error_code) : Exception("ares", error_code) {}

    Error(int error_code, char const* arg_types, ...)
            : Exception("ares", error_code) { ARES_SET_EXCEPTION_VARARGS }
};

// An unimplemented action was attempted.
struct Not_implemented_error : public Error {
    Not_implemented_error(char const* operation)
            : Error(Errors::NOT_IMPLEMENTED, "s", operation) {}
    char const* message() const;
};

// A value was out of range.
struct Range_error : public Error {
    Range_error() : Error(Errors::RANGE) {}
    char const* message() const;
};

// Some operation timed out.
struct Timeout_error : public Error {
    Timeout_error() : Error(Errors::TIMEOUT) {}
    char const* message() const;
};

// A communications protocol was violated.
struct Protocol_error : public Error {
    Protocol_error(char const* description)
            : Error(Errors::PROTOCOL, "s", description) {}
    char const* message() const;
};

// A conversion from character data to a number failed.
struct Char_to_num_conversion_error : public Error {
    Char_to_num_conversion_error(char const* value)
            : Error(Errors::CHAR_TO_NUM_CONVERSION, "s", value) {}
    char const* message() const;
};

// There was an unexpected internal error in the system.
struct Internal_error : public Error {
    Internal_error(char const* arg_types, ...)
            : Error(Errors::INTERNAL) { ARES_SET_EXCEPTION_VARARGS }
    char const* message() const;
};

// A system call failed.
struct System_error : virtual public Error {
    System_error(char const* syscall, int errnum);
    char const* message() const;
};

// An I/O syscall failed.
struct IO_error : public System_error {
    IO_error(char const* syscall, int errnum);
    char const* message() const;
};

// A network syscall failed.
struct Network_error : public System_error {
    Network_error(char const* syscall, int errnum);
    char const* message() const;
};

// A network I/O syscall failed.
struct Network_io_error : public Network_error, public IO_error {
    Network_io_error(char const* syscall, int errnum);
    char const* message() const;
};

// An invalid socket handle was passed to a function.
struct Invalid_socket_error : public Error {
    Invalid_socket_error(int fd)
            : Error(Errors::INVALID_SOCKET, "d", fd) {}
    char const* message() const;
};

// System failed to multiplex i/o on a socket handle.
struct Socket_not_selectable_error : public Error {
    Socket_not_selectable_error(int fd)
            : Error(Errors::SOCKET_NOT_SELECTABLE, "d", fd) {}
    char const* message() const;
};

// System tried to start a thread that was already running.
struct Thread_already_running_error : public Error {
    Thread_already_running_error()
            : Error(Errors::THREAD_ALREADY_RUNNING) {}
    char const* message() const;
};

// System tried to stop a thread that was not running.
struct Thread_not_running_error : public Error {
    Thread_not_running_error() : Error(Errors::THREAD_NOT_RUNNING) {}
    char const* message() const;
};

// System timed out while waiting for a thread to exit.
struct Thread_exit_timeout_error : public Error {
    Thread_exit_timeout_error() : Error(Errors::THREAD_EXIT_TIMEOUT) {}
    char const* message() const;
};

// Base class for all errors thrown by Cmdline_arg_parser objects.
struct Cmdline_arg_parser_error : virtual public Error {
    Cmdline_arg_parser_error() : Error(Errors::CMDLINE_ARG_PARSER) {}
    char const* message() const;
};

// A Cmdline_arg_parser was asked for a non-existent argument.
struct Cmdline_arg_not_found_error : public Cmdline_arg_parser_error {
    Cmdline_arg_not_found_error(std::string arg_name)
            : Error(Errors::CMDLINE_ARG_NOT_FOUND, "s", arg_name.c_str()) {}
    char const* message() const;
};

// A Cmdline_arg_parser failed on bad syntax: missing key in a key=value pair.
struct Cmdline_empty_key_error : public Cmdline_arg_parser_error {
    Cmdline_empty_key_error() : Error(Errors::CMDLINE_EMPTY_KEY) {}
    char const* message() const;
};

// A Cmdline_arg_parser failed on bad syntax (generic).
struct Cmdline_syntax_error : public Cmdline_arg_parser_error {
    Cmdline_syntax_error() : Error(Errors::CMDLINE_SYNTAX) {}
    char const* message() const;
};

// No tokens remaining in stream, but a tokenizer was asked for the next token.
struct String_tokenizer_underflow : public Error {
    String_tokenizer_underflow() : Error(Errors::STRING_TOKENIZER_UNDERFLOW) {}
    char const* message() const;
};

// An invalid destination for trace dump files was specified.
struct Invalid_trace_dump_dest_error : public Error {
    Invalid_trace_dump_dest_error(std::string const& path)
            : Error(Errors::INVALID_TRACE_DUMP_DEST, "s", path.c_str()) {}
    char const* message() const;
};

// An invalid thread name for tracing was specified.
struct Invalid_trace_thread_name_error : public Error {
    Invalid_trace_thread_name_error(char const* name)
            : Error(Errors::INVALID_TRACE_THREAD_NAME, "s", name) {}
    char const* message() const;
};

// Base class for all errors thrown by Buffer_formatter objects. Thrown when
// an operation on a Buffer_formatter or derived object fails.
// Buffer_formatter objects throw exceptions only after the
// Buffer_formatter::raise_errors function is passed a true value.
struct Buffer_formatter_error : virtual public Error {
    Buffer_formatter_error() : Error(Errors::BUFFER_FORMATTER) {}
    char const* message() const;
};

// Thrown when a the overall size of a packet (including headers) exceeds the
// reader's maximum packet size (which is returned by
// Message_reader::max_packet_size). Message_reader is incapable of recovering
// from an exception of this type, and further message parsing on the affected
// communications channel will not be possible.
struct Packet_size_exceeded_error : public Buffer_formatter_error {
    Packet_size_exceeded_error(int packet_size, int max_packet_size)
            : Error(Errors::PACKET_SIZE_EXCEEDED, "dd",
                    packet_size, max_packet_size) {}
    char const* message() const;
};

// Thrown when a the combined data size of a series of chained packets
// comprising a single message exceeds the reader's maximum message size
// (which is returned by Message_reader::max_message_size). Message_reader
// will automatically recover from an error of this type.
struct Message_size_exceeded_error : public Buffer_formatter_error {
    Message_size_exceeded_error(int message_size, int max_message_size)
            : Error(Errors::MESSAGE_SIZE_EXCEEDED, "dd",
                    message_size, max_message_size) {}
    char const* message() const;
};

// Thrown when the system reads consecutive message packets with
// non-sequential packet numbers. This typically indicates a non-recoverable
// error in the communication protocol.
struct Packet_sequence_error : public Error {
    Packet_sequence_error() : Error(Errors::PACKET_SEQUENCE) {}
    char const* message() const;
};

// Base class for all errors thrown by Socket_acceptor objects.
struct Socket_acceptor_error : virtual public Error {
    Socket_acceptor_error() : Error(Errors::SOCKET_ACCEPTOR) {}
    char const* message() const;
};

// System tried to bind a listen socket that was already bound.
struct Listen_socket_already_bound_error : public Socket_acceptor_error {
    Listen_socket_already_bound_error()
            : Error(Errors::LISTEN_SOCKET_ALREADY_BOUND) {}
    char const* message() const;
};

// An operation failed because the listen socket was closed/not bound.
struct Listen_socket_not_bound_error : Socket_acceptor_error {
    Listen_socket_not_bound_error()
            : Error(Errors::LISTEN_SOCKET_NOT_BOUND) {}
    char const* message() const;
};

// Failed to bind a socket to a host and port.
struct Bind_failed_error : public Socket_acceptor_error {
    Bind_failed_error(char const* address, char const* port)
            : Error(Errors::BIND_FAILED, "ss", address, port) {}
    char const* message() const;
};

// System tried to set the number of command processors to an illegal value.
struct Illegal_processor_count_error : public Error {
    Illegal_processor_count_error(int count)
            : Error(Errors::ILLEGAL_PROCESSOR_COUNT, "d", count) {}
    char const* message() const;
};

// The system logger encountered an unrecognized or invalid log level.
struct Invalid_log_level_error : public Error {
    Invalid_log_level_error() : Error(Errors::INVALID_LOG_LEVEL) {}
    char const* message() const;
};

// An error occurred while tried to attach to a new log file.
struct Log_file_attach_error : public Error {
    Log_file_attach_error(std::string const& filename)
            : Error(Errors::LOG_FILE_ATTACH, "s", filename.c_str()) {}
    char const* message() const;
};

// Communication protocol failed because an input buffer was too small.
struct Input_buffer_too_small_error : public Error {
    Input_buffer_too_small_error(int capacity)
            : Error(Errors::INPUT_BUFFER_TOO_SMALL, "d", capacity) {}
    char const* message() const;
};

} // namespace ares

#endif
