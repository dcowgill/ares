// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/date.hpp"
#include "ares/error.hpp"
#include "ares/string_util.hpp"
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cmath>

using namespace std;

// Oracle's date format specifiers (well, some of them):
//
// AD       AD indicator.
// AM       Meridian indicator.
// BC       BC indicator.
// D        Day of the week (1-7).
// DAY      Name of day, padding with blanks to a length of 9 characters.
// DD       Day of month (1-31).
// DDD      Day of year (1-366).
// DY       Abbreviated name of day.
// HH       Hour of day (1-12).
// HH12     Hour of day (1-12).
// HH24     Hour of day (0-23).
// J        Julian day; the number of days since January 1, 4712 BC. Numbers
//          specified with 'J' must be integers.
// MI       Minute (0-59).
// MM       Two-digit numeric abbreviation of month (01-12; JAN = 01).
// MON      Abbreviated name of month.
// MONTH    Name of month, padding with blanks to length of 9 characters.
// PM       Meridian indicator.
// RM       Roman numeral month (I-XII; Jan = I).
// SS       Second (0-59).
// SSSSS    Seconds past midnight (0-86399).
// YYYY     4-digit year.

namespace
{
const int DAY_SEC = 24*60*60;

// Returns true if the given year is a leap year.
inline bool is_leap_year(int year)
{
    return (year%4 == 0 && year%100 != 0) || (year%400 == 0);
}

// Returns the number of seconds in a given year.
inline int seconds_in_year(int year)
{
    return (is_leap_year(year) ? 366 : 365) * DAY_SEC;
}

// Returns the number of days in the given month.
inline int days_in_month(int month, bool is_leap_year)
{
    static int const DAYS[] = {
        0, // ---
        31, // Jan
        28, // Feb
        31, // Mar
        30, // Apr
        31, // May
        30, // Jun
        31, // Jul
        31, // Aug
        30, // Sep
        31, // Oct
        30, // Nov
        31, // Dec
    };
    assert(month >= 1 && month <= 12);
    return month==2 ? (is_leap_year ? 29 : 28) : DAYS[month];
}

// Returns the number of days in the given month and year.
inline int days_in_month(int month, int year)
{
    return days_in_month(month, is_leap_year(year));
}
}


ares::Date::Date()
        : m_year(1970)
        , m_mon(1)
        , m_day(1)
        , m_hour(0)
        , m_min(0)
        , m_sec(0)
{}

ares::Date::Date(int year, int mon, int day, int hour, int min, int sec)
{
    set(year, mon, day, hour, min, sec);
}

ares::Date::Date(time_t t)
{
    *this = t;
}

ares::Date::Date(Date const& d)
        : m_year(d.m_year)
        , m_mon(d.m_mon)
        , m_day(d.m_day)
        , m_hour(d.m_hour)
        , m_min(d.m_min)
        , m_sec(d.m_sec)
{}

ares::Date& ares::Date::operator=(Date const& d)
{
    m_year = d.m_year;
    m_mon  = d.m_mon;
    m_day  = d.m_day;
    m_hour = d.m_hour;
    m_min  = d.m_min;
    m_sec  = d.m_sec;
    return *this;
}

ares::Date& ares::Date::operator=(time_t t)
{
    m_year = 1970;
    m_mon  = 1;
    m_day  = 1;
    m_hour = 0;
    m_min  = 0;
    m_sec  = 0;

    add_seconds(t);
    return *this;
}

ares::Date& ares::Date::operator+=(double real_days)
{
    int days = int(real_days);
    int seconds = int((real_days - days) * DAY_SEC);
    return add(days, seconds);
}

ares::Date& ares::Date::operator-=(double real_days)
{
    return (*this += -real_days);
}

ares::Date& ares::Date::add_seconds(int n)
{
    int days = n / DAY_SEC;
    n -= days * DAY_SEC;
    return add(days, n);
}

void ares::Date::set(int year, int mon, int day, int hour, int min, int sec)
{
    m_year = year;

    // bring the month within range
    if (mon < 1) {
        do { mon += 12; m_year--; } while (mon < 1);
    } else if (mon > 12) {
        do { mon -= 12; m_year++; } while (mon > 12);
    }

    m_mon = mon;

    // bring the day within range
    if (day < 1) {
        while (day < 1) {
            decr_mon();
            day += days_in_month(m_mon, m_year);
        }
    } else {
        while (day > days_in_month(m_mon, m_year)) {
            day -= days_in_month(m_mon, m_year);
            incr_mon();
        }
    }

    m_day = day;

    // bring the hour within range
    if (hour < 0) {
        while (hour < 0) {
            hour += 24;
            decr_day();
        }
    } else {
        while (hour > 23) {
            hour -= 24;
            incr_day();
        }
    }

    m_hour = hour;

    // bring the minutes within range
    if (min < 0) {
        while (min < 0) {
            min += 60;
            decr_hour();
        }
    } else {
        while (min > 59) {
            min -= 60;
            incr_hour();
        }
    }

    m_min = min;

    // bring the seconds within range
    if (sec < 0) {
        while (sec < 0) {
            sec += 60;
            decr_min();
        }
    } else {
        while (sec > 59) {
            sec -= 60;
            incr_min();
        }
    }

    m_sec = sec;
}

