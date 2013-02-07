// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_date
#define included_ares_date

#include "ares/types.hpp"
#include <ctime>    // for std::time_t
#include <iostream>
#include <string>

namespace ares {

// A simple date class with second resolution. This class does not have any
// support for time zones. Years are represented as full numbers--partial
// years, e.g. 97 to mean 1997, are not supported. Months and days are
// one-based, while hours, minutes, and seconds are zero-based.
class Date {
  public:
    // Constructs the default date and time, which is the unix epoch (i.e
    // 1970-01-01 00:00:00 UTC).
    Date();

    // Constructs a date given its year, month, day, hour, minute, and second.
    Date(int year, int mon, int day, int hour=0, int min=0, int sec=0);

    // Constructs a date from a unix timestamp.
    explicit Date(std::time_t t);

    // Constructs a date that is a copy of another.
    Date(Date const& d);

    // Assigns the specified date's value to this one.
    Date& operator=(Date const& d);

    // Assigns a unix timestamp to this date.
    Date& operator=(std::time_t t);

    // Adds the specified number of days to this date.
    Date& operator+=(double real_days);

    // Subtracts the specified number of days to this date.
    Date& operator-=(double real_days);

    // Adds the specified number of seconds to this date.
    Date& add_seconds(int n);

    // Sets this date and time given a year, month, day, hour, minute, and
    // second. Works exactly like the constructor taking similar arguments.
    void set(int year, int mon, int day, int hour=0, int min=0, int sec=0);

    // Returns the year for this date as a full number, including century.
    int year() const { return m_year; }

    // Returns the month of the year for this date as an integer between 1-12.
    int month() const { return m_mon; }

    // Returns the day of the month for this date as an integer between 1-31.
    int day() const { return m_day; }

    // Returns the hour in the day for this date as an integer between 0-23.
    int hour() const { return m_hour; }

    // Returns the minute in the hour for this date as an integer between
    // 0-59.
    int minutes() const { return m_min; }

    // Returns the second in the minute for this date as an integer between
    // 0-59.
    int seconds() const { return m_sec; }

    // Returns the day of the year for this date as an integer between 1-366.
    int day_of_year() const;

    // Returns the day of the week for this date as an integer between 0-6,
    // where 0 is sunday and 6 is saturday.
    int day_of_week() const;

    // Returns the julian day number for this date.
    int julian_day() const;

    // Returns the ISO-8601 week number for this date. Algorithm courtesy of
    // the Calendar FAQ (http://www.tondering.dk/claus/calendar.html).
    int week_number() const;

    // Returns an integer less than, equal to, or greater than zero if d is
    // less than, equal to, or greater than this date.
    int compare(Date const& d) const;

    // Truncates this date to the specified precision, which can be one of the
    // following: 'Y' for year (rounds up on July 1); 'M' for month (rounds up
    // on the sixteenth day); 'J' for day; 'D' for starting day of the week
    // (i.e. sunday); 'H' for hour; 'I' for minute; 'S' for second. Note that
    // only 'Y' and 'M' will round up; all other precisions simply truncate.
    Date trunc(char prec = 'J') const;

    // Converts this date to a unix timestamp.
    std::time_t to_timestamp() const;

    // Converts this date to a string using the default format, which is the
    // standard ISO-8601 format: "yyyy-mm-dd hh24:mi:ss".
    std::string to_string() const;

    // Returns the current operating system date and time.
    static Date now();

  private:
    Date& add(int days, int seconds);
    void incr_mon();
    void decr_mon();
    void incr_day();
    void decr_day();
    void incr_hour();
    void decr_hour();
    void incr_min();
    void decr_min();

  private:
    Int16  m_year;  // year including century
    Int8   m_mon;   // month as a number between 1 and 12
    Int8   m_day;   // day of the month
    Int8   m_hour;  // hour as a number between zero and 23
    Int8   m_min;   // minutes within the hour
    Int8   m_sec;   // seconds within the minute
};

// Arithmetic operators
Date operator+(Date const& date, double real_days);
Date operator-(Date const& date, double real_days);
double operator-(Date const& lhs, Date const& rhs);

// Comparison operators
bool operator==(Date const& lhs, Date const& rhs);
bool operator!=(Date const& lhs, Date const& rhs);
bool operator< (Date const& lhs, Date const& rhs);
bool operator<=(Date const& lhs, Date const& rhs);
bool operator> (Date const& lhs, Date const& rhs);
bool operator>=(Date const& lhs, Date const& rhs);

// Output operator
std::ostream& operator<<(std::ostream& o, Date const& d);

} // namespace ares

#endif
