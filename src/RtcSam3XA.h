/*
 * RtcSam3XA.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#ifndef RTCSAM3XA_SRC_RTCSAM3XA_H_
#define RTCSAM3XA_SRC_RTCSAM3XA_H_

#include "Sam3XA_RtcTime.h"
#include <cstdlib>
#include <ctime>
#include <include/rtc.h>

class RtcSam3XA {
public:
  /**
   * The one and only clock object.
   *
   * Usage examples:
   *
   * tm time;
   * RtcSam3XA::RtClock.getLocalTime(time);
   *
   * const tm time { 24, 59, 11, 12, 1, tm::make_tm_year(2016), false };
   * RtcSam3XA::RtClock.setByLocalTime(time);
   */
  static RtcSam3XA RtClock;

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
  void begin(RTC_OSCILLATOR source = RTC_OSCILLATOR::XTAL, const char* timezone = nullptr);

  /**
   * Set the RTC by the local time.
   */
  void setByLocalTime(const std::tm &time);

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

  void setSecondCallback(void (*secondCallback)(void*), void *secondCallbackParam = nullptr);

  // Class that extends struct std::tm with constructors and convenient functions.
  class tm : public std::tm {
  public:
    // make a tm::tm_year format from a year.
    static int make_tm_year(int ad_year /* anno domini year */) { return ad_year - 1900; }

    tm();
    tm(int _sec, int _min, int _hour, int _mday /* 1..31 */, int _mon /* 0..11 */, int _year /* year since 1900 */, bool _isdst);

//    /** Set this tm struct by Sam3XA RTC formatted struct. */
//    void set(const Sam3XA_RtcTime& sam2xatime) {set(*this, sam2xatime);}
  private:
    friend class RtcSam3XA;

    /** Set tm struct by Sam3XA RTC formatted struct. */
    static std::time_t set(std::tm& time, const Sam3XA_RtcTime& sam2xatime);
  };

private:
  // Interrupt handler
  void RtcSam3XA_Handler();
  RtcSam3XA();

  // Global interrupt handler forwards to RtcSam3XA_Handler()
  friend void ::RTC_Handler();
  volatile bool mSetTimeRequest;
  Sam3XA_RtcTime mSetTimeCache;
  void(*mSecondCallback)(void*);
  void* mSecondCallbackPararm;
  void(*mAlarmCallback)(void*);
  void* mAlarmCallbackPararm;
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
