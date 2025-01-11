/*
 * Sam3XATime.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#ifndef RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_
#define RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_

#include <stdint.h>
#include <ctime>

// Time structure to read from and write to the Sam3X RTC.
class Sam3XA_RtcTime {
  uint8_t mHour;
  uint8_t mMinute;
  uint8_t mSecond;

  uint16_t mYear;
  uint8_t mMonth;
  uint8_t mDay;
  uint8_t mWeekDay;

  static uint8_t tmDayOfWeek(const std::tm &time) {
    // Calling mktime will fix tm_wday.
    std::tm t = time; (void)std::mktime(&t); return t.tm_wday;
  }
public:
  inline uint8_t hour() const {return mHour;}
  inline uint8_t minute() const {return mMinute;}
  inline uint8_t second() const {return mSecond;}
  inline uint16_t year() const {return mYear;}
  inline uint8_t month() const {return mMonth;}
  inline uint8_t day() const {return mDay;}
  inline uint8_t week() const {return mWeekDay;}

  inline int tmHour()const {return mHour;}
  inline int tmMinute()const {return mMinute;}
  inline int tmSecond()const {return mSecond;}
  inline int tmYear()const {return mYear;}
  inline int tmMonth()const {return mMonth - 1;}
  inline int tmDay()const {return mDay;}
  inline int tmWeek()const {return mWeekDay - 1;}

  static uint8_t rtcMonth(const std::tm& time) {return time.tm_mon + 1;}
  static uint8_t rtcDayOfWeek(const std::tm& time) {return tmDayOfWeek(time) + 1;}

  void set(const std::tm &time) {
    mHour = time.tm_hour; mMinute = time.tm_min; mSecond = time.tm_sec;
    mYear = time.tm_year; mMonth = rtcMonth(time); mDay = time.tm_mday;
    mWeekDay = rtcDayOfWeek(time);
  }

  /** Get a tm struct from this Sam3XA RTC struct. */
  std::time_t get(std::tm& time) const {
    time.tm_hour = tmHour(); time.tm_min = tmMinute(); time.tm_sec = tmSecond();
    time.tm_year = tmYear(); time.tm_mon = tmMonth(); time.tm_mday = tmDay();
    time.tm_wday = tmWeek(); time.tm_yday = 0; time.tm_isdst = 0 /* We get standard time from Rtc. */;

    // mktime will fix the tm_yday as well as tm_ist and the
    // hour, if time is within daylight savings period.
    return mktime(&time);
  }

  void readFromRtc();
};




#endif /* RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_ */
