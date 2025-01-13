#include "core-sam-GapClose.h"
#include "Sam3XA_RtcTime.h"

#define ASSERT_Sam3XA_RtcTime_isdst true
#define MEASURE_Sam3XA_RtcTime_isdst true

#if ASSERT_Sam3XA_RtcTime_isdst
#include <assert.h>
#endif

#if MEASURE_Sam3XA_RtcTime_isdst
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

int calcWdayOccurranceInMonth(int tm_wday, const Sam3XA_RtcTime& rtcTime) {
  const int wdayOffset = tm_wday - rtcTime.tmDayOfWeek();
  return (rtcTime.tmDayOfMonth() - 1 - wdayOffset) / 7 + 1;
}

inline int calcMdayOfNextWdayOccurance(int tm_wday, const Sam3XA_RtcTime& rtcTime) {
  const int daysUntilNextOccurranceOfWday = 7 - (tm_wday - rtcTime.tmDayOfWeek());
  return rtcTime.tmDayOfMonth() + daysUntilNextOccurranceOfWday;
}

inline bool isMdayValidWithinMonth(int tm_mday, const Sam3XA_RtcTime& rtcTime) {
  return tm_mday <= month_lengths[isLeapYear(rtcTime.year())][rtcTime.tmMonth()];
}

inline bool isLastWdayWithinMonth(int tm_wday, const Sam3XA_RtcTime& rtcTime) {
  return not isMdayValidWithinMonth(calcMdayOfNextWdayOccurance(tm_wday, rtcTime), rtcTime);
}

inline int expiredSecondsWithinDay(const Sam3XA_RtcTime& rtcTime) {
  return ((rtcTime.tmHour() * 60) + rtcTime.tmMinute()) * 60 + rtcTime.tmSecond();
}

int hasTransitionedDstRule(const Sam3XA_RtcTime& rtcTime, const __tzrule_struct* const tzrule,
    int dstshift) {
  int result = 0;
  if(rtcTime.month() >= tzrule->m) {
    if(rtcTime.month() == tzrule->m) {
      const int wdayOccuranceInMonth = calcWdayOccurranceInMonth(tzrule->d, rtcTime);
      const bool dayMatch = (wdayOccuranceInMonth >= tzrule->n) ||
        (tzrule->n >= 5 && isLastWdayWithinMonth(tzrule->d, rtcTime));
      if(dayMatch) {
        if(expiredSecondsWithinDay(rtcTime) - dstshift >= tzrule->s) {
          result = 1;
        }
      }
    } else {
      result = 1;
    }
  }
  return result;
}

inline int isdst(const Sam3XA_RtcTime& rtcTime) {
#if MEASURE_Sam3XA_RtcTime_isdst
  const uint32_t s = micros();
#endif

  int result = 0;
  if(_daylight) {
    const __tzinfo_type * const tz = __gettzinfo ();
    const __tzrule_struct* const tzrule_DstBegin = &tz->__tzrule[not tz->__tznorth];
    const __tzrule_struct* const tzrule_DstEnd = &tz->__tzrule[tz->__tznorth];

    if(hasTransitionedDstRule(rtcTime, tzrule_DstBegin, 0)) {
      const int dstshift = tzrule_DstEnd->offset - tzrule_DstBegin->offset;
      result = not hasTransitionedDstRule(rtcTime, tzrule_DstEnd,
        /* time shift at the end of dst should'nt be positive. However, limit to 0 */
        dstshift < 0 ? dstshift : 0);
    }
  }

#if MEASURE_Sam3XA_RtcTime_isdst
  const uint32_t d = micros() - s;
  Serial.print("isdst duration: ");
  Serial.println(d);
  Serial.print("usec");
#endif

  return result;
}

} // anonymous namespace

int Sam3XA_RtcTime::isdst() const {
  return ::isdst(*this);
}

void Sam3XA_RtcTime::set(const std::tm &time) {
  mHour = time.tm_hour;
  mMinute = time.tm_min;
  mSecond = time.tm_sec;
  mYear = rtcYear(time);
  mMonth = rtcMonth(time);
  mDayOfMonth = time.tm_mday;
  mDayOfWeekDay = rtcDayOfWeek(time);
}

std::time_t Sam3XA_RtcTime::get(std::tm &time) const {
  time.tm_hour = tmHour();
  time.tm_min = tmMinute();
  time.tm_sec = tmSecond();
  time.tm_year = tmYear();
  time.tm_mon = tmMonth();
  time.tm_mday = tmDayOfMonth();
  time.tm_wday = tmDayOfWeek();
  time.tm_yday = -1; // invalid
  time.tm_isdst = 0;
  // mktime will fix the tm_yday as well as tm_ist and the
  // hour, if time is within daylight savings period.
  std::time_t result = mktime(&time);
  localtime_r(&result, &time);

#if ASSERT_Sam3XA_RtcTime_isdst
  assert(isdst() == time.tm_isdst);
#endif

  return result;
}

void Sam3XA_RtcTime::readFromRtc() {
  ::RTCgapclose_GetTimeAndDate(RTC, &mHour, &mMinute, &mSecond, &mYear, &mMonth,
      &mDayOfMonth, &mDayOfWeekDay);
}

