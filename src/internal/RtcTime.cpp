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

const int month_lengths[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
} ;

inline int isLeapYear(int rtc_year) {
  const int result = ( !(rtc_year % 4) && ( (rtc_year % 100) || !(rtc_year % 400) ) );;
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
      int32_t constexpr SEC_PER_DAY = 60 * 60 * 24;
      if(rule_s < 0) {
        rule_s = 0;
      } else if (rule_s >= SEC_PER_DAY) {
        rule_s = SEC_PER_DAY - 1;
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

} // anonymous namespace

namespace Sam3XA {

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
