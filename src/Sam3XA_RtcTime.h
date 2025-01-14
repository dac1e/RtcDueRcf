/*
 * Sam3XATime.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#pragma once

#ifndef RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_
#define RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_

#include <stdint.h>
#include <ctime>

namespace Sam3XA {

// Time structure to read from and write to the Sam3X RTC.
class RtcTime {
  uint8_t mHour;
  uint8_t mMinute;
  uint8_t mSecond;

  uint16_t mYear;
  uint8_t mMonth; // 1..12
  uint8_t mDayOfMonth;  // 1..31
  uint8_t mDayOfWeekDay;// 1..7

  static uint8_t tmDayOfWeek(const std::tm &time) {
    // Calling mktime will fix tm_wday.
    std::tm t = time; (void)std::mktime(&t); return t.tm_wday;
  }
  static inline int tmMonth(uint8_t month) {return month - 1;}

public:
  inline uint8_t hour() const {return mHour;}
  inline uint8_t minute() const {return mMinute;}
  inline uint8_t second() const {return mSecond;}
  inline uint16_t year() const {return mYear;}
  inline uint8_t month() const {return mMonth;}
  inline uint8_t day() const {return mDayOfMonth;}
  inline uint8_t day_of_week() const {return mDayOfWeekDay;}

  inline int tmHour()const {return mHour;}
  inline int tmMinute()const {return mMinute;}
  inline int tmSecond()const {return mSecond;}
  inline int tmYear()const {return mYear - 1900;}
  inline int tmMonth()const {return tmMonth(mMonth);}
  inline int tmDayOfMonth()const {return mDayOfMonth;}
  inline int tmDayOfWeek()const {return mDayOfWeekDay - 1;}

  static uint16_t rtcYear(const std::tm& time) {return time.tm_year + 1900;}
  static uint8_t rtcMonth(const std::tm& time) {return time.tm_mon + 1;}
  static uint8_t rtcDayOfWeek(const std::tm& time) {return tmDayOfWeek(time) + 1;}

  /** Set this Sam3XA RTC struct from a tm struct. */
  void set(const std::tm &time);

  /** Get a tm struct from this Sam3XA RTC struct. */
  std::time_t get(std::tm &time) const;
  int isdst() const;
  static int monthLength(uint8_t month /* 1..12 */, bool bIsLeapYear);
  static int isLeapYear(uint16_t year);

  void readFromRtc();
};

} // namespace Sam3XA_Rtc

#endif /* RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_ */
