/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2014, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file RTime.h
/// Provides classes for accessing and working with dates and times.
/////////////////////////////////////////////////////////////////////

#ifndef RC_TIME_H
#define RC_TIME_H


#include "Types.h"
#include "RStr.h"
#include "Data1D.h"
#include <ctime>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#endif


namespace RC {
  /// Accesses and formats the system date and time, and provides high
  /// precision timing.
  class Time {
    public:

    /// Start the timer.
    inline void Start() {
      proc_start_time = clock();
      real_start_time = Time::Get();
    }


    /// Default constructor.  Starts the timer.
    inline Time() {
      Start();
    }


    /// Returns seconds since start in real time.
    inline f64 SinceStart() {
      return (Time::Get() - real_start_time);
    }


    /// Returns seconds since start in process time.
    inline f64 ProcSinceStart() {
      clock_t now;

      now = clock();
      return ((f64(now-proc_start_time))/CLOCKS_PER_SEC);
    }


    protected:
    /// @cond PROTECTED

    clock_t proc_start_time;
    f64 real_start_time;

    static inline struct tm Epoch() {
      struct tm tmval;
      tmval.tm_sec = tmval.tm_min = tmval.tm_hour = 0;
      tmval.tm_mday = 1;
      tmval.tm_mon = 0;
      tmval.tm_year = 70;
      tmval.tm_wday = 4;
      tmval.tm_yday = 0;
      tmval.tm_isdst = 0;
      return tmval;
    }

    /// Returns the amount time_t changes for one second.
    static inline f64 time_t_ScaleFactor() {
      static bool set = false;
      static f64 factor;
      if (!set) {
        struct tm tmval = Epoch();
        time_t epoch, plus1;
        epoch = mktime(&tmval);
        tmval.tm_sec = 1;
        plus1 = mktime(&tmval);
        factor = plus1 - epoch;
        set = true;
      }
      return factor;
    }

    static inline f64 time_t_Offset() {
      static bool set = false;
      static f64 offset;
      if (!set) {
        struct tm tmval = Epoch();
        tmval.tm_isdst = Get_tm().tm_isdst;
        time_t epoch = mktime(&tmval);
        offset = epoch / time_t_ScaleFactor() + GetTimezone();
        set = true;
      }
      return offset;
    }

    /// @endcond
    public:

    /// Returns seconds since 1970-01-01 epoch to a precision between
    /// 100 nanoseconds and 1 microsecond.
    static inline f64 Get() {
      f64 retval;
#ifdef WIN32
      i64 time_100ns;
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      time_100ns = i64(
        (((u64(ft.dwHighDateTime))<<u64(32)) + ft.dwLowDateTime));
      time_100ns -= 116444736000000000ull;  // 1601 epoch to 1970 epoch
      retval = (f64(time_100ns)) * 1e-7L;
#else // POSIX
      struct timespec tp;
      clock_gettime(CLOCK_REALTIME, &tp);
      retval = tp.tv_sec + 1e-9L*tp.tv_nsec;
#endif
      return retval;
    }

    /// Converts a time_t value into seconds from the 1970-01-01 epoch.
    static inline f64 Get(time_t tt) {
      return tt / time_t_ScaleFactor() - time_t_Offset();
    }

    /// Returns the seconds from the 1970-01-01 epoch for tmval.
    static inline f64 Get(struct tm tmval) {
      return Get_UTC(Local2UTC(tmval));
    }

    /// Given a UTC struct tm, returns the seconds since epoch portably.
    static inline f64 Get_UTC(struct tm tmval) {
      struct tm tmepoch = Epoch();
      // Actually gives UTC diff, since mktime local, and real epoch is UTC.
      return f64(difftime(mktime(&tmval), mktime(&tmepoch)));
    }


