/*
  RtcSam3XA - Arduino libary for RtcSam3XA - builtin RTC Copyright (c)
  2024 Wolfgang Schmieder.  All right reserved.

  Contributors:
  - Wolfgang Schmieder

  Project home: https://github.com/dac1e/RcSwitchReceiver/

  This library is free software; you can redistribute it and/or modify it
  the terms of the GNU Lesser General Public License as under published
  by the Free Software Foundation; either version 3.0 of the License,
  or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "core-sam-GapClose.h"
#include "RtcTime.h"

#ifndef RTC_DEBUG_HOUR_MODE
  #define RTC_DEBUG_HOUR_MODE true
#endif

#ifndef MEASURE_Sam3XA_RtcTime_isdst
  #define MEASURE_Sam3XA_RtcTime_isdst false
#endif

#ifndef MEASURE_Sam3XA_RtcTime_isdst
  #define ASSERT_Sam3XA_RtcTime_isdst  false
#endif

#if ASSERT_Sam3XA_RtcTime_isdst
#include <assert.h>
#endif

#if MEASURE_Sam3XA_RtcTime_isdst || RTC_DEBUG_HOUR_MODE
#include "Arduino.h"
#endif

namespace {

constexpr int SECSPERMIN = 60;
constexpr int SECSPERHOUR = SECSPERMIN * 60;
constexpr int32_t SECSPERDAY = SECSPERHOUR * 24;

const int month_lengths[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
} ;

const int month_yday[2][12] = {
  {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333},
  {-1, 30, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
} ;

inline int isLeapYear(uint16_t rtc_year) {
  const int result = ( not (rtc_year % 4) && ( (rtc_year % 100) || not(rtc_year % 400) ) );;
  return result;
}

int calcWdayOccurranceInMonth(int tm_wday, const Sam3XA::RtcTime& rtcTime) {
  const int wdayOffset = tm_wday - rtcTime.tm_wday();
  return (rtcTime.tm_mday() - 1 - wdayOffset) / 7 + 1;
}

inline int calcMdayOfNextWdayOccurance(int tm_wday, const Sam3XA::RtcTime& rtcTime) {
  const int daysUntilNextOccurranceOfWday = 7 - (tm_wday - rtcTime.tm_wday());
  return rtcTime.tm_mday() + daysUntilNextOccurranceOfWday;
}

inline bool isMdayValidWithinMonth(int tm_mday, const Sam3XA::RtcTime& rtcTime) {
  return tm_mday <= month_lengths[isLeapYear(rtcTime.year())][rtcTime.tm_mon()];
}

inline bool isLastWdayWithinMonth(int tm_wday, const Sam3XA::RtcTime& rtcTime) {
  return not isMdayValidWithinMonth(calcMdayOfNextWdayOccurance(tm_wday, rtcTime), rtcTime);
}

inline int32_t expiredSecondsWithinDay(const Sam3XA::RtcTime& rtcTime) {
  return ((rtcTime.tm_hour() * 60) + rtcTime.tm_min()) * 60 + rtcTime.tm_sec();
}

/**
 * rtcAheadTime is 0 or positive when checking against dst begin rule.
 * rtcAheadTime is 0 or negative when checking against dst end rule.
 */
int hasTransitionedDstRule(const Sam3XA::RtcTime& rtcTime, const __tzrule_struct* const tzrule,
    int32_t rtcAheadTime) {
  int result = 0;
  if(rtcTime.month() >= tzrule->m) {
    if(rtcTime.month() == tzrule->m) {
      // The RTC contains local standard time. So calculate the local standard time
      // of the daylight transition.
      // Note: When transition happens to daylight saving time, the there is no
      // difference between local standard time and daylight saving time.
      int d = tzrule->d;
      int32_t rule_s = tzrule->s + rtcAheadTime;
      if(rule_s < 0) {
        rule_s = 0;
      } else if (rule_s >= SECSPERDAY) {
        rule_s = SECSPERDAY - 1;
      }

      const int wdayOccuranceInMonth = calcWdayOccurranceInMonth(d, rtcTime);
      const bool dayMatch = (wdayOccuranceInMonth >= tzrule->n) ||
        (tzrule->n >= 5 && isLastWdayWithinMonth(tzrule->d, rtcTime));

      if(dayMatch) {
        if(expiredSecondsWithinDay(rtcTime) >= rule_s) {
          result = 1;
        }
      }
    } else {
      result = 1;
    }
  }
  return result;
}

