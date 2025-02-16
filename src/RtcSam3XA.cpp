/*
  RtcSam3XA - Arduino libary for RtcSam3XA - builtin RTC Copyright (c)
  2024 Wolfgang Schmieder.  All right reserved.

  Contributors:
  - Wolfgang Schmieder

  Project home: https://github.com/dac1e/RcSwitchReceiver/

  This library is free software; you can redistribute it and/or modify it
  the terms of the GNU Lesser General Public License as under published
  by the Free Software Foundation; either version 3.0 of the License,
  or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <assert.h>

#include "TM.h"
#include "internal/RtcTime.h"
#include "internal/core-sam-GapClose.h"
#include "RtcSam3XA.h"

#ifndef MEASURE_DST_RTC_REQUEST
#define MEASURE_DST_RTC_REQUEST false
#endif

#ifndef DEBUG_DST_REQUEST
#define DEBUG_DST_REQUEST false
#endif

#if RTC_MEASURE_ACKUPD || MEASURE_DST_RTC_REQUEST || DEBUG_DST_REQUEST
#include <Arduino.h>
#endif

namespace {

/**
 * Substitute for the original api function  RTC_GetHourMode()
 * from rtc.h which has a bug.
 */
uint32_t RTC_GetHourMode_()
{
    return RTC->RTC_MR & 0x00000001;
}

/**
 * Calculate the seconds that will be switched forward, when daylight
 * savings starts.
 * This is identical to the seconds that will be switched backward,
 * when daylight saving ends.
 */
int dstToStandardTimeDiff() {
  const __tzinfo_type* const tz = __gettzinfo();
  const __tzrule_struct* const tzrule_DstBegin = &tz->__tzrule[!tz->__tznorth];
  const __tzrule_struct* const tzrule_DstEnd = &tz->__tzrule[tz->__tznorth];
  // Note: Unit of the offsets is seconds. The offsets become negative in
  // west direction. Result will become positive.
  return tzrule_DstBegin->offset - tzrule_DstEnd->offset;
}

} // anonymous namespace

RtcSam3XA RtcSam3XA::clock;

/**
 * Global interrupt handler forwards to RtcSam3XA_Handler()
 */
void RTC_Handler (void) {
  RtcSam3XA::clock.RtcSam3XA_Handler();
}

/**
 * Default constructor. Constructor is private, because there must
 * be only one object RtcSam3XA::clock.
 */
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

void RtcSam3XA::begin(const char* timezone, const uint8_t irqPrio, const RTC_OSCILLATOR source) {
  RTC_DisableIt(RTC, RTC_IDR_ACKDIS | RTC_IDR_ALRDIS | RTC_IDR_SECDIS
      | RTC_IDR_TIMDIS | RTC_IDR_CALDIS);

  if(timezone != nullptr) {
    tzset(timezone);
  }

  if (source == XTAL) {
    pmc_switch_sclk_to_32kxtal(0);
    while (!pmc_osc_is_ready_32kxtal());
  }

  NVIC_DisableIRQ(RTC_IRQn);
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, irqPrio);
  RTC_EnableIt(RTC, RTC_IER_SECEN | RTC_IER_ACKEN);
  NVIC_EnableIRQ(RTC_IRQn);
}

/**
 * Place the time in the mSetTimeCache, set the mSetTimeRequest
 * to true. Set a RTC time and calendar update request.
 * This update request will fire an interrupt, once the RTC
 * is ready to accept a new time and date. The RTC_Handler
 * will then pickup the time and date from the mSetTimeCache
 * and write it to the RTC.
 */
