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

#if RTC_MEASURE_ACKUPD
#include <Arduino.h>
#endif

RtcSam3XA RtcSam3XA::clock;

void RTC_Handler (void) {
  RtcSam3XA::clock.RtcSam3XA_Handler();
}

void RtcSam3XA::getAlarm(RtcSam3XA_Alarm &alarmTime) {
  getAlarmRaw(alarmTime);
  // Always return 24 hour mode
  alarmTime.hour += 12 * RTC_GetHourMode(RTC);
}

void RtcSam3XA::dstFixAlarm() {
  Sam3XA::RtcTime dueTimeAndDate;
  // Use the hour mode bit to store the information whether
  // the alarm is already dst adjusted. It is a save location
  // to store this info, because the information isn't lost
  // when the RTC power is backed up with a battery.
  const bool is12hrMode = RTC_GetHourMode(RTC);
  dueTimeAndDate.readFromRtc();
  if (dueTimeAndDate.isdst()) {
    // Let the RTC
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
      ::RTC_SetTimeAndDate(RTC, mSetTimeCache.hour(),
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

  Sam3XA::RtcTime dueTimeAndDate;
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
  mAlarmCallbackPararm = alarmCallbackParam;
  mAlarmCallback = alarmCallback;
}

void RtcSam3XA::setAlarm(const RtcSam3XA_Alarm& a) {

  const uint8_t* _hour = a.hour == RtcSam3XA_Alarm::INVALID_VALUE ? nullptr : &a.hour;
  const uint8_t* _minute = a.minute == RtcSam3XA_Alarm::INVALID_VALUE ? nullptr : &a.minute;
  const uint8_t* _second = a.second == RtcSam3XA_Alarm::INVALID_VALUE ? nullptr : &a.second;
  const uint8_t* _day = a.day == RtcSam3XA_Alarm::INVALID_VALUE ? nullptr : &a.day;
  const uint8_t* _month = a.month == RtcSam3XA_Alarm::INVALID_VALUE ? nullptr : &a.month;

  RTC_DisableIt(RTC, RTC_IER_ALREN);
  // Need to do a const_cast here, because the sam API ignores const correctness.
  RTC_SetTimeAlarm(RTC, const_cast<uint8_t*>(_hour),  const_cast<uint8_t*>(_minute),  const_cast<uint8_t*>(_second));
  RTC_SetDateAlarm(RTC, const_cast<uint8_t*>(_month), const_cast<uint8_t*>(_day));
  RTC_EnableIt(RTC, RTC_IER_ALREN);
}

void RtcSam3XA::getAlarmRaw(RtcSam3XA_Alarm& alarmTime) {
  RTC_GetTimeAlarm(RTC, &alarmTime.hour, &alarmTime.minute, &alarmTime.second);
  RTC_GetDateAlarm(RTC, &alarmTime.month, &alarmTime.day);
}

void RtcSam3XA::setSecondCallback(void (*secondCallback)(void*),
    void *secondCallbackParam) {
  mSecondCallback = secondCallback;
  mSecondCallbackPararm = secondCallbackParam;
}


/******************
 * RtcSam3XA_Alarm
 *****************/

RtcSam3XA_Alarm::RtcSam3XA_Alarm() :
    second(INVALID_VALUE), minute(INVALID_VALUE), hour(INVALID_VALUE), day(INVALID_VALUE), month(INVALID_VALUE) {
}

RtcSam3XA_Alarm::RtcSam3XA_Alarm(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon)
  : second(tm_sec < 60 ? tm_sec : INVALID_VALUE), minute(tm_min < 60 ? tm_min : INVALID_VALUE)
  , hour(tm_hour < 24 ? tm_hour : INVALID_VALUE), day(tm_mday < 32 ? tm_mday : INVALID_VALUE)
  , month(tm_mon < 12 ? tm_mon+1 : INVALID_VALUE) {
}

static inline bool subtractTimeFraction(int& q, uint8_t& v, unsigned d) {
  if(q > 0) {
    if (v != RtcSam3XA_Alarm::INVALID_VALUE) {
      int r = q % d;
      q = q / d;
      if(r > v) {
        r -= d;
        ++q;
      }
      v -= r;
      return true;
    }
  }
  return false;
}

void RtcSam3XA_Alarm::subtract(int _seconds, bool bIsLeapYear) {
  // Check allowed boundaries.
  assert(_seconds < 24 * 60 * 60 * 28);
  assert(_seconds > 0);
  int q = _seconds;

  if(not subtractTimeFraction(q, second, 60) ) {return;}
  if(not subtractTimeFraction(q, minute, 60) ) {return;}
  if(not subtractTimeFraction(q, hour, 24) )   {return;}
  if (day != INVALID_VALUE) {
    uint8_t _day = day - 1;
    int d = 31;
    if(month != INVALID_VALUE) {
      const uint8_t _month = month - 1;
      const int previousMonth = (_month - 1 + 12) % 12 + 1;
      d = Sam3XA::RtcTime::monthLength(previousMonth, bIsLeapYear);
    }
    subtractTimeFraction(q, _day, d);
    day = _day + 1;
    if (month != INVALID_VALUE) {
      month = (month - 1 - q + 12) % 12 + 1;
    }
  }
}

static inline bool addTimeFraction(int& q, uint8_t& v, unsigned d) {
  if(q > 0) {
    if (v != RtcSam3XA_Alarm::INVALID_VALUE) {
      const int r = (v + q) % d;
      q = q / d;
      if(r < v) {
        ++q;
      }
      v = r;
      return true;
    }
  }
  return false;
}

void RtcSam3XA_Alarm::add(int _seconds /* 0.. (24 * 60 * 60 * 28) */, bool bIsLeapYear) {
  assert(_seconds < 24 * 60 * 60 * 28);
  assert(_seconds > 0);
  int q = _seconds;

  if(not addTimeFraction(q, second, 60) ) {return;}
  if(not addTimeFraction(q, minute, 60) ) {return;}
  if(not addTimeFraction(q, hour, 24) )   {return;}
  if (day != INVALID_VALUE) {
    uint8_t _day = day - 1;
    int d = 31;
    if(month != INVALID_VALUE) {
      const uint8_t tm_mon = month - 1;
      d = Sam3XA::RtcTime::monthLength(month, bIsLeapYear);
    }
    addTimeFraction(q, _day, d);
    day = _day + 1;
    if (month != INVALID_VALUE) {
      month = (month - 1 + q + 12) % 12 + 1;
    }
  }
}

static inline void fillAlarmFraction(uint8_t& v) {
  if(v == RtcSam3XA_Alarm::INVALID_VALUE) { v = 0; }
}

RtcSam3XA_Alarm RtcSam3XA_Alarm::gaps2zero() const {
  RtcSam3XA_Alarm result = *this;

  // Fill all less significant values below the highest
  // significant valid value with 0, if set to
  // INVALID_VALUE.
  if(month != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(result.day);
    fillAlarmFraction(result.hour);
    fillAlarmFraction(result.minute);
    fillAlarmFraction(result.second);
  } else if(day != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(result.hour);
    fillAlarmFraction(result.minute);
    fillAlarmFraction(result.second);
  } else if(day != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(result.minute);
    fillAlarmFraction(result.second);
  } else if(day != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    fillAlarmFraction(result.second);
  }
  return result;
}

static inline void emptyAlarmFraction(uint8_t& v) {
  if(v == 0) { v = RtcSam3XA_Alarm::INVALID_VALUE; }
}

RtcSam3XA_Alarm RtcSam3XA_Alarm::zero2gaps() const {
  RtcSam3XA_Alarm result = *this;

  // Fill all less significant values below the highest
  // significant valid value with INVALID_VALUE if
  // set to 0.
  if(month != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    emptyAlarmFraction(result.day);
    emptyAlarmFraction(result.hour);
    emptyAlarmFraction(result.minute);
    emptyAlarmFraction(result.second);
  } else if(day != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    emptyAlarmFraction(result.hour);
    emptyAlarmFraction(result.minute);
    emptyAlarmFraction(result.second);
  } else if(day != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    emptyAlarmFraction(result.minute);
    emptyAlarmFraction(result.second);
  } else if(day != RtcSam3XA_Alarm::INVALID_VALUE) {
    // Fill all lower significant values with 0 if not set
    emptyAlarmFraction(result.second);
  }
  return result;
}