inline int isdst_(const Sam3XA::RtcTime& rtcTime) {
  if(_daylight) {
#if MEASURE_Sam3XA_RtcTime_isdst
    const uint32_t s = micros();
#endif
    const __tzinfo_type * const tz = __gettzinfo ();
    const __tzrule_struct* const tzrule_DstBegin = &tz->__tzrule[not tz->__tznorth];
    const __tzrule_struct* const tzrule_DstEnd = &tz->__tzrule[tz->__tznorth];

    const bool isRtcDst = rtcTime.rtc12hrsMode(); // is RTC carrying daylight savings time ?

    int result = tzrule_DstBegin->m >= tzrule_DstEnd->m;

    const int32_t rtcAheadTimeForDstBeginCheck = isRtcDst ? tzrule_DstEnd->offset - tzrule_DstBegin->offset : 0;
    if(not result)
    {
      // North hemisphere
      // rtcAheadTimeForDstBeginCheck may be positive or -1.
      if(hasTransitionedDstRule(rtcTime, tzrule_DstBegin, rtcAheadTimeForDstBeginCheck - 1)) {
        // Unit of the offsets is seconds.
        const int rtcAheadTimeForDstEndCheck = isRtcDst ? 0 : tzrule_DstEnd->offset - tzrule_DstBegin->offset;
        // rtcAheadTimeForDstEndCheck may be 0 or negative.
        result = not hasTransitionedDstRule(rtcTime, tzrule_DstEnd, rtcAheadTimeForDstEndCheck);
      }
    } else {
      // South hemisphere
      // rtcAheadTimeForDstBeginCheck may be positive or -1.
      if(not hasTransitionedDstRule(rtcTime, tzrule_DstBegin, rtcAheadTimeForDstBeginCheck) - 1) {
        // Unit of the offsets is seconds.
        const int rtcAheadTimeForDstEndCheck = isRtcDst ? 0 : tzrule_DstEnd->offset - tzrule_DstBegin->offset;
        // rtcAheadTimeForDstEndCheck may be 0 or negative.
        result = not hasTransitionedDstRule(rtcTime, tzrule_DstEnd, rtcAheadTimeForDstEndCheck);
      }
    }
#if MEASURE_Sam3XA_RtcTime_isdst
    const uint32_t d = micros() - s;
    Serial.print("isdst duration: ");
    Serial.print(d);
    Serial.println("usec");
#endif
    return result;
  }
  return 0;
}

/**
 * Get the time difference between standard time and daylight savings in seconds.
 */
int stdToDstDiff() {
  const __tzinfo_type * const tz = __gettzinfo ();
  const __tzrule_struct* const tzrule_DstBegin = &tz->__tzrule[not tz->__tznorth];
  const __tzrule_struct* const tzrule_DstEnd = &tz->__tzrule[tz->__tznorth];
  // Unit of the offsets is seconds.
  return tzrule_DstEnd->offset - tzrule_DstBegin->offset;
}

/**
 * Calculate the number of leap years since 1970 for a given year.
 * @param tm_year The offset to 1900 of the year to be used for
 *  the calculation. This is the same format as the tm_year part
 *  of the std::tm;
 * @return 1 if the given year is a leap year. Otherwise 0.
 */
int leapYearsSince1970 (int tm_year) {
  const int year = tm_year + 1900;
  const int yearsDiv4count   = (year-1968 /* first year that divides by   4 without rest. */) /   4;
  const int yearsDiv100count = (year-1900 /* first year that divides by 100 without rest. */) / 100;
  const int yearsDiv400count = (year-1600 /* first year that divides by 400 without rest. */) / 400;
  return yearsDiv4count - yearsDiv100count + yearsDiv400count;
}

#if 0
inline int yday(const Sam3XA::RtcTime& rtcTime) {
  return month_yday[rtcTime.tm_mon()] + rtcTime.day();
}

std::time_t mkgmtime(const Sam3XA::RtcTime& time) {
  using time_t = std::time_t;
  const bool leapYear = isLeapYear(time.year());
  const time_t leapYearsBeforeThisYear = leapYearsSince1970(time.year()) - leapYear;
  const time_t yearOffset = time.year() - 1970;
  const time_t result = time.second() + (time.minute() + (time.hour() + (yday(time) +
      leapYearsBeforeThisYear + yearOffset * 365) * 24) * 60) * 60;
  return result;
}



/* Move epoch from 01.01.1970 to 01.03.0000 (yes, Year 0) - this is the first
 * day of a 400-year long "era", right after additional day of leap year.
 * This adjustment is required only for date calculation, so instead of
 * modifying time_t value (which would require 64-bit operations to work
 * correctly) it's enough to adjust the calculated number of days since epoch.
 */
