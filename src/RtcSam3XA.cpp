/*
 * RtcSam3XA.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include <assert.h>
#include "TM.h"
#include "RtcSam3XA.h"
#include "core-sam-GapClose.h"

namespace {

/**
 * Calculate whether a given year is a leap year.
 * @param tm_year The offset to 1900 of the year to be used for
 *  the calculation. This is the same format as the tm_year
 *  part of the std::tm;
 * @return the number of leap years since the given year.
 *  This number includes the given tm_year if it is a leap year.
 */
int isLeapYear(int tm_year) {
  const int year = tm_year + 1900;
  const int result = ( !(year % 4) && ( (year % 100) || !(year % 400) ) );;
  return result;
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

/**
 * Calculate the time difference between local daylight saving
 * time and local standard time.
 * @param time The tm structure that holds the tm_isdst flag to
 *  be evaluated for the return value.
 * @return The time difference in seconds if the tm_isdst flag
 *  in time is is > 0. Otherwise 0.
 */
std::time_t dstdiff(std::tm& time) {
  if(time.tm_isdst > 0) {
    const __tzinfo_type * tzinfo =  __gettzinfo();
    const int tz_offset = tzinfo->__tzrule[0].offset;
    const int tz_dstoffset = tzinfo->__tzrule[1].offset;
    const int dst_diff = tz_dstoffset - tz_offset;
    return dst_diff;
  }
  return 0;
}

/**
 * Convert time to a unix time stamp. Neither a time zone nor
 * a daylight saving shift is considered in this calculation.
 * Prerequisite: tm_yday and tm_isdst are set correctly.
 * @param time The time for which to calculate the time stamp.
 *
 * @return The unix time stamp for the given time.
 */
std::time_t mkgmtime(std::tm& time) {
  using time_t = std::time_t;
  const bool leapYear = isLeapYear(time.tm_year);
  const time_t leapYearsBeforeThisYear = leapYearsSince1970(time.tm_year) - leapYear;
  const time_t yearOffset = time.tm_year - 70;
  const time_t result = time.tm_sec + (time.tm_min + (time.tm_hour + (time.tm_yday +
      leapYearsBeforeThisYear + yearOffset * 365) * 24) * 60) * 60 + dstdiff(time);
  return result;
}

///** Set tm struct by Sam3XA RTC formatted struct. */
//std::time_t set(std::tm& time, const Sam3XA_RtcTime& sam2xatime) {
//  time.tm_hour = sam2xatime.tmHour(); time.tm_min = sam2xatime.tmMinute(); time.tm_sec = sam2xatime.tmSecond();
//  time.tm_year = sam2xatime.tmYear(); time.tm_mon = sam2xatime.tmMonth(); time.tm_mday = sam2xatime.tmDay();
//  time.tm_wday = sam2xatime.tmWeek(); time.tm_yday = 0; time.tm_isdst = 0 /* We get standard time from Rtc. */;
//
//  // mktime will fix the tm_yday as well as tm_ist and the
//  // hour, if time is within daylight savings period.
//  return mktime(&time);
//}

} // anonymous namespace

RtcSam3XA RtcSam3XA::clock;

void Sam3XA_RtcTime::readFromRtc() {
  ::RTCgapclose_GetTimeAndDate(RTC, &mHour, &mMinute, &mSecond, &mYear, &mMonth,
      &mDay, &mWeekDay);
}

void RTC_Handler (void) {
  RtcSam3XA::clock.RtcSam3XA_Handler();
}

RtcSam3XA::AlarmTime::AlarmTime() :
    second(UINT8_MAX), minute(UINT8_MAX), hour(UINT8_MAX), day(UINT8_MAX), month(
        UINT8_MAX) {
}

RtcSam3XA::RtcSam3XA()
  : mSetTimeRequest(false)
  , mSecondCallback(nullptr)
  , mSecondCallbackPararm(nullptr)
  , mAlarmCallback(nullptr)
  , mAlarmCallbackPararm(nullptr)
{
}

void RtcSam3XA::onSecTransitionInterrupt() {
  if (mSecondCallback) {
    (*mSecondCallback)(mSecondCallbackPararm);
  }
}

void RtcSam3XA::RtcSam3XA_Handler() {
  const uint32_t status = RTC->RTC_SR;

  /* Acknowledge for Update interrupt */
  if ((status & RTC_SR_ACKUPD) == RTC_SR_ACKUPD) {
    //    RTC_DisableIt(RTC, RTC_IDR_SECDIS);

    if (mSetTimeRequest) {
      ::RTCgapclose_SetTimeAndDate(RTC, mSetTimeCache.hour(),
          mSetTimeCache.minute(), mSetTimeCache.second(),
          mSetTimeCache.year(), mSetTimeCache.month(),
          mSetTimeCache.day(), mSetTimeCache.week());
      mSetTimeRequest = false;
    }

    RTC_ClearSCCR(RTC, RTC_SCCR_ACKCLR);
    //    RTC_EnableIt(RTC, RTC_IER_SECEN);
  }

  /* Second increment interrupt */
  if ((status & RTC_SR_SEC) == RTC_SR_SEC) {
    onSecTransitionInterrupt();
    RTC_ClearSCCR(RTC, RTC_SCCR_SECCLR);
  }

  /* Time or date alarm */
  if ((status & RTC_SR_ALARM) == RTC_SR_ALARM) {
    if(mAlarmCallback) {
      (*mAlarmCallback)(mAlarmCallbackPararm);
    }
    RTC_ClearSCCR(RTC, RTC_SCCR_ALRCLR);
  }
}