    /// Empirically determines the precision of Get(), and stores this value
    /// for future calls to this.
    /** @return The smallest amount by which Get() can increment.
     */
    static inline f64 GetPrecision() {
      static RC_THREAD_LOCAL f64 min = RC::MAX_VAL<f64>();
      if (min < RC::MAX_VAL<f64>()) { return min; }

      f64 last_t, t;
      t = RC::Time::Get();
      for (u32 tries=0; tries<10;) {
        last_t = t;
        t = RC::Time::Get();
        f64 diff = t - last_t;
        if (diff > 0) {
          tries++;
          min = (min <= diff) ? min : diff;
        }
      }
      return min;
    }


    /// Returns the number of seconds offset from UTC
    static inline i32 GetTimezone() {
      time_t tt = time(NULL);
      struct tm tmloc = Get_tm(tt);
      struct tm tmutc = Get_tm_UTC(tt);
      i32 offset = 3600*(tmloc.tm_hour - tmutc.tm_hour)
                   + 60*(tmloc.tm_min - tmutc.tm_min)
                   + tmloc.tm_sec - tmutc.tm_sec;
      if (offset > 3600*12) { offset -= 3600*24; }
      else if (offset <= -3600*12) { offset += 3600*24; }
      return offset;
    }

    /// Returns { hours, min, sec } offset from UTC.
    static inline Data1D<i32> GetTimezoneData() {
      Data1D<i32> retval(3);
      i32 offset = GetTimezone();
      retval[0] = offset / 3600;
      retval[1] = offset / 60 - 60*retval[0];
      retval[2] = offset - 3600*retval[0] - 60*retval[1];
      return retval;
    }

    /// Returns the local time given UTC time tmval.
    static inline struct tm Local2UTC(struct tm tmval) {
      struct tm retval = tmval;
      Data1D<i32> tz = GetTimezoneData();
      retval.tm_hour -= tz[0];
      retval.tm_min -= tz[1];
      retval.tm_sec -= tz[2];
      retval.tm_isdst = 0;
      mktime(&retval);
      return retval;
    }

    /// Returns the UTC time given local time tmval.
    static inline struct tm UTC2Local(struct tm tmval) {
      struct tm retval = tmval;
      Data1D<i32> tz = GetTimezoneData();
      retval.tm_hour += tz[0];
      retval.tm_min += tz[1];
      retval.tm_sec += tz[2];
      retval.tm_isdst = Get_tm().tm_isdst;
      mktime(&retval);
      return retval;
    }


    /// Returns time_t as number of seconds since 1970-01-01 00:00:00 UTC.
    static inline time_t Get_time_t() {
      return time(NULL);
    }

    /// Portably converts val as seconds since 1970-01-01 epoch to time_t.
    static inline time_t Get_time_t(f64 val) {
      return time_t((val + time_t_Offset()) * time_t_ScaleFactor());
    }
    /// Converts tmval in local timezone to time_t.
    static inline time_t Get_time_t(struct tm tmval) {
      return mktime(&tmval);
    }
    /// Converts tmval in UTC to time_t.
    static inline time_t Get_time_t_UTC(struct tm tmval) {
      return Get_time_t(UTC2Local(tmval));
    }


    /// Normalizes the input tmval.
    static inline void Normalize(struct tm &tmval) {
      if (mktime(&tmval) == -1) {
        Throw_RC_Error("Invalid calendar time");
      }
    }


    /// Provides a struct tm in the local timezone for the current time.
    static inline struct tm Get_tm() {
      return Get_tm(time(NULL));
    }
    /// Provides a struct tm in the current timezone for the given time_t.
    static inline struct tm Get_tm(time_t tt) {
      struct tm tmval;
      tzset();
#ifdef WIN32
      struct tm* tmptmval;
      tmptmval = localtime(&tt);
      if (tmptmval != NULL) {
        tmval = *tmptmval;
      }
#else
      localtime_r(&tt, &tmval);
#endif

      return tmval;
    }
    /// Provides a struct tm in the local timezone for val seconds from
    /// the 1970-01-01 epoch.
    static inline struct tm Get_tm(f64 val) {
      return Get_tm(Get_time_t(val));
    }


