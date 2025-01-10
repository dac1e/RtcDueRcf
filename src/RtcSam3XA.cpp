/*
 * RtcSam3XA.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "RtcSam3XA.h"
#include "core-sam-GapClose.h"
#include <assert.h>

namespace {

int isLeapYear(int tm_year) {
  const int year = tm_year + 1900;
  const int result = ( !(year % 4) && ( (year % 100) || !(year % 400) ) );;
  return result;
}

/**
 * The return value includes this year if it is a leap year.
 */
int leapYearsSince1970 (int tm_year) {
  const int year = tm_year + 1900;
  const int yearsDiv4count   = (year-1968 /* first year that divides by   4 without rest. */) /   4;
  const int yearsDiv100count = (year-1900 /* first year that divides by 100 without rest. */) / 100;
  const int yearsDiv400count = (year-1600 /* first year that divides by 400 without rest. */) / 400;
  return yearsDiv4count - yearsDiv100count + yearsDiv400count;
}

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
 * Convert UTC time to time stamp.
 * Prerequisite: tm_yday and tm_isdst are set correctly.
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

} // anonymous namespace

void Sam3XA_RtcTime::readFromRtc() {
  ::RTCgapclose_GetTimeAndDate(RTC, &mHour, &mMinute, &mSecond, &mYear, &mMonth,
      &mDay, &mWeekDay);
}

RtcSam3XA::AlarmTime::AlarmTime() :
    second(UINT8_MAX), minute(UINT8_MAX), hour(UINT8_MAX), day(UINT8_MAX), month(
        UINT8_MAX) {
}

RtcSam3XA::tm::tm() : tm(0, 0, 0, 1, 1, make_tm_year(1970), false) {
}


RtcSam3XA::tm::tm(int _sec, int _min, int _hour, int _mday, int _mon, int _year, bool _isdst) {
  tm_mday = _mday; tm_mon = _mon; tm_year = _year;
  tm_hour = _hour; tm_min = _min; tm_sec = _sec;
  tm_isdst = _isdst;
  tm_wday = -1; // unknown
  tm_yday = -1; // unknown
}

RtcSam3XA RtcSam3XA::RtClock;

void RTC_Handler (void) {
  RtcSam3XA::RtClock.RtcSam3XA_Handler();
}

std::time_t RtcSam3XA::tm::set(std::tm& time, const Sam3XA_RtcTime& sam2xatime) {
  time.tm_hour = sam2xatime.tmHour(); time.tm_min = sam2xatime.tmMinute(); time.tm_sec = sam2xatime.tmSecond();
  time.tm_year = sam2xatime.tmYear(); time.tm_mon = sam2xatime.tmMonth(); time.tm_mday = sam2xatime.tmDay();
  time.tm_wday = sam2xatime.tmWeek(); time.tm_yday = 0; time.tm_isdst = 0 /* We get standard time from Rtc. */;

  // mktime will fix the tm_yday as well as tm_ist and the
  // hour, if time is within daylight savings period.
  return mktime(&time);
}

RtcSam3XA::RtcSam3XA()
  : mSetTimeRequest(false)
  , mSecondCallback(nullptr)
  , mSecondCallbackPararm(nullptr)
  , mAlarmCallback(nullptr)
  , mAlarmCallbackPararm(nullptr)
{
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
    if(mSecondCallback) {
      (*mSecondCallback)(mSecondCallbackPararm);
    }
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

void RtcSam3XA::begin(RTC_OSCILLATOR source, const char* timezone) {
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
  RTC_EnableIt(RTC, RTC_IER_ACKEN);
  //    RTC_EnableIt(RTC, RTC_IER_SECEN | RTC_IER_ALREN);
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
    return tm::set(time, mSetTimeCache);
  }

  Sam3XA_RtcTime dueTimeAndDate;
  dueTimeAndDate.readFromRtc();
  return tm::set(time, dueTimeAndDate);
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

void RtcSam3XA::setAlarm(const AlarmTime& alarmTime) {

  const uint8_t* _hour = alarmTime.hour == UINT8_MAX ? nullptr : &alarmTime.hour;
  const uint8_t* _minute = alarmTime.minute == UINT8_MAX ? nullptr : &alarmTime.minute;
  const uint8_t* _second = alarmTime.second == UINT8_MAX ? nullptr : &alarmTime.second;
  const uint8_t* _day = alarmTime.day == UINT8_MAX ? nullptr : &alarmTime.day;
  const uint8_t* _month = alarmTime.month == UINT8_MAX ? nullptr : &alarmTime.month;

  RTC_DisableIt(RTC, RTC_IER_ALREN);
  // Need to do a const_cast here, because the sam API ignores const correctness.
  RTC_SetTimeAlarm(RTC, const_cast<uint8_t*>(_hour),  const_cast<uint8_t*>(_minute),  const_cast<uint8_t*>(_second));
  RTC_SetDateAlarm(RTC, const_cast<uint8_t*>(_month), const_cast<uint8_t*>(_day));
  RTC_EnableIt(RTC, RTC_IER_ALREN);
}

void RtcSam3XA::setSecondCallback(void (*secondCallback)(void*),
    void *secondCallbackParam) {
  mSecondCallback = secondCallback;
  mSecondCallbackPararm = secondCallbackParam;
}
