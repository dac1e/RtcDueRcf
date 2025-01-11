#include "core-sam-GapClose.h"
#include "Sam3XA_RtcTime.h"

void Sam3XA_RtcTime::set(const std::tm &time) {
  mHour = time.tm_hour;
  mMinute = time.tm_min;
  mSecond = time.tm_sec;
  mYear = rtcYear(time);
  mMonth = rtcMonth(time);
  mDay = time.tm_mday;
  mWeekDay = rtcDayOfWeek(time);
}

time_t Sam3XA_RtcTime::get(std::tm &time) const {
  time.tm_hour = tmHour();
  time.tm_min = tmMinute();
  time.tm_sec = tmSecond();
  time.tm_year = tmYear();
  time.tm_mon = tmMonth();
  time.tm_mday = tmDay();
  time.tm_wday = tmWeek();
  time.tm_yday = 0;
  time.tm_isdst = 0;
  // mktime will fix the tm_yday as well as tm_ist and the
  // hour, if time is within daylight savings period.
  return mktime(&time);
}

void Sam3XA_RtcTime::readFromRtc() {
  ::RTCgapclose_GetTimeAndDate(RTC, &mHour, &mMinute, &mSecond, &mYear, &mMonth,
      &mDay, &mWeekDay);
}