    /// Provides a struct tm in UTC for the current time.
    static inline struct tm Get_tm_UTC() {
      return Get_tm_UTC(time(NULL));
    }
    /// Provides a struct tm in UTC for the given time_t.
    static inline struct tm Get_tm_UTC(time_t tt) {
      struct tm tmval;
      tzset();
#ifdef WIN32
      struct tm* tmptmval;
      tmptmval = gmtime(&tt);
      if (tmptmval != NULL) {
        tmval = *tmptmval;
      }
#else
      gmtime_r(&tt, &tmval);
#endif

      return tmval;
    }

    /// Provides a struct tm in the local timezone for val seconds from
    /// the 1970-01-01 epoch.
    static inline struct tm Get_tm_UTC(f64 val) {
      return Get_tm_UTC(Get_time_t(val));
    }

    /// Returns a normalized struct tm.
    static inline struct tm Get_tm(struct tm tmval) {
      Normalize(tmval);
      return tmval;
    }


    /// Provides the current time in the format "Fri Aug  1 17:05:25 2014"
    static inline RStr GetStr() {
      return GetStr(Get_tm());
    }

    /// Provides the time in the format "Fri Aug  1 17:05:25 2014"
    static inline RStr GetStr(struct tm tmval) {
      mktime(&tmval);
#ifdef WIN32
      char *ascval;
      ascval = asctime(&tmval);
#else
      char ascval[256];
      asctime_r(&tmval, ascval);
#endif

      RStr retval = ascval;
      retval.Chomp();

      return retval;
    }

    /// Provides the UTC time in the format "Fri Aug  1 17:05:25 2014"
    static inline RStr GetStr_UTC() {
      return GetStr(Get_tm_UTC());
    }


    /// Provides the local time as a string with the format processed by
    /// strftime.
    static inline RStr GetStr(const RStr& format) {
      return GetStr(format, Get_tm());
    }
    /// Provides the tmval as a string with the format processed by strftime.
    static inline RStr GetStr(const RStr& format, struct tm tmval) {
      char ascval[2048];
      mktime(&tmval);  // Must sanitize invalid values or strftime segfaults.
      if (strftime(ascval, 2048, format.c_str(), &tmval) == 0) {
        return RStr();
      }
      else {
        return RStr(ascval);
      }
    }

    /// Provides the UTC time as a string with the format processed by
    /// strftime.
    static inline RStr GetStr_UTC(const RStr& format) {
      return GetStr(format, Get_tm_UTC());
    }

    /// Returns a local date string formatted like "2011-07-28"
    static inline RStr GetDate() {
      return GetStr("%Y-%m-%d");
    }
    /// Returns a date string for tmval formatted like "2011-07-28"
    static inline RStr GetDate(struct tm tmval) {
      return GetStr("%Y-%m-%d", tmval);
    }
    /// Returns a UTC date string formatted like "2011-07-28"
    static inline RStr GetDate_UTC() {
      return GetStr_UTC("%Y-%m-%d");
    }

    /// Returns a local time string formatted like "19:49:18"
    static inline RStr GetTime() {
      return GetStr("%H:%M:%S");
    }
    /// Returns a time string for tmval formatted like "19:49:18"
    static inline RStr GetTime(struct tm tmval) {
      return GetStr("%H:%M:%S", tmval);
    }
    /// Returns a UTC time string formatted like "19:49:18"
    static inline RStr GetTime_UTC() {
      return GetStr_UTC("%H:%M:%S");
    }

    /// Returns a local date-time string formatted like "2011-07-28_19-49-18",
    /// which is suitable for filenames.
    static inline RStr GetDateTime() {
      return GetStr("%Y-%m-%d_%H-%M-%S");
    }
    /// Returns a date-time string from tmval formatted like
    /// "2011-07-28_19-49-18", which is suitable for filenames.
    static inline RStr GetDateTime(struct tm tmval) {
      return GetStr("%Y-%m-%d_%H-%M-%S", tmval);
    }
    /// Returns a UTC date-time string formatted like "2011-07-28_19-49-18",
    /// which is suitable for filenames.
    static inline RStr GetDateTime_UTC() {
      return GetStr_UTC("%Y-%m-%d_%H-%M-%S");
    }

