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

#if RTC_MEASURE_ACKUPD
#include <Arduino.h>
#endif


RtcSam3XA RtcSam3XA::clock;

void RTC_Handler (void) {
  RtcSam3XA::clock.RtcSam3XA_Handler();
}

RtcSam3XA::AlarmTime::AlarmTime() :
    second(UINT8_MAX), minute(UINT8_MAX), hour(UINT8_MAX), day(UINT8_MAX), month(UINT8_MAX) {
}

RtcSam3XA::RtcSam3XA()
  : mSetTimeRequest(false)
  , mSecondCallback(nullptr)
  , mSecondCallbackPararm(nullptr)
  , mAlarmCallback(nullptr)
  , mAlarmCallbackPararm(nullptr)
#if RTC_MEASURE_ACKUPD
  , mTimestampACKUPD(0)
#endif
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
#if RTC_MEASURE_ACKUPD
    mTimestampACKUPD = millis();
#endif
    if (mSetTimeRequest) {
      ::RTCgapclose_SetTimeAndDate(RTC, mSetTimeCache.hour(),
          mSetTimeCache.minute(), mSetTimeCache.second(),
          mSetTimeCache.year(), mSetTimeCache.month(),
          mSetTimeCache.day(), mSetTimeCache.day_of_week());
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

std::time_t RtcSam3XA::setByLocalTime(const std::tm &time) {
  assert(time.tm_year >= TM::make_tm_year(2000));

  RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  RTC_DisableIt(RTC, RTC_IER_ACKEN);

  // Call mktime in order to fix tm_yday, tm_isdst
  // and the hour, depending on whether the time is
  // within daylight saving period or not.
  std::tm buffer = time;
  const time_t localTime = mktime(&buffer);

  // Calculate local standard time (I.e. local time
  // without daylight savings shift.)
  std::time_t localStandardTime = TM::mkgmtime(buffer);
  gmtime_r(&localStandardTime, &buffer);

  // Fill cache with standard time.
  mSetTimeCache.set(buffer);
  mSetTimeRequest = true;

  RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  RTC_EnableIt(RTC, RTC_IER_ACKEN);
  return localTime;
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
