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

#include "internal/RtcTime.h"
#include "RtcSam3XA_Alarm.h"

#define RTC_MEASURE_ACKUPD false

/**
 * RtcSam3XA offers functions to operate the built in Real Time Clock
 * (RTC) and it' alarms features.
 * The RTC is represented as a single object named RtcSam3XA::clock.
 *
 * The standard structure std::tm and the standard type std::time_t
 * are used to operate the RTC.
 * There is the class TM derived from std::tm that enhances structure
 * std::tm with some convenience functions for std::tm.
 *
 * The class RtcSam3XA_alarm is used to operate the alarm features
 * of the RTC.
 */
class RtcSam3XA {
public:
  /**
   * The static object RtcSam3XA::clock is the one and only clock object.
   *
   * Usage examples:
   *
   * include "TM.H
   *
   * // Read the local time and date from RTC.
   * TM time;
   * RtcSam3XA::clock.getLocalTime(time);
   *
   * // Write the local time and date to the RTC.
   * const TM time {24, 59, 11, 12, TM::make_tm_month(2), TM::make_tm_year(2016), false};
   * RtcSam3XA::clock.setByLocalTime(time);
   */
  static RtcSam3XA clock;

  /**
   * Set time zone. Setting the time zone is required for the correct
   * collaboration of the standard C++ entities std::tm, std::time_t,
   * std::mktime(), std::gmtime(), std::localtime() along with the
   * this RtcSam3XA class. It is also needed for determining the day light
   * savings (referred to as dst) begin and end.
   *
   * Note: When the time zone changes, the local time and the alarms
   * still operate local.
   *
   * E.g. The local time is 15:00h and there is an alarm set at 16:00h.
   * After the time zone has changed, the local time within the new time
   * zone is still 15:00h and the alarm time will still appear at 16:00h
   * within the new time zone.
   *
   * @param timezone Refer to https://man7.org/linux/man-pages/man3/tzset.3.html
   *  for the explanation of the timezone string format.
   */
  static void tzset(const char* timezone) {
    setenv("TZ", timezone, true);
  }

  /**
   * Start RTC and optionally set time zone.
   */
  enum RTC_OSCILLATOR {RC = 0, XTAL = 1};
  void begin(const char* timezone = nullptr,
      const RTC_OSCILLATOR source = RTC_OSCILLATOR::XTAL);

  /**
   * Set the RTC by passing the local time as std::tm struct.
   *
   * Note: The RTC does not support dates before 1st of January 2000. Hence
   * the tm_year that is containing the elapsed years since 1900 must be
   * greater than 100.
   *
   * @time The local time.
   *    Note: time.tm_yday and time.wday fields may be random and
   *    tm_isdst (the daylight savings flag) can be set to -1. This is
   *    because all these fields will anyway be fixed before time is
   *    used to set the RTC. Fixing is performed by calling
   *    std::mktime(). This function uses the time zone information for
   *    the tm_isdst calculation. However, there is one situation
   *    when std::mktime() has to solve an ambiguity. This is when
   *    setting the time to the day and the hour, when time switches
   *    back from daylight savings to standard time. Typically that
   *    happens between 2:00h and 3:00h.
   *    So when setting the time to e.g. 2:30h at that day, the
   *    calculation assumes that you mean standard time 2:30h (after
   *    the hour has been switched back). If you mean 2:30h at daylight
   *    saving time (before the hour has been switched back), you have
   *    to set tm_dst to 1.
   *
   * @return The local time as expired seconds since 1st of January 1970.
   */
  std::time_t setByLocalTime(const std::tm &time);

  /**
   * Set the RTC by passing a Unix time stamp. Prerequisite: time zone
   *  is set correctly.
   */
  void setByUnixTime(std::time_t timestamp);

  /**
   * Get the local time from the RTC.
   */
  std::time_t getLocalTime(std::tm &time) const;

  /**
   * Get the Unix time stamp from the RTC. Prerequisite: time zone is
   *  set correctly.
   */
  std::time_t getUnixTime() const;

  /**
   * Set alarm time and date.
   *
   * @param alarm The alarm time and alarm date, to be set.
   */
  void setAlarm(const RtcSam3XA_Alarm& alarm);

  /**
   * Get the current RTC alarm time and date.
   *
   * @param alarm Reference to the variable receiving the alarm time
   *              and date.
   */
  void getAlarm(RtcSam3XA_Alarm &alarm);

  /**
   * @brief Delete alarm settings.
   *
   */
  void clearAlarm(){setAlarm(RtcSam3XA_Alarm());}

  /**
   * Set the callback to be called upon RTC alarm.
   *
   * @param alarmCallback The function to be called upon alarm.
   * @param alarmCallbackParam This parameter will be passed
   *  to the alarmCallback function.
   */
  void setAlarmCallback(void (*alarmCallback)(void* alarmCallbackParam),
      void *alarmCallbackParam = nullptr);

  /**
   * Set the callback being called upon every RTC second transition.
   *
   * @param secondCallback The function to be called upon second
   *  transition.
   * @param alarmCallbackParam This parameter will be passed
   *  to the secondCallback function.
   */
  void setSecondCallback(void (*secondCallback)(void*), void *secondCallbackParam = nullptr);

private:
  RtcSam3XA();
  friend void ::RTC_Handler();

  inline void RtcSam3XA_Handler();
  inline void RtcSam3XA_DstChecker();
  inline void RtcSam3XA_AckUpdHandler();

  enum SET_TIME_REQUEST {NO_REQUEST = 0, REQUEST, DST_RTC_REQUEST};
  volatile SET_TIME_REQUEST mSetTimeRequest;
  Sam3XA::RtcTime mSetTimeCache;

  void(*mSecondCallback)(void*);
  void* mSecondCallbackPararm;

  void(*mAlarmCallback)(void*);
  void* mAlarmCallbackPararm;

#if RTC_MEASURE_ACKUPD
private:
  uint32_t mTimestampACKUPD;
public:
  uint32_t getTimestampACKUPD() const {
    return mTimestampACKUPD;
  }
#endif
};

namespace TZ {
  /**
   * Some predefined time zone strings for convenience.
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