void RtcSam3XA::begin(const char* timezone, const RTC_OSCILLATOR source) {
  if(timezone != nullptr) {
    tzset(timezone);
  }

  if (source == XTAL) {
    pmc_switch_sclk_to_32kxtal(0);
    while (!pmc_osc_is_ready_32kxtal());
  }
  NVIC_DisableIRQ(RTC_IRQn);
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, 0);
  RTC_EnableIt(RTC, RTC_IER_SECEN | RTC_IER_ACKEN);
  NVIC_EnableIRQ(RTC_IRQn);
}

void RtcSam3XA::setByLocalTime(const std::tm &time) {
  RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  RTC_DisableIt(RTC, RTC_IER_ACKEN);

  // Call mktime in order to fix tm_yday, tm_isdst
  // and the hour, depending on whether the time is
  // within daylight saving period or not.
  std::tm buffer = time;
  (void)mktime(&buffer);

  // Calculate local standard time (I.e. local time
  // without daylight savings shift.)
  std::time_t localStandardTime = mkgmtime(buffer);
  gmtime_r(&localStandardTime, &buffer);

  // There might be set time request pending.
  // Disallow the RTC handler to pick it, while
  // updating the cache.
  RTC_DisableIt(RTC, RTC_IER_ACKEN);

  // Fill cache with standard time.
  mSetTimeCache.set(buffer);
  mSetTimeRequest = true;

  RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  RTC_EnableIt(RTC, RTC_IER_ACKEN);
}

void RtcSam3XA::setByUnixTime(std::time_t timestamp) {
  tm time;
  localtime_r(&timestamp, &time);
  setByLocalTime(time);
}

std::time_t RtcSam3XA::getLocalTime(std::tm &time) const {
  if (mSetTimeRequest) {
    return mSetTimeCache.get(time);
  }

  Sam3XA_RtcTime dueTimeAndDate;
  dueTimeAndDate.readFromRtc();
  return dueTimeAndDate.get(time);
}

time_t RtcSam3XA::getUnixTime() const {
  tm time;
  getLocalTime(time);
  return mktime(&time);
}

void RtcSam3XA::setAlarmCallback(void (*alarmCallback)(void*),
    void *alarmCallbackParam) {
  mAlarmCallback = alarmCallback;
  mAlarmCallbackPararm = alarmCallbackParam;
}

static inline void fillAlarmFraction(uint8_t& v) {
  if(v == UINT8_MAX) { v = 0; }
}

void RtcSam3XA::setAlarm(const AlarmTime& alarmTime) {
  AlarmTime a = alarmTime;

  // Fill all less significant values below the highest
  // significant valid value with 0, if not set. This
  // allows to adjust the alarm setting upon daylight
  // saving transition.
  if(alarmTime.month != UINT8_MAX) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(a.day);
    fillAlarmFraction(a.hour);
    fillAlarmFraction(a.minute);
    fillAlarmFraction(a.second);
  } else if(alarmTime.day != UINT8_MAX) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(a.hour);
    fillAlarmFraction(a.minute);
    fillAlarmFraction(a.second);
  } else if(alarmTime.day != UINT8_MAX) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(a.minute);
    fillAlarmFraction(a.second);
  } else if(alarmTime.day != UINT8_MAX) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(a.second);
  }

  const uint8_t* _hour = a.hour == UINT8_MAX ? nullptr : &a.hour;
  const uint8_t* _minute = a.minute == UINT8_MAX ? nullptr : &a.minute;
  const uint8_t* _second = a.second == UINT8_MAX ? nullptr : &a.second;
  const uint8_t* _day = a.day == UINT8_MAX ? nullptr : &a.day;
  const uint8_t* _month = a.month == UINT8_MAX ? nullptr : &a.month;

  RTC_DisableIt(RTC, RTC_IER_ALREN);
  // Need to do a const_cast here, because the sam API ignores const correctness.
  RTC_SetTimeAlarm(RTC, const_cast<uint8_t*>(_hour),  const_cast<uint8_t*>(_minute),  const_cast<uint8_t*>(_second));
  RTC_SetDateAlarm(RTC, const_cast<uint8_t*>(_month), const_cast<uint8_t*>(_day));
  RTC_EnableIt(RTC, RTC_IER_ALREN);
}

void RtcSam3XA::getAlarm(AlarmTime& alarmTime) {
  RTCgapclose_GetTimeAlarm(RTC, &alarmTime.hour, &alarmTime.minute, &alarmTime.second);
  RTCgapclose_GetDateAlarm(RTC, &alarmTime.month, &alarmTime.day);
}

void RtcSam3XA::setSecondCallback(void (*secondCallback)(void*),
    void *secondCallbackParam) {
  mSecondCallback = secondCallback;
  mSecondCallbackPararm = secondCallbackParam;
}
