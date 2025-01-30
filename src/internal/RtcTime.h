/*
 * Sam3XATime.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#pragma once

#ifndef RTCSAM3XA_SRC_INTERNAL_SAM3XA_RTCTIME_H_
#define RTCSAM3XA_SRC_INTERNAL_SAM3XA_RTCTIME_H_

#include <stdint.h>
#include <ctime>

#define RTC_DEBUG_HOUR_MODE true

class Stream;

namespace Sam3XA {

/** Time structure to read from and write to the Sam3X RTC. */
class RtcTime {
  static inline int tmMonth(uint8_t month) {return month-1;}
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
  inline uint8_t day() const {return mDayOfMonth;}
  inline uint8_t day_of_week() const {return mDayOfWeekDay;}
  inline uint8_t rtc12hrsMode() const {return mRtc12hrsMode > 0;}

  inline int tm_hour()const {return mHour;}
  inline int tm_min()const {return mMinute;}
  inline int tm_sec()const {return mSecond;}
  inline int tm_year()const {return mYear - 1900;}
  inline int tm_mon()const {return tmMonth(mMonth);}
  inline int tm_mday()const {return mDayOfMonth;}
  inline int tm_wday()const {return mDayOfWeekDay-1;}

  static uint16_t rtcYear(const std::tm& time) {return time.tm_year + 1900;}
  static uint8_t  rtcMonth(const std::tm& time) {return time.tm_mon + 1;}
  static uint8_t  rtcDayOfWeek(const std::tm& time) {return tmDayOfWeek(time) + 1;}

  /** Get a tm struct from this Sam3XA RTC struct. */
  std::time_t get(std::tm &time) const;

  /** Set this Sam3XA RTC struct from a tm struct. */
  void set(const std::tm &time);

  /** Read 24 hour representation independent of the hrs mode the RTC is running in. */
  void readFromRtc();
  void writeToRtc() const;

  /** Determine whether this time is within daylight savings period. */
  bool dstRtcRequest(std::tm &time);
  int isdst() const;

private:

  uint8_t mHour;
  uint8_t mMinute;
  uint8_t mSecond;

  //  0: RTC runs in 24-hrs mode.
  //  1: RTC runs in 12-hrs mode.
  uint8_t  mRtc12hrsMode;

  uint16_t mYear; // 4 digits ad year
  uint8_t mMonth; // 1..12
  uint8_t mDayOfMonth;  // 1..31
  uint8_t mDayOfWeekDay;// 1..7

  uint8_t mIsFromRTC;
};

} // namespace Sam3XA_Rtc

#endif /* RTCSAM3XA_SRC_INTERNAL_SAM3XA_RTCTIME_H_ */
