// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "ares/job/interval.hpp"

ares::job::Interval::Interval()
        : m_precision(0)    // indicates the the interval is null
{}

ares::job::Interval::Interval(char precision, double num_days)
        : m_num_days(num_days)
        , m_precision(precision)
{}

ares::job::Interval::Interval(Interval const& interval)
        : m_num_days(interval.m_num_days)
        , m_precision(interval.m_precision)
{}

ares::job::Interval& ares::job::Interval::operator=(Interval const& rhs)
{
    m_num_days = rhs.m_num_days;
    m_precision = rhs.m_precision;
    return *this;
}

ares::Date ares::job::Interval::next_date(Date date) const
{
    return is_null() ? date : date.trunc(m_precision) + m_num_days;
}

bool ares::job::Interval::is_null() const
{
    return m_precision == 0;
}
