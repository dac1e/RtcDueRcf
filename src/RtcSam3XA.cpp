/*
 * RtcSam3XA.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "RtcSam3XA.h"
#include "core-sam-GapClose.h"

RtcSam3XA RtcSam3XA::rtClock;

// Time structure to read from and write to the Sam3X RTC.
class Sam3XA_RtcTime {
  uint8_t mHour;
  uint8_t mMinute;
  uint8_t mSecond;

  uint16_t mYear;
  uint8_t mMonth;
  uint8_t mDay;
  uint8_t mWeek;

public:
  inline uint8_t hour() const {return mHour;}
  inline uint8_t minute() const {return mMinute;}
  inline uint8_t second() const {return mSecond;}
  inline uint16_t year() const {return mYear;}
  inline uint8_t month() const {return mMonth;}
  inline uint8_t day() const {return mDay;}
  inline uint8_t week() const {return mWeek;}

  inline int tmHour()const {return mHour;}
  inline int tmMinute()const {return mMinute;}
  inline int tmSecond()const {return mSecond;}
  inline int tmYear()const {return mYear;}
  inline int tmMonth()const {return mMonth - 1;}
  inline int tmDay()const {return mDay;}
  inline int tmWeek()const {return mWeek - 1;}

  static uint8_t rtcMonth(const std::tm& t) {return t.tm_mon + 1;}
  static uint8_t rtcDayOfWeek(const std::tm& t) {return t.tm_wday + 1;}

  void set(const std::tm &t) {
    mHour = t.tm_hour; mMinute = t.tm_min; mSecond = t.tm_sec;
    mYear = t.tm_year; mMonth = rtcMonth(t); mDay = t.tm_mday;
    mWeek = rtcDayOfWeek(t);
  }

  void readFromRtc();
};

void RTC_Handler (void) {
  RtcSam3XA::rtClock.RtcSam3XA_Handler();
}

RtcSam3XA::tm::tm() : tm(1, 0, 2000, 0, 0 , 0, false) {
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
  tm_wday = td.tmWeek(); tm_yday = 0; tm_isdst = 0;
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
  uint8_t* _hour = alarmTime.hour < 0 ? nullptr : &alarmTime.hour;
  uint8_t* _minute = alarmTime.minute < 0 ? nullptr : alarmTime.minute;
  uint8_t* _second = alarmTime.second < 0 ? nullptr : alarmTime.second;
  uint8_t* _day = alarmTime.day < 0 ? nullptr : alarmTime.day;
  uint8_t* _month = alarmTime.month < 0 ? nullptr : alarmTime.month;
  RTC_SetTimeAlarm(RTC, _hour, _minute, _second);
  RTC_SetDateAlarm(RTC, _month, _day);
  RTC_EnableIt(RTC, RTC_IER_ALREN);
}

void RtcSam3XA::setSecondCallback(void (*secondCallback)(void*),
    void *secondCallbackParam) {
  mSecondCallback = secondCallback;
  mSecondCallbackPararm = secondCallbackParam;
}
