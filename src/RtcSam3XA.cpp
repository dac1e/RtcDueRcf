/*
 * RtcSam3XA.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include <assert.h>
#include "TM.h"
#include "RtcSam3XA.h"

#include "internal/core-sam-GapClose.h"
#include "internal/RtcTime.h"

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

void RtcSam3XA::getAlarm(RtcSam3XA_Alarm &alarm) {
  alarm.readFromRtc();
}

void RtcSam3XA::dstFixAlarm() {
  Sam3XA::RtcTime dueTimeAndDate;
  // Use the hour mode bit to store the information whether the alarm is
  // already dst adjusted. It is a save location to store this info,
  // because the information isn't lost when the RTC power is backed up
  // with a battery.

  // Cannot use RTC_GetHourMode() from rtc.h because it has a bug.
  // const bool is12hrMode = RTC_GetHourMode(RTC);
  const bool is12hrsMode = RTC_GetHourMode_();
  dueTimeAndDate.readFromRtc();
  if (dueTimeAndDate.isdst()) {
    if(not is12hrsMode) {
      // Change from standard to daylight savings time -> fix the alarms
      const int timeDiff = dstToMormalTimeDiff();
      // add time diff to alarm time
      RtcSam3XA_Alarm alarm;
      getAlarm(alarm);
      alarm = alarm.gaps2zero();
    }
  } else {
    if(is12hrsMode) {
      // Change from daylight savings to standard time -> fix the alarms
      const int timeDiff = dstToMormalTimeDiff();
      // add time diff to alarm time
      RtcSam3XA_Alarm alarm;
      getAlarm(alarm);

    }
  }
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
#if RTC_DEBUG_ACKUPD
      Serial.println("I");
#endif
      RTC_SetTimeAndDate(RTC, mSetTimeCache.hour(), mSetTimeCache.minute(),
        mSetTimeCache.second(), mSetTimeCache.year(), mSetTimeCache.month(),
        mSetTimeCache.day(), mSetTimeCache.day_of_week());
      // In order to detect whether RTC carries daylight savings time or
      // standard time, 12-hrs mode of RTC is applied, when RTC carries
      // daylight savings time.
      RTC_SetHourMode(RTC, mSetTimeCache.rtc12hrsMode());
      mSetTimeRequest = false;
    }
    RTC_ClearSCCR(RTC, RTC_SCCR_ACKCLR);
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

#if 0
  // Calculate local standard time (I.e. local time
  // without daylight savings shift).
  std::time_t localStandardTime = TM::mkgmtime(buffer);
  gmtime_r(&localStandardTime, &buffer);
#endif

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

  Sam3XA::RtcTime dueTimeAndDate;

#if RTC_DEBUG_HOUR_MODE
  dueTimeAndDate.readFromRtc(Serial);
#else
  dueTimeAndDate.readFromRtc();
#endif

  return dueTimeAndDate.get(time);
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

void RtcSam3XA::setSecondCallback(void (*secondCallback)(void*),
    void *secondCallbackParam) {
  mSecondCallback = secondCallback;
  mSecondCallbackPararm = secondCallbackParam;
}