int ares::Date::day_of_year() const
{
    bool is_leap = is_leap_year(m_year);
    int yday = 0;

    // add months
    for (int i = 1; i < m_mon; i++)
        yday += days_in_month(i, is_leap);

    // add days
    return yday + m_day;
}

int ares::Date::day_of_week() const
{
    int a = (14-month()) / 12;
    int y = year() - a;
    int m = month() + 12*a - 2;

    return (day() + y + y/4 - y/100 + y/400 + 31*m/12) % 7;
}

int ares::Date::julian_day() const
{
    int a = (14-month()) / 12;
    int y = year() + 4800 - a;
    int m = month() + 12*a - 3;

    return day() + (153*m+2)/5 + 365*y + y/4 - y/100 + y/400 - 32405;
}

int ares::Date::week_number() const
{
    int j = julian_day();
    int d4 = (j + 31741 - (j%7)) % 146097 % 36524 % 1461;
    int l = d4/1460;
    int d1 = ((d4-l) % 365) + l;
    return d1/7 + 1;
}

int ares::Date::compare(Date const& d) const
{
    if (int delta = year() - d.year())
        return delta;
    if (int delta = month() - d.month())
        return delta;
    if (int delta = day() - d.day())
        return delta;
    if (int delta = hour() - d.hour())
        return delta;
    if (int delta = minutes() - d.minutes())
        return delta;
    if (int delta = seconds() - d.seconds())
        return delta;

    return 0;
}

ares::Date ares::Date::trunc(char prec) const
{
    Date copy(*this);

    switch (toupper(prec)) {
        case 'Y':
            if (copy.m_mon >= 7) {
                ++copy.m_year;
            }
            copy.m_mon = copy.m_day = 1;
            copy.m_hour = copy.m_min = copy.m_sec = 0;
            break;
        case 'M':
            if (copy.m_day >= 16) {
                if (++copy.m_mon > 12) {
                    copy.m_mon = 1;
                    ++copy.m_year;
                }
            }
            copy.m_day = 1;
            copy.m_hour = copy.m_min = copy.m_sec = 0;
            break;
        case 'J':
            copy.m_hour = copy.m_min = copy.m_sec = 0;
            break;
        case 'D':
            copy -= day_of_week();
            copy.m_hour = copy.m_min = copy.m_sec = 0;
            break;
        case 'H':
            copy.m_min = copy.m_sec = 0;
            break;
        case 'I':
            copy.m_sec = 0;
            break;
        case 'S':
            break;
    }

    return copy;
}

time_t ares::Date::to_timestamp() const
{
    bool const is_leap = is_leap_year(m_year);
    time_t t = 0;

    // add (or subtract) years
    if (m_year > 1970) {
        for (int i = 1970; i < m_year; i++) {
            t += seconds_in_year(i);
        }
    } else {
        for (int i = m_year; i < 1970; i++) {
            t -= seconds_in_year(i);
        }
    }

    // add months
    for (int i = 1; i < m_mon; i++) {
        t += days_in_month(i, is_leap)*DAY_SEC;
    }

    // add days, hours, minutes, seconds
    return t + (m_day-1)*DAY_SEC + m_hour*60*60 + m_min*60 + m_sec;
}

string ares::Date::to_string() const
{
    // the standard ISO-8601 format is "yyyy-mm-dd hh24:mi:ss"

    return format("%d-%02d-%02d %02d:%02d:%02d",
                  year(),
                  month(),
                  day(),
                  hour(),
                  minutes(),
                  seconds());
}

ares::Date ares::Date::now()
{
    return Date(time(0));
}