    /// Parses a date string like 2011-07-28, 2011-7-28, 07-28, 7-28, or
    /// "/" instead of "-", into base tmval.
    static inline struct tm ParseDate(const RStr& date, struct tm tmval) {
      Data1D<RStr> parts = date.SplitAny("-/");
      if (parts.size() < 2) { Throw_RC_Error("Invalid date"); }
      tmval.tm_sec = tmval.tm_min = tmval.tm_hour = 0;
      tmval.tm_isdst = -1;
      if (parts.size() < 3) {
        tmval.tm_year = Get_tm().tm_year;
        tmval.tm_mon  = parts[0].Get_i32(10) - 1;
        tmval.tm_mday = parts[1].Get_i32(10);
      }
      else {
        tmval.tm_year = parts[0].Get_i32(10) - 1900;
        tmval.tm_mon  = parts[1].Get_i32(10) - 1;
        tmval.tm_mday = parts[2].Get_i32(10);
      }
      mktime(&tmval);
      return tmval;
    }
    /// Parses a date string like 2011-07-28, 2011-7-28, 07-28, 7-28, or
    /// "/" instead of "-", into base local time.
    static inline struct tm ParseDate(const RStr& date) {
      return ParseDate(date, Get_tm());
    }
    /// Parses a date string like 2011-07-28, 2011-7-28, 07-28, 7-28, or
    /// "/" instead of "-", into base UTC time.
    static inline struct tm ParseDate_UTC(const RStr& date) {
      return ParseDate(date, Get_tm_UTC());
    }

    /// Parses a time string like 19:49:18, 19:49, or "-" instead of ":",
    /// into base tmval.
    static inline struct tm ParseTime(const RStr& date, struct tm tmval) {
      Data1D<RStr> parts = date.SplitAny("-:");
      if (parts.size() < 2) { Throw_RC_Error("Invalid time"); }
      tmval.tm_hour = parts[0].Get_i32(10);
      tmval.tm_min  = parts[1].Get_i32(10);
      if (parts.size() < 3) {
        tmval.tm_sec = 0;
      }
      else {
        tmval.tm_sec = parts[2].Get_i32(10);
      }
      mktime(&tmval);
      return tmval;
    }
    /// Parses a time string like 19:49:18, 19:49, or "-" instead of ":",
    /// into base local time.
    static inline struct tm ParseTime(const RStr& date) {
      return ParseTime(date, Get_tm());
    }
    /// Parses a time string like 19:49:18, 19:49, or "-" instead of ":",
    /// into base UTC time.
    static inline struct tm ParseTime_UTC(const RStr& date) {
      return ParseTime(date, Get_tm_UTC());
    }

    /// Parses a string like ParseDate and ParseTime separated by "_" or " ",
    /// into base tmval.
    static inline struct tm ParseDateTime(const RStr& date, struct tm tmval) {
      Data1D<RStr> parts = date.SplitAny("_ ");
      if (parts.size() < 2) { Throw_RC_Error("Invalid date-time"); }
      return ParseTime(parts[1], ParseDate(parts[0], tmval));
    }
    /// Parses a string like ParseDate and ParseTime separated by "_" or " ",
    /// into base local time.
    static inline struct tm ParseDateTime(const RStr& date) {
      return ParseDateTime(date, Get_tm());
    }
    /// Parses a string like ParseDate and ParseTime separated by "_" or " ",
    /// into base UTC time.
    static inline struct tm ParseDateTime_UTC(const RStr& date) {
      return ParseDateTime(date, Get_tm_UTC());
    }


    /// Sleeps for a floating point number of seconds, with sub-second
    /// precision to the limit of the system.
    static inline void Sleep(f64 seconds) {
      if (seconds <= 0) {
        return;
      }
#ifdef WIN32
      ::Sleep(seconds*1000);
#else // POSIX
      struct timespec req;
      req.tv_sec = time_t(seconds);
      req.tv_nsec = long(1e9*(seconds-u64(seconds)));
      nanosleep(&req, NULL);
#endif
    }
  };


