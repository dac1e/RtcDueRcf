/*
 * RtcSam3XA.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include <assert.h>

#include "TM.h"
#include "internal/RtcTime.h"
#include "internal/core-sam-GapClose.h"
#include "RtcSam3XA.h"

#if RTC_MEASURE_ACKUPD || RTC_DEBUG_HOUR_MODE
#include <Arduino.h>
#endif

namespace {

// Substitute function, because the original api function  RTC_GetHourMode()
// from rtc.h has a bug.
uint32_t RTC_GetHourMode_()
{
    return RTC->RTC_MR & 0x00000001;
}

int dstToMormalTimeDiff() {
  // Change from standard to daylight savings time -> fix the alarms
  const __tzinfo_type* const tz = __gettzinfo();
  const __tzrule_struct* const tzrule_DstBegin = &tz->__tzrule[!tz->__tznorth];
  const __tzrule_struct* const tzrule_DstEnd = &tz->__tzrule[tz->__tznorth];
  // Note: Unit of the offsets is seconds. The offsets become negative in
  // west direction. result will become positive.
  return tzrule_DstBegin->offset - tzrule_DstEnd->offset;
}

} // anonymous namespace


RtcSam3XA RtcSam3XA::clock;

void RTC_Handler (void) {
  RtcSam3XA::clock.RtcSam3XA_Handler();
}

RtcSam3XA::RtcSam3XA()
  : mSetTimeRequest(SET_TIME_REQUEST::NO_REQUEST)
  , mSecondCallback(nullptr)
  , mSecondCallbackPararm(nullptr)
  , mAlarmCallback(nullptr)
  , mAlarmCallbackPararm(nullptr)
#if RTC_MEASURE_ACKUPD
  , mTimestampACKUPD(0)
#endif
{
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

void RtcSam3XA::RtcSam3XA_DstChecker() {
  if(mSetTimeRequest != SET_TIME_REQUEST::DST_RTC_REQUEST) {
    TM tm; Sam3XA::RtcTime dueTimeAndDate;
    if(dueTimeAndDate.dstRtcRequest(tm)) {
      // Fill cache with time.
      mSetTimeCache.set(tm);
      if(not mSetTimeRequest) {
        mSetTimeRequest = SET_TIME_REQUEST::DST_RTC_REQUEST;
        RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
      }
    }
  }
}

void RtcSam3XA::RtcSam3XA_AckUpdHandler() {
  if (mSetTimeRequest) {
    mSetTimeCache.writeToRtc();
    mSetTimeRequest = SET_TIME_REQUEST::NO_REQUEST;
  }
}

void RtcSam3XA::RtcSam3XA_Handler() {
  const uint32_t status = RTC->RTC_SR;
  /* Second increment interrupt */
  if ((status & RTC_SR_SEC) == RTC_SR_SEC) {
    RtcSam3XA_DstChecker();
    if (mSecondCallback) {
      (*mSecondCallback)(mSecondCallbackPararm);
    }
    RTC_ClearSCCR(RTC, RTC_SCCR_SECCLR);
  }

  /* Acknowledge for Update interrupt */
  if ((status & RTC_SR_ACKUPD) == RTC_SR_ACKUPD) {
#if RTC_MEASURE_ACKUPD
    mTimestampACKUPD = millis();
#endif
    RtcSam3XA_AckUpdHandler();
    RTC_ClearSCCR(RTC, RTC_SCCR_ACKCLR);
  }

  /* Time or date alarm */
  if ((status & RTC_SR_ALARM) == RTC_SR_ALARM) {
    if(mAlarmCallback) {
      (*mAlarmCallback)(mAlarmCallbackPararm);
    }
    RTC_ClearSCCR(RTC, RTC_SCCR_ALRCLR);
  }
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

  // Fill cache with time.
  mSetTimeCache.set(buffer);
  mSetTimeRequest = SET_TIME_REQUEST::REQUEST;

  RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  RTC_EnableIt(RTC, RTC_IER_ACKEN);
  return localTime;
}

std::time_t RtcSam3XA::getLocalTime(std::tm &time) const {
  if (mSetTimeRequest) {
    return mSetTimeCache.get(time);
  }

  Sam3XA::RtcTime dueTimeAndDate;
  dueTimeAndDate.readFromRtc();
  return dueTimeAndDate.get(time);
}

void RtcSam3XA::setByUnixTime(std::time_t timestamp) {
  tm time;
  localtime_r(&timestamp, &time);
  setByLocalTime(time);
}

time_t RtcSam3XA::getUnixTime() const {
  tm time;
  getLocalTime(time);
  return mktime(&time);
}

void RtcSam3XA::setAlarmCallback(void (*alarmCallback)(void*),
    void *alarmCallbackParam) {
  RTC_DisableIt(RTC, RTC_IER_ALREN);
  mAlarmCallback = alarmCallback;
  mAlarmCallbackPararm = alarmCallbackParam;
  RTC_EnableIt(RTC, RTC_IER_ALREN);

}

void RtcSam3XA::setAlarm(const RtcSam3XA_Alarm& alarm) {
  RTC_SetTimeAndDateAlarm(RTC, alarm.hour, alarm.minute, alarm.second, alarm.month, alarm.day);
}

void RtcSam3XA::getAlarm(RtcSam3XA_Alarm &alarm) {
  RTC_GetTimeAlarm(RTC, &alarm.hour, &alarm.minute, &alarm.second);
  RTC_GetDateAlarm(RTC, &alarm.month, &alarm.day);

}

void RtcSam3XA::setSecondCallback(void (*secondCallback)(void*),
    void *secondCallbackParam) {
  mSecondCallback = secondCallback;
  mSecondCallbackPararm = secondCallbackParam;
}

