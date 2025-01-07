/*
 * RtcSam3XA.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "RtcSam3XA.h"
#include "core-sam-GapClose.h"

RtcSam3XA RtcSam3XA::rtClock;

void RTC_Handler (void) {
  RtcSam3XA::rtClock.RtcSam3XA_Handler();
}

RtcSam3XA::tm::tm() : tm(1, 0, 2000, 0, 0 , 0, -1) {
}

RtcSam3XA::tm::tm(int _day, int _m, int _year, int _hours, int _minutes, int _seconds, bool _sl) {
  tm_mday = _day; tm_mon = _m; tm_year = _year;
  tm_hour = _hours; tm_min = _minutes; tm_sec = _seconds;
  tm_wday = tmDayofWeek(_year, tm_mon, _day); tm_isdst = _sl;
}

int RtcSam3XA::tm::tmDayofWeek (uint16_t _year, int _month, int _day)
{
  ++_month;
  if (_month == 1 || _month == 2) {
    _month += 12;
    --_year;
  }

  /**
   * Zeller's congruence for the Gregorian calendar.
   * With 0=Monday, ... 5=Saturday, 6=Sunday
   * http://www.mikrocontroller.net/topic/144905
   * 2 * _month + 3 * (_month + 1) / 5 != (13 * _month + 3) / 5
   * With 0=Monday, ... 5=Saturday, 6=Sunday
   */
  /*
  _week = (_day + 2 * _month + 3 * (_month + 1) / 5 + _year + _year / 4 - _year / 100 + _year / 400) % 7;
  */

  // http://esmz-designz.com/index.php?site=blog&entry=68
  // With 0=Sunday, 1=Monday... , 6=Saturday
  return (_day + 2 * _month + 3 * (_month + 1) / 5 + _year + _year / 4 - _year / 100 + _year / 400 + 1) % 7;
}

void RtcSam3XA::tm::set(Sam3XA_RtcTime& td) {
  tm_hour = td.tmHour(); tm_min = td.tmMinute(); tm_sec = td.tmSecond();
  tm_year = td.tmYear(); tm_mon = td.tmMonth(); tm_mday = td.tmDay();
  tm_wday = td.tmWeek(); tm_yday = 0; tm_isdst = -1;
}

void Sam3XA_RtcTime::readFromRtc() {
  ::RTCgapclose_GetTimeAndDate(RTC, &mHour, &mMinute, &mSecond, &mYear, &mMonth,
      &mDay, &mWeek);
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

void RtcSam3XA::begin(RTC_OSCILLATOR source) {
  if (source == XTAL) {
    pmc_switch_sclk_to_32kxtal(0);
    while (!pmc_osc_is_ready_32kxtal())
      ;

  }
  NVIC_DisableIRQ(RTC_IRQn);
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, 0);
  RTC_EnableIt(RTC, RTC_IER_ACKEN);
  //    RTC_EnableIt(RTC, RTC_IER_SECEN | RTC_IER_ALREN);
  NVIC_EnableIRQ(RTC_IRQn);
}

void RtcSam3XA::set_time(const std::tm &t) {
  RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  RTC_DisableIt(RTC, RTC_IER_ACKEN);
  mSetTimeCache.set(t);
  mSetTimeRequest = true;
  RTC_EnableIt(RTC, RTC_IER_ACKEN);
}

void RtcSam3XA::get_time(tm &td) {
  if (mSetTimeRequest) {
    td.set(mSetTimeCache);
  } else {
    Sam3XA_RtcTime dueTimeAndDate;
    dueTimeAndDate.readFromRtc();
    td.set(dueTimeAndDate);
  }
}

void RtcSam3XA::setAlarmCallback(void (*alarmCallback)(void*),
    void *alarmCallbackParam) {
  mAlarmCallback = alarmCallback;
  mAlarmCallbackPararm = alarmCallbackParam;
}

void RtcSam3XA::setAlarm(const AlarmTime& alarmTime) {
  RTC_DisableIt(RTC, RTC_IER_ALREN);
  const uint8_t* _hour = alarmTime.hour == UINT8_MAX ? nullptr : &alarmTime.hour;
  const uint8_t* _minute = alarmTime.minute == UINT8_MAX ? nullptr : &alarmTime.minute;
  const uint8_t* _second = alarmTime.second == UINT8_MAX ? nullptr : &alarmTime.second;
  const uint8_t* _day = alarmTime.day == UINT8_MAX ? nullptr : &alarmTime.day;
  const uint8_t* _month = alarmTime.month == UINT8_MAX ? nullptr : &alarmTime.month;
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