constexpr int32_t EPOCH_ADJUSTMENT_DAYS = 719468L;
/* 1st March of year 0 is Wednesday */
constexpr int ADJUSTED_EPOCH_WDAY = 3;
constexpr int DAYSPERWEEK = 7;

inline int tmWeekday(const std::time_t& lcltime) {
  long days = lcltime / SECSPERDAY + EPOCH_ADJUSTMENT_DAYS;
  long rem = lcltime % SECSPERDAY;
  if (rem < 0) {
    rem += SECSPERDAY;
    --days;
  }
  int weekday = ((ADJUSTED_EPOCH_WDAY + days) % DAYSPERWEEK);
  if (weekday < 0) {
    weekday += DAYSPERWEEK;
  }
  return weekday;
}

/* year to which the adjustment was made */
constexpr int ADJUSTED_EPOCH_YEAR = 0;
/* there are 97 leap years in 400-year periods. ((400 - 97) * 365 + 97 * 366) */
constexpr int32_t DAYS_PER_ERA = 146097L;
/* there are 24 leap years in 100-year periods. ((100 - 24) * 365 + 24 * 366) */
constexpr int32_t DAYS_PER_CENTURY = 36524L;
/* there is one leap year every 4 years */
constexpr int DAYS_PER_4_YEARS = 3 * 365 + 366;
/* number of days in a non-leap year */
constexpr int DAYS_PER_YEAR = 365;
/* number of days in January */
constexpr int DAYS_IN_JANUARY = 31;
/* number of days in non-leap February */
constexpr int DAYS_IN_FEBRUARY = 28;
/* number of years per era */
constexpr int YEARS_PER_ERA = 400;

constexpr int32_t YEAR_BASE = 1900;


Sam3XA::RtcTime* gmtime_r (const time_t& lcltime, Sam3XA::RtcTime* result)
{
  long days = lcltime / SECSPERDAY + EPOCH_ADJUSTMENT_DAYS;
  long rem = lcltime % SECSPERDAY;
  if (rem < 0) {
    rem += SECSPERDAY;
    --days;
  }

  // Compute hour, min, and sec
  result->mHour = (rem / SECSPERHOUR);
  rem %= SECSPERHOUR;
  result->mMinute = (rem / SECSPERMIN);
  result->mSecond = (rem % SECSPERMIN);

  // Compute day of week
  result->mDayOfWeekDay = tmWeekday(lcltime) + 1;

  // Compute year, month, day & day of year. For description of this algorithm see
  // http://howardhinnant.github.io/date_algorithms.html#civil_from_days
  const int era = (days >= 0 ? days : days - (DAYS_PER_ERA - 1)) / DAYS_PER_ERA;
  const unsigned long eraday = days - era * DAYS_PER_ERA;             /* [0, 146096] */
  const unsigned erayear = (eraday - eraday / (DAYS_PER_4_YEARS - 1) +
    eraday / DAYS_PER_CENTURY - eraday / (DAYS_PER_ERA - 1)) / 365;   /* [0, 399] */

  const unsigned yearday = eraday - (DAYS_PER_YEAR * erayear + erayear / 4 - erayear / 100); /* [0, 365] */
  const unsigned m = (5 * yearday + 2) / 153;           /* [0, 11] */
  result->mDayOfMonth = yearday - (153 * m + 2) / 5 + 1;   /* [1, 31] */

  {
    const unsigned month = m < 10 ? 2 : -10;
    result->mYear = ADJUSTED_EPOCH_YEAR + erayear + era * YEARS_PER_ERA + (month <= 1);;
    result->mMonth = month + 1;
  }

  return result;
}

/**
 * Add seconds to an RtcTime structure.
 *
 * @param sec The seconds to be added.
 *
 * @return The sum of this time structure and the parameter 'sec'.
 */
tm add(const Sam3XA::RtcTime& rtcTime, const time_t sec) {
  std::time_t t = mkgmtime(rtcTime) + sec;
  tm result;
  gmtime_r(&t, &result);
  return result;
}

#endif

}


