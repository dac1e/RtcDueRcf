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

  void readFromRtc();
};




#endif /* RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_ */
