// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_exception
#define included_ares_exception

#include <stdarg.h>
#include <string>

namespace ares {

#define ARES_SET_EXCEPTION_VARARGS {            \
        va_list ap;                             \
        va_start(ap, arg_types);                \
        set_arguments(arg_types, ap);           \
        va_end(ap);                             \
    }

// Base class for all exception objects thrown by the framework.
class Exception {
  public:
    // Constructs an exception. The caller must specify the module to which
    // the error belongs and an integer that uniquely identifies the error.
    // The module name does not indicate where the exception originated.
    Exception(const char* module, int error_code);

    // Constructs an exception with an arbitrary number of arguments. The
    // caller must specify the module to which the error belongs and an
    // integer that uniquely identifies the error. The module name does not
    // indicate where the exception originated. This constructor can also take
    // a variable number of additional arguments, which are stored with the
    // exception object and used to format the error message. Each character
    // in arg_types specifies the types of a subsequent argument in the order
    // in which they are supplied. Only int, double, const char*, and const
    // void* types are supported; they are identified by the characters 'd',
    // 'g', 's', 'p' in arg_types.
    Exception(const char* module, int error_code, const char* arg_types, ...);

    // Copy constructor.
    Exception(const Exception& e);

    // Destructor.
    virtual ~Exception();

    // Sets the causal exception, creating an error chain. This allows
    // low-level errors to be translated into high-level ones without the loss
    // of any information. The exception passed to this function can
    // subsequently be queried via the Exception::next_exception function.
    void set_cause(const Exception& e);

    // Returns the module to which this exception belongs.
    const char* module() const;

    // Returns the unique error code for this exception.
    int code() const;

    // Returns the type of the nth argument to this exception. See the
    // constructor documentation for more information. Undefined if n is out
    // of range.
    int argument_type(int n) const;

    // Returns the integer value of the nth argument to this exception,
    // converting it if necessary. Undefined if n is out of range.
    int arg_int_val(int n) const;

    // Returns the double value of the nth argument to this exception,
    // converting it if necessary. Undefined if n is out of range.
    double arg_dbl_val(int n) const;

    // Returns the string value of the nth argument to this exception,
    // converting it if necessary. Undefined if n is out of range.
    std::string arg_str_val(int n) const;

    // Returns the pointer value of the nth argument to this exception,
    // converting it if necessary. Undefined if n is out of range.
    const void* arg_ptr_val(int n) const;

    // Returns the number of arguments to this exception.
    int num_arguments() const;

    // Returns the exception in the error chain after this one. Undefined if
    // there is no next exception (see Exception::has_next_exception).
    const Exception& next_exception() const;

    // Tests if there is an exception after this one in the error chain.
    bool has_next_exception() const;

    // Returns a textual representation of this exception.
    std::string to_string() const;

  private:
    virtual const char* message() const;

  protected:
    void set_arguments(const char* arg_types, ...);
    void set_arguments(const char* arg_types, va_list args);

  private:
    Exception& operator=(const Exception&);  // not implemented

    union Value {
        int         int_val;
        double      dbl_val;
        char*       str_val;
        const void* ptr_val;
    };
    typedef std::pair<int, Value> Arg;  // represents an exception argument

    void set_arguments(Arg* args, int num_args);
    void unset_arguments();

    const char* m_module;   // module in which the error originated
    const int m_code;       // unique code identifying the error
    const char* m_message;  // printf-style format for error message
    Arg* m_args;            // array of exception arguments
    int m_num_args;         // size of m_args
    Exception* m_next;      // link to next error in stack
};

} // namespace ares

#endif
