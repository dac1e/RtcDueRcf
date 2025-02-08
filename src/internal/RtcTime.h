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

#pragma once

#ifndef RTCSAM3XA_SRC_INTERNAL_SAM3XA_RTCTIME_H_
#define RTCSAM3XA_SRC_INTERNAL_SAM3XA_RTCTIME_H_

#include <stdint.h>
#include <ctime>
#include <utility>

class Stream;

namespace Sam3XA {

/**
 * A class to read RTC registers from and write RTC registers
 * to the Sam3X RTC.
 * This class will always hold a 24-hrs representation of the
 * time independent of what the current RTC hrs mode is.
 * Conversion from/to 12 hrs mode will be performed if needed.
 */
class RtcTime {
  static constexpr int32_t TM_YEAR_BASE = 1900;

  static inline int tmMonth(uint8_t month) {return month-1;}
  static uint8_t tmDayOfWeek(const std::tm &time);

  void getRaw(std::tm &time) const;
  void readFromRtc_();

public:
  inline uint8_t hour() const {return mHour;}
  inline uint8_t minute() const {return mMinute;}
  inline uint8_t second() const {return mSecond;}
  inline uint16_t year() const {return mYear;}
  inline uint8_t month() const {return mMonth;}
  inline uint8_t day() const {return mDayOfMonth;}
  inline uint8_t day_of_week() const {return mDayOfWeekDay;}
  inline uint8_t rtc12hrsMode() const {return mRtc12hrsMode > 0;}

  inline int tm_hour() const {return mHour;}
  inline int tm_min() const {return mMinute;}
  inline int tm_sec() const {return mSecond;}
  inline int tm_year() const {return mYear - TM_YEAR_BASE;}
  inline int tm_mon() const {return tmMonth(mMonth);}
  inline int tm_mday() const {return mDayOfMonth;}
  inline int tm_wday() const {return mDayOfWeekDay-1;}

  static uint16_t rtcYear(const std::tm& time) {return time.tm_year + 1900;}
  static uint8_t  rtcMonth(const std::tm& time) {return time.tm_mon + 1;}
  static uint8_t  rtcDayOfWeek(const std::tm& time) {return tmDayOfWeek(time) + 1;}

  /** Get a tm struct from this RtcTime. */
  std::time_t get(std::tm &time) const;

  /** Set RtcTime from a tm struct. */
  void set(const std::tm &time);

  Sam3XA::RtcTime operator+(const time_t sec) const;
  Sam3XA::RtcTime operator-(const time_t sec) const;
  bool operator==(const RtcTime& other)const {
    return mHour == other.mHour &&
    mMinute == other.mMinute &&
    mSecond == other.mSecond &&
    mRtc12hrsMode == other.mRtc12hrsMode &&
    mYear == other.mYear &&
    mMonth == other.mMonth &&
    mDayOfMonth == other.mDayOfMonth &&
    mDayOfWeekDay == other.mDayOfWeekDay &&
    mState == other.mState;
  }

  /**
   * Read the RTC time and date and stores the result in this object.
   * Convert the result to 24 hrs mode if RTC runs in 12 hrs mode.
   */
  void readFromRtc();

  /**
   * Write the time and date of this object to the RTC. If RTC runs
   * in 12-hrs mode, RTC registers will be set with a 12-hrs mode
   * time format. I.e. hours mode of the RTC isn't changed.
   */
  void writeToRtc() const;

  /**
   * Determine whether this time is within daylight savings period.
   * Let the RTC run in 12-hrs mode during the daylight savings
   * period. Let it run in 24-hrs mode outside of the daylight
   * savings period (as per software design decision).
   */
  static int isdst(Sam3XA::RtcTime& stdTime, Sam3XA::RtcTime& dstTime);

  bool dstRtcRequest();

  uint8_t isValid() const {return mState != INVALID;}
  uint8_t isFromRtc() const {return mState == FROM_RTC;}
//  void clearFromRtcFlag() {mState = 0;}
  static const Sam3XA::RtcTime* getDstBeginCompareTime(Sam3XA::RtcTime& stdTime, const Sam3XA::RtcTime& dstTime,
      const int32_t dstTimeShift);
  static const Sam3XA::RtcTime* getDstEndCompareTime(const Sam3XA::RtcTime& stdTime, Sam3XA::RtcTime& dstTime,
      const int32_t dstTimeShift);

  void set(const std::time_t timestamp, const uint8_t isdst);
  std::time_t toTimeStamp() const;

private:
  enum STATE : uint8_t {
    INVALID,
    VALID,
    FROM_RTC,
  };

  STATE mState = INVALID;

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
};

} // namespace Sam3XA_Rtc

#endif /* RTCSAM3XA_SRC_INTERNAL_SAM3XA_RTCTIME_H_ */
