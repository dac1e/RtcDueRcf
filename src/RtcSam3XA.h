/*
 * RtcSam3XA.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#pragma once

#ifndef RTCSAM3XA_SRC_RTCSAM3XA_H_
#define RTCSAM3XA_SRC_RTCSAM3XA_H_

#include <cstdlib>
#include <ctime>
#include <include/rtc.h>
#include "Sam3XA_RtcTime.h"

#define RTC_MEASURE_ACKUPD true

class RtcSam3XA {
public:
  /**
   * RtcSam3XA clock is the one and only clock object.
   *
   * Usage examples:
   *
   * include "TM.H
   *
   * TM time;
   * RtcSam3XA::clock.getLocalTime(time);
   *
   * const TM time { 24, 59, 11, 12, TM::make_tm_month(TM::FEBRUARY), TM::make_tm_year(2016), false };
   * RtcSam3XA::clock.setByLocalTime(time);
   */
  static RtcSam3XA clock;

  class AlarmTime {
  public:
    AlarmTime();
    void setSecond(int _second) {second = _second;}
    void setMinute(int _minute) {minute = _minute;}
    void setHour(int _hour) {hour = _hour;}
    void setDay(int _day) {second = _day;}
    void setMonth(int _month) {second = _month;}
  private:
    friend class RtcSam3XA;

    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month; // Jan = 0
  };

  /**
   * Set timezone. Refer to
   * https://man7.org/linux/man-pages/man3/tzset.3.html
   * for the explanation of the timezone string format.
   *
   * Note: When the time zone is changed, the
   * local time and the alarms will not change.
   *
   * E.g. The local time is 15:00h and there is an alarm
   * set at 16:00h. After the time zone has changed, the
   * local time within the new time zone is 15:00h and the
   * alarm time will appear at 16:00h within the new time
   * zone.
   */
  static void tzset(const char* timezone) {
    setenv("TZ", timezone, true);
  }

  enum RTC_OSCILLATOR {RC = 0, XTAL = 1};

  /**
   * Start RTC and set time zone optionally.
   */
  void begin(const char* timezone = nullptr, const RTC_OSCILLATOR source = RTC_OSCILLATOR::XTAL);

  /**
   * Set the RTC by the local time.
   *
   * The RTC does not support dates before 1st of January 2000. So the
   * tm_year that contains the elapsed years since 1900 must be greater
   * than 100.
   *
   * @return The local time as expired seconds since 1st of January 1970.
   */
  std::time_t setByLocalTime(const std::tm &time);

  /**
   * Set the RTC by a unix time stamp.
   * Prerequisite: time zone is set correctly.
   */
  void setByUnixTime(std::time_t timestamp);

  /**
   * Get the local time from the RTC.
   */
  std::time_t getLocalTime(std::tm &td) const;

  /**
   * Get a unix time stamp from the RTC.
   * Prerequisite: time zone is set correctly.
   */
  std::time_t getUnixTime() const;

  void setAlarmCallback(void (*alarmCallback)(void*), void *alarmCallbackParam = nullptr);
  void setAlarm(const AlarmTime& alarmTime);
  void clearAlarm(){setAlarm(AlarmTime());}
  void getAlarm(AlarmTime& alarmTime);

  void setSecondCallback(void (*secondCallback)(void*), void *secondCallbackParam = nullptr);


private:
  RtcSam3XA();
  void RtcSam3XA_Handler();

  // Interrupt handler
  void onSecTransitionInterrupt();

  // Global interrupt handler forwards to RtcSam3XA_Handler()
  volatile bool mSetTimeRequest;
  Sam3XA_RtcTime mSetTimeCache;
  void(*mSecondCallback)(void*);
  void* mSecondCallbackPararm;
  void(*mAlarmCallback)(void*);
  void* mAlarmCallbackPararm;

  friend void ::RTC_Handler();

#if RTC_MEASURE_ACKUPD
private:
  uint32_t mTimestampACKUPD;
public:
  uint32_t getTimestampACKUPD() const {
    return mTimestampACKUPD;
  }
#else
public:
  uint32_t getTimestampACKUPD() const {
    return 0;
  }
#endif
};

namespace TZ {
  /**
   * Some predefined timezone strings for convenience.
   */
  constexpr char* UTC = "UTC+0:00:00"; // (Coordinated Universal Time)

  // Daylight savings start in the last week of march at 2:00h and ends in the last week of October at 3:00h
  constexpr char* UK  = "UK+0:00:00UKDST-1:00:00,M3.5.0/2,M10.5.0/3";   // (United Kingdom)
  constexpr char* CET = "CET-1:00:00CETDST-2:00:00,M3.5.0/2,M10.5.0/3"; // (Central Europe)

  // Daylight savings start in the second week of march at 2:00h and ends in the first week of November at 3:00h
  constexpr char* EST= "EST+5:00:00";                                // (Eastern Standard Time)
  constexpr char* ET = "ET+5:00:00ETDST+4:00:00,M3.2.0/2,M11.1.0/3"; // (Eastern Time)
  constexpr char* CST= "CST+6:00:00";                                // (Central Standard Time)
  constexpr char* CT = "CT+6:00:00+5:00:00,M3.2.0/2,M11.1.0/3";      // (Central Time)
  constexpr char* MST= "MST+7:00:00";                                // (Mountain Standard Time)
  constexpr char* MT = "MST+7:00:00+6:00:00,M3.2.0/2,M11.1.0/3";     // (Mountain Time)
  constexpr char* PST= "PST+8:00:00";                                // (Pacific Standard Time)
  constexpr char* PT = "PST+8:00:00+7:00:00,M3.2.0/2,M11.1.0/3";     // (Pacific Time)
}

#endif /* RTCSAM3XA_SRC_RTCSAM3XA_H_ */