namespace Sam3XA {

uint8_t RtcTime::tmDayOfWeek(const std::tm &time) {
  /** Calling mktime will calculate and set tm_wday. */
  std::tm t = time;
  std::mktime(&t);
  return t.tm_wday;
}

int RtcTime::isdst() const {return isdst_(*this);}

void RtcTime::set(const std::tm &time) {
  mIsFromRTC = 0;
  mHour = time.tm_hour;
  mMinute = time.tm_min;
  mSecond = time.tm_sec;
#if ASSERT_Sam3XA_RtcTime_isdst
  assert(time.tm_isdst >= 0);
#endif
  mRtc12hrsMode = time.tm_isdst;
  mYear = rtcYear(time);
  mMonth = rtcMonth(time);
  mDayOfMonth = time.tm_mday;
  mDayOfWeekDay = rtcDayOfWeek(time);
}

void RtcTime::getRaw(std::tm &time) const {
  time.tm_isdst = -1;
  time.tm_hour = tm_hour();
  time.tm_min = tm_min();
  time.tm_sec = tm_sec();
  time.tm_year = tm_year();
  time.tm_mon = tm_mon();
  time.tm_mday = tm_mday();
  time.tm_wday = tm_wday();
  time.tm_yday = -1;
}

std::time_t RtcTime::get(std::tm &time) const {
  getRaw(time);
  time.tm_isdst = mIsFromRTC ? 0 : mRtc12hrsMode;

  // Get UTC from RTC time.
  std::time_t result = mktime(&time);

  if(mIsFromRTC && mRtc12hrsMode) {
    // RTC carries daylight savings time (typically 1 hour ahead).
    // That might even be the case if we are not in dst period.
    // Hence we must fix the UTC here.
    result += stdToDstDiff();
  }

  // Calculate local time from UTC. localtime_r will
  // also fix the tm_yday as well as tm_ist and the
  // hour, if time is within daylight savings period.
  localtime_r(&result, &time);

#if ASSERT_Sam3XA_RtcTime_isdst
  assert(isdst() == time.tm_isdst);
#endif

  return result;
}

bool RtcTime::dstRtcRequest(std::tm &time) {
  bool result = false;
  readFromRtc_();
  if(isdst()) {
    if (not mRtc12hrsMode) {
      result = true;
      getRaw(time);
      time.tm_isdst = 1;
      time.tm_hour -= stdToDstDiff() / 3600;
#if RTC_DEBUG_HOUR_MODE
      Serial.print(__FUNCTION__); Serial.print(", ");
      Serial.print(time.tm_hour); Serial.print(':');
      Serial.print(time.tm_min);  Serial.print(':');
      Serial.print(time.tm_sec);
      Serial.println(", request to 12-hrs mode.");
#endif
    }
  } else {
    if (mRtc12hrsMode) {
      result = true;
      getRaw(time);
      time.tm_isdst = 0;
      time.tm_hour += stdToDstDiff() / 3600;
#if RTC_DEBUG_HOUR_MODE
      Serial.print(__FUNCTION__); Serial.print(", ");
      Serial.print(time.tm_hour); Serial.print(':');
      Serial.print(time.tm_min);  Serial.print(':');
      Serial.print(time.tm_sec);
      Serial.println(", request to 24-hrs mode.");
#endif
    }
  }
  return result;
}

void RtcTime::writeToRtc() const {
  ::RTC_SetTimeAndDate(RTC, hour(), minute(), second(), year(),
      month(), day(), day_of_week());
  // In order to detect whether RTC carries daylight savings time or
  // standard time, 12-hrs mode of RTC is applied, when RTC carries
  // daylight savings time.
#if RTC_DEBUG_HOUR_MODE
  Serial.print(__FUNCTION__);
  if(mRtc12hrsMode) {
    Serial.println(", set to 12-hrs mode.");
  } else {
    Serial.println(", set to 24-hrs mode.");
  }
#endif
  RTC_SetHourMode(RTC, mRtc12hrsMode);
}

void RtcTime::readFromRtc() {
  mRtc12hrsMode = ::RTC_GetTimeAndDate(RTC, nullptr, &mHour, &mMinute, &mSecond, &mYear, &mMonth, &mDayOfMonth,
      &mDayOfWeekDay);
  mIsFromRTC = 1;
}

void RtcTime::readFromRtc_() {
  mRtc12hrsMode = ::RTC_GetTimeAndDate(RTC, nullptr, &mHour, &mMinute, &mSecond, &mYear, &mMonth, &mDayOfMonth,
      &mDayOfWeekDay);
  mIsFromRTC = 1;
#if RTC_DEBUG_HOUR_MODE
  Serial.print(__FUNCTION__);
  if(mRtc12hrsMode) {
    Serial.println(" @RTC 12-hrs mode.");
  } else {
    Serial.println(" @RTC 24-hrs mode.");
  }
#endif
}

} // namespace Sam3XA_Rtc
