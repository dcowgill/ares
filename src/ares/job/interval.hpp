// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_job_interval
#define included_ares_job_interval

#include "ares/date.hpp"

namespace ares { namespace job {

// Specifies an interval between times a job should be run. The algorithm used
// to apply an interval is simple: after running a job, the scheduler
// truncates the current date and time using the precision specified by the
// interval, then adds the number of days specified by the interval to the
// truncated date and time. The new date is the next run date of the job. If
// the interval is null (that is, its precision is zero), the job will not be
// scheduled to run again.
//
// This simple algorithm allows us to specify relatively complex schedules:
//
//      "Tomorrow at 1300 GMT."
//      Precision is 'J' (day), days is 1+13/24.
//
//      "Five hours from now."
//      Precision is 'I' (minute), days is 5/24.
//
//      "Next monday at 0300 GMT."
//      Precision is 'D' (the Sunday of the current week), days is 8+3/24.
//
// Note that this approach cannot support a schedule such as "every minute
// between 0200 and 0500 on Tuesdays and Saturdays," which would be trivial
// using a scheduler such as cron.
class Interval {
  public:
    // Constructs a "null" interval, that is, an invalid interval that should
    // not be used to compute future dates. The Interval::is_null function
    // will return true for this interval.
    Interval();

    // Constructs a new interval. Unless the precision is zero, the
    // Interval::is_null function will return false for this interval.
    Interval(char precision, double num_days);

    // Copy constructor.
    Interval(const Interval& interval);

    // Assignment operator.
    Interval& operator=(const Interval& rhs);

    // Adds this interval to the given date and returns the result. If the
    // interval is null, the date is returned unchanged.
    Date next_date(Date date) const;

    // Returns true if this interval is null, that is, it does not correspond
    // to a valid interval of time. If an interval is null, it should not be
    // used to schedule a new job (instead, the job should be run just once).
    bool is_null() const;

  private:
    double m_num_days;      // amount to add to truncated date
    char m_precision;       // precision to use when truncating date
};

} } // namespace ares::job

#endif
