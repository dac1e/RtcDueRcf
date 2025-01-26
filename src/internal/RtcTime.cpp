#include "RtcTime.h"

#include "core-sam-GapClose.h"

#define MEASURE_Sam3XA_RtcTime_isdst false
#define ASSERT_Sam3XA_RtcTime_isdst true

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

inline int expiredSecondsWithinDay(const Sam3XA::RtcTime& rtcTime) {
  return ((rtcTime.tm_hour() * 60) + rtcTime.tm_min()) * 60 + rtcTime.tm_sec();
}

int hasTransitionedDstRule(const Sam3XA::RtcTime& rtcTime, const __tzrule_struct* const tzrule,
    int normalTimeToDstDifference) {
  int result = 0;
  if(rtcTime.month() >= tzrule->m) {
    if(rtcTime.month() == tzrule->m) {
      // The RTC contains local standard time. So calculate the local standard time
      // of the daylight transition.
      // Note: When transition happens to daylight saving time, the there is no
      // difference between local standard time and daylight saving time.
      int d = tzrule->d;
      if(normalTimeToDstDifference != 0) {
        const int localStandardTimeDaylightTransition = tzrule->s + normalTimeToDstDifference;
        if(localStandardTimeDaylightTransition < 0) {
          // The standard time of the daylight transition is the predecessor day
          // of daylight saving time dst transition. This will typically
          // not happen when the shift is from 3:00h dst to 2:00 std
          d = (++d % 7);
          normalTimeToDstDifference += 24 * 60 * 60;
        }
      }

      const int wdayOccuranceInMonth = calcWdayOccurranceInMonth(d, rtcTime);
      const bool dayMatch = (wdayOccuranceInMonth >= tzrule->n) ||
        (tzrule->n >= 5 && isLastWdayWithinMonth(tzrule->d, rtcTime));

      if(dayMatch) {

        if(expiredSecondsWithinDay(rtcTime) - normalTimeToDstDifference >= tzrule->s) {
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
#if MEASURE_Sam3XA_RtcTime_isdst
  const uint32_t s = micros();
#endif
  int result = 0;
  if(_daylight) {
    const __tzinfo_type * const tz = __gettzinfo ();
    const __tzrule_struct* const tzrule_DstBegin = &tz->__tzrule[not tz->__tznorth];
    const __tzrule_struct* const tzrule_DstEnd = &tz->__tzrule[tz->__tznorth];

    if(hasTransitionedDstRule(rtcTime, tzrule_DstBegin, 0)) {
      // Unit of the offsets is seconds.
      const int stdToDstDifference = rtcTime.rtc12hrsMode() ? 0 : tzrule_DstEnd->offset - tzrule_DstBegin->offset;
      result = not hasTransitionedDstRule(rtcTime, tzrule_DstEnd, stdToDstDifference);
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

  assert(time.tm_isdst >= 0);
  mRtc12hrsMode = time.tm_isdst;
  mYear = rtcYear(time);
  mMonth = rtcMonth(time);
  mDayOfMonth = time.tm_mday;
  mDayOfWeekDay = rtcDayOfWeek(time);
}

std::time_t RtcTime::get(std::tm &time) const {
  time.tm_isdst = mIsFromRTC ? 0 : mRtc12hrsMode;
  time.tm_hour = tm_hour();
  time.tm_min = tm_min();
  time.tm_sec = tm_sec();
  time.tm_year = tm_year();
  time.tm_mon = tm_mon();
  time.tm_mday = tm_mday();
  time.tm_wday = tm_wday();
  time.tm_yday = -1;

  // Get UTC from RTC time.
  std::time_t result = mktime(&time);

  if(mIsFromRTC && rtc12hrsMode()) {
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
  readFromRtc();
  const bool is12hrsMode = rtc12hrsMode();
  const bool isDst = isdst();
  if (isDst != is12hrsMode) {
    // Change RTC to carry daylight savings time so that alarms will be adjusted.
    get(time);
    result = true;
#if RTC_DEBUG_HOUR_MODE
  Serial.print(__FUNCTION__);
  if(is12hrsMode) {
    Serial.println(" to 24-hrs mode.");
  } else {
    Serial.println(" to 12-hrs mode.");
  }
#endif
  }
  return result;
}

void RtcTime::writeToRtc() const {
  RTC_SetTimeAndDate(RTC, hour(), minute(), second(), year(),
      month(), day(), day_of_week());
  // In order to detect whether RTC carries daylight savings time or
  // standard time, 12-hrs mode of RTC is applied, when RTC carries
  // daylight savings time.
  RTC_SetHourMode(RTC, rtc12hrsMode());
}

void RtcTime::readFromRtc() {
  mRtc12hrsMode = ::RTC_GetTimeAndDate(RTC, nullptr,
    &mHour, &mMinute, &mSecond, &mYear, &mMonth,
    &mDayOfMonth, &mDayOfWeekDay);
  mIsFromRTC = 1;
#if RTC_DEBUG_HOUR_MODE
  Serial.print(__FUNCTION__);
  if(rtc12hrsMode()) {
    Serial.println(" @RTC 12-hrs mode.");
  } else {
    Serial.println(" @RTC 24-hrs mode.");
  }
#endif
}

} // namespace Sam3XA_Rtc
