// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_string_tokenizer
#define included_ares_string_tokenizer

#include "ares/error.hpp"
#include "ares/utility.hpp"
#include <string>

namespace ares {

// Divides a string into a sequence of delimited tokens.
class String_tokenizer : boost::noncopyable {
  public:
    // Creates a new tokenizer for s. The default delimiters are the space,
    // newline, carriage-return and form-feed characters.
    String_tokenizer(std::string const& s);

    // Creates a new tokenizer for s, specifying the characters used to
    // delimit tokens in delim. Each character in delim is a valid delimiter
    // character.
    String_tokenizer(std::string const& s, std::string const& delim);

    // Destructor.
    ~String_tokenizer();

    // Tokenizes s, using the configuration supplied to the constructor.
    void reset(std::string const& s);

    // Sets the character set used to delimit tokens. Note that, if this
    // function is called during the tokenization of a string, any delimiter
    // characters that follow the last consumed token will be lost.
    void set_delim(std::string const& delim);

    // Returns the next token in the sequence. Throws a Logic_error exception
    // is the token sequence has been exhausted.
    std::string const& next();

    // Returns true if there are more tokens available. It is an error to call
    // next when this method would return false.
    bool has_next() const;

    // Returns the remaining input, un-tokenized. If the entire input has
    // already been exhausted, this function returns an empty string. Leading
    // delimiter characters are not included in the remaining input.
    std::string rest() const;

  private:
    std::string m_string;       // string to tokenize
    std::string m_delim;        // delimiter characters
    std::string m_token;        // storage for current token
    mutable int m_cur_pos;      // current position in string
    mutable int m_max_pos;      // maximum position in string
};

} // namespace ares

#endif