ares::Date& ares::Date::add(int days, int seconds)
{
    assert((days >= 0 && seconds >= 0) || (days <= 0 && seconds <= 0));
    assert(seconds < DAY_SEC && seconds > -DAY_SEC);

    int hour = m_hour;
    int min  = m_min;
    int sec  = m_sec;

    if (seconds > 0) {
        // Add the seconds to the date.
        if ((sec += seconds) >= 60) {
            int minutes = sec/60;
            sec -= minutes*60;
            if ((min += minutes) >= 60) {
                int hours = min/60;
                min -= hours*60;
                if ((hour += hours) >= 24) {
                    assert(hour < 48);
                    hour -= 24;
                    days++;
                }
            }
        }
    } else {
        seconds = -seconds;

        // subtract the seconds from the date
        if ((sec -= seconds) < 0) {
            int minutes = int(ceil(-sec/60.0));
            sec += minutes*60;
            if ((min -= minutes) < 0) {
                int hours = int(ceil(-min/60.0));
                min += hours*60;
                if ((hour -= hours) < 0) {
                    assert(hour >= -24);
                    hour += 24;
                    days--;
                }
            }
        }
    }

    m_hour = hour;
    m_min  = min;
    m_sec  = sec;

    // normalize to jan-01 of the current year
    days += day_of_year() - 1;
    m_mon = 1;
    m_day = 1;

    if (days < 0) {
        // transform this into an addition problem
        while (days < 0) {
            bool is_leap = is_leap_year(m_year-1);
            int ndays = is_leap ? 366 : 365;
            days += ndays;
            m_year--;
        }
    }

    // add years
    bool is_leap = is_leap_year(m_year);
    int ndays = is_leap ? 366 : 365;
    while (days >= ndays) {
        days -= ndays;
        is_leap = is_leap_year(++m_year);
        ndays = is_leap ? 366 : 365;
    }

    // add months
    ndays = days_in_month(m_mon, is_leap);
    while (days >= ndays) {
        days -= ndays;
        ndays = days_in_month(++m_mon, is_leap);
    }

    // add days
    m_day += days;

    return *this;
}

void ares::Date::incr_mon()
{
    if (++m_mon > 12) {
        m_mon = 1;
        m_year++;
    }
}

void ares::Date::decr_mon()
{
    if (--m_mon < 1) {
        m_mon = 12;
        m_year--;
    }
}

void ares::Date::incr_day()
{
    if (++m_day > days_in_month(m_mon, m_year)) {
        incr_mon();
        m_day = 1;
    }
}

void ares::Date::decr_day()
{
    if (--m_day < 1) {
        decr_mon();
        m_day = days_in_month(m_mon, m_year);
    }
}

void ares::Date::incr_hour()
{
    if (++m_hour > 23) {
        incr_day();
        m_hour = 0;
    }
}

void ares::Date::decr_hour()
{
    if (--m_hour < 0) {
        decr_day();
        m_hour = 23;
    }
}

void ares::Date::incr_min()
{
    if (++m_min > 59) {
        incr_hour();
        m_min = 0;
    }
}

void ares::Date::decr_min()
{
    if (--m_min < 0) {
        decr_hour();
        m_min = 59;
    }
}


// FREE OPERATORS

ares::Date ares::operator+(Date const& date, double real_days)
{
    Date result(date);
    return result += real_days;
}

ares::Date ares::operator-(Date const& date, double real_days)
{
    Date result(date);
    return result -= real_days;
}

double ares::operator-(Date const& lhs, Date const& rhs)
{
    int days = 0;

    // compute the difference of years in days
    if (lhs.year() >= rhs.year()) {
        for (int i = rhs.year(); i < lhs.year(); i++) {
            days += is_leap_year(i) ? 366 : 365;
        }
    } else {
        for (int i = lhs.year(); i < rhs.year(); i++) {
            days -= is_leap_year(i) ? 366 : 365;
        }
    }

    // compute the difference of months and days
    days += lhs.day_of_year() - rhs.day_of_year();

    // compute the difference in time of day
    int seconds = (lhs.hour()-rhs.hour())*3600 +
                  (lhs.minutes()-rhs.minutes())*60 +
                  (lhs.seconds()-rhs.seconds());

    return days + 1.0*seconds/86400;
}


// COMPARISON OPERATORS

bool ares::operator==(Date const& lhs, Date const& rhs)
{
    return lhs.compare(rhs) == 0;
}

bool ares::operator!=(Date const& lhs, Date const& rhs)
{
    return !(lhs == rhs);
}

bool ares::operator< (Date const& lhs, Date const& rhs)
{
    return lhs.compare(rhs) < 0;
}

bool ares::operator<=(Date const& lhs, Date const& rhs)
{
    return lhs.compare(rhs) <= 0;
}

bool ares::operator> (Date const& lhs, Date const& rhs)
{
    return !(lhs <= rhs);
}

bool ares::operator>=(Date const& lhs, Date const& rhs)
{
    return !(lhs < rhs);
}


// OUTPUT OPERATOR

ostream& ares::operator<<(ostream& o, Date const& d)
{
    return o << d.to_string();
}