bool RtcSam3XA::setTime(const std::tm &localTime) {
  if(localTime.tm_year >= TM::make_tm_year(2000)) {
    RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
    RTC_DisableIt(RTC, RTC_IER_ACKEN);

    /**
     * Call mktime in order to fix tm_yday, tm_isdst and the hour,
     * depending on whether the time is within daylight saving
     * period or not.
     */
     std::tm buffer = localTime;
     const time_t localTime = mktime(&buffer);

    // Fill cache with time.
    mSetTimeCache.set(buffer);
#if DEBUG_DST_REQUEST
      Serial.print("setTime");
#endif

    if(not mSetTimeRequest) {
      mSetTimeRequest = SET_TIME_REQUEST::REQUEST;
#if DEBUG_DST_REQUEST
      Serial.println(", REQUEST");
#endif
      RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
    } else {
#if DEBUG_DST_REQUEST
      Serial.println();
#endif
    }

    RTC_EnableIt(RTC, RTC_IER_ACKEN);
    return true;
  }
  return false;
}

/**
 * Check daylight savings transition, and update the RTC accordingly.
 * Adjusting the RTC to local daylight saving time ensures, that
 * the RTC alarms happen at the expected time.
 *
 * When compiled with option -Os, the function takes 10us to execute,
 * in case RTC update is not required. It takes 80us when transition
 * is required (2 times in a year). These values are related to
 * compile option -Os.
 */
void RtcSam3XA::RtcSam3XA_DstChecker() {
  if(mSetTimeRequest != SET_TIME_REQUEST::DST_RTC_REQUEST) {
#if MEASURE_DST_RTC_REQUEST
    const uint32_t start = micros();
#endif
    Sam3XA::RtcTime dueTimeAndDate;
    const bool request = dueTimeAndDate.isDstRtcRequest();
    if(request) {
      // Fill cache with time.
      mSetTimeCache = dueTimeAndDate;
#if DEBUG_DST_REQUEST
      Serial.print("RtcSam3XA_DstChecker");
#endif
      if(not mSetTimeRequest) {
        mSetTimeRequest = SET_TIME_REQUEST::DST_RTC_REQUEST;
#if DEBUG_DST_REQUEST
        Serial.println(", DST_RTC_REQUEST");
#endif
        RTC->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
      } else {
        mSetTimeRequest = SET_TIME_REQUEST::DST_RTC_REQUEST;
#if DEBUG_DST_REQUEST
        Serial.println(", DST_RTC_REQUEST");
#endif
      }
    }
#if MEASURE_DST_RTC_REQUEST
    const uint32_t diff = micros()-start;
    if(request) {
      Serial.print("DST_RTC_REQUEST (TRUE)  duration: ");
    } else {
      Serial.print("DST_RTC_REQUEST (FALSE) duration: ");
    }
    Serial.println(diff);
#endif
  }
}

/**
 * Pick the mSetTimeCache and write it to the RTC.
 */
void RtcSam3XA::RtcSam3XA_AckUpdHandler() {
  if (mSetTimeRequest != SET_TIME_REQUEST::NO_REQUEST) {
    mSetTimeCache.writeToRtc();
    mSetTimeRequest = SET_TIME_REQUEST::NO_REQUEST;
#if DEBUG_DST_REQUEST
    Serial.println("NO_REQUEST");
#endif
  }
}

/**
 * RtcSam3XA interrupt handler
 */
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

  /* RTC alarm */
  if ((status & RTC_SR_ALARM) == RTC_SR_ALARM) {
    if(mAlarmCallback) {
      (*mAlarmCallback)(mAlarmCallbackPararm);
    }
    RTC_ClearSCCR(RTC, RTC_SCCR_ALRCLR);
  }
}

bool RtcSam3XA::setTime(std::time_t utcTimestamp) {
  tm time;
  localtime_r(&utcTimestamp, &time);
  return setTime(time);
}

std::time_t RtcSam3XA::getLocalTimeAndUTC(std::tm &time) const {
  if (mSetTimeRequest) {
    return mSetTimeCache.get(time);
  }

  Sam3XA::RtcTime dueTimeAndDate;
  dueTimeAndDate.readFromRtc();
  return dueTimeAndDate.get(time);
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