  /// A class which obeys periodic time-of-day boundaries, and can manage
  /// times being within a daily time frame.
  class TimeOfDay {
    protected:
    /// @cond PROTECTED
    u32 sec;

    inline void Wrap() {
      sec = sec % 86400;
    }
    /// @endcond
    public:
    /// Default constructor, initializes to local time.
    inline TimeOfDay() {
      (*this) = Time::Get_tm();
    }

    /// Initializes to tmval.
    inline TimeOfDay(struct tm tmval) {
      (*this) = tmval;
    }

    /// Initializes to the given hr, min, sec from midnight.
    inline TimeOfDay(u32 hr, u32 min=0, u32 sec=0)
      : sec(sec + 60*min + 3600*hr) {
      Wrap();
    }

    /// Initializes to tmval.
    inline TimeOfDay& operator=(struct tm tmval) {
      sec = tmval.tm_sec + 60*tmval.tm_min + 3600*tmval.tm_hour;
      Wrap();
      return *this;
    }

    /// Subtracts another TimeOfDay amount, obeying periodic boundaries.
    inline TimeOfDay& operator-=(const TimeOfDay& other) {
      if (sec < other.sec) {
        sec += 86400;
      }
      sec -= other.sec;
      Wrap();
      return *this;
    }

    /// Subtracts two TimeOfDay amounts, obeying periodic boundaries.
    inline TimeOfDay operator-(const TimeOfDay& other) const {
      TimeOfDay retval(*this);
      retval -= other;
      return retval;
    }

    /// Adds another TimeOfDay amount, obeying periodic boundaries.
    inline TimeOfDay& operator+=(const TimeOfDay& other) {
      sec += other.sec;
      Wrap();
      return *this;
    }

    /// Adds two TimeOfDay amounts, obeying periodic boundaries.
    inline TimeOfDay operator+(const TimeOfDay& other) const {
      TimeOfDay retval(*this);
      retval += other;
      return retval;
    }

    /// Creates TimeOfDay comparitors.
#define TIMEOFDAY_COMP(OP) \
    /** \brief True if this TimeOfDay is OP the other. */ \
    inline bool operator OP \
    (const TimeOfDay& other) const {\
      return sec OP other.sec;\
    }
    TIMEOFDAY_COMP(<)
    TIMEOFDAY_COMP(<=)
    TIMEOFDAY_COMP(>)
    TIMEOFDAY_COMP(>=)
    TIMEOFDAY_COMP(==)


    /// True if this TimeOfDay is later than left and earlier than right,
    /// obeying periodic boundaries.
    inline bool Between(const TimeOfDay& left, const TimeOfDay& right) const {
      u32 thissec = ((*this) - left).sec;
      u32 rightsec = (right - left).sec;
      return (thissec < rightsec);
    }

    /// Returns the 0-23 hour value after midnight.
    inline u32 Hr() const { return sec / 3600; }
    /// Returns the 0-59 minute value after midnight.
    inline u32 Min() const { return (sec / 60) % 60; }
    /// Returns the 0-59 second value after midnight.
    inline u32 Sec() const { return sec % 60; }

    /// Returns the total seconds after midnight.
    inline u32 AsSeconds() const { return sec; }
    /// Returns the total minutes after midnight.
    inline u32 AsMinutes() const { return sec / 60; }
    /// Returns the total hours after midnight.
    inline u32 AsHours() const { return sec / 3600; }

    /// Returns a formatted string representation.
    inline RStr ToString(bool AmPm=false) const {
      return RStr(AmPm ? (Hr()+11)%12+1 : Hr()).PadLeft(2, '0') + ":" +
             RStr(Min()).PadLeft(2, '0') + ":" +
             RStr(Sec()).PadLeft(2, '0') +
             (AmPm ? (Hr() < 12 ? "am" : "pm") : "");
    }

    /// Initializes from a number of seconds after midnight.
    inline void FromSeconds(u32 newsec) { sec = newsec; Wrap(); }
    /// Initializes from a number of minutes after midnight.
    inline void FromMinutes(u32 newmin) { sec = newmin * 60; Wrap(); }
  };
}


#endif // RC_TIME_H

