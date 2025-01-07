/*
 * Sam3XATime.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#ifndef RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_
#define RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_

#include <stdint.h>

// Time structure to read from and write to the Sam3X RTC.
class Sam3XA_RtcTime {
  uint8_t mHour;
  uint8_t mMinute;
  uint8_t mSecond;

  uint16_t mYear;
  uint8_t mMonth;
  uint8_t mDay;
  uint8_t mWeek;

public:
  inline uint8_t hour() const {return mHour;}
  inline uint8_t minute() const {return mMinute;}
  inline uint8_t second() const {return mSecond;}
  inline uint16_t year() const {return mYear;}
  inline uint8_t month() const {return mMonth;}
  inline uint8_t day() const {return mDay;}
  inline uint8_t week() const {return mWeek;}

  inline int tmHour()const {return mHour;}
  inline int tmMinute()const {return mMinute;}
  inline int tmSecond()const {return mSecond;}
  inline int tmYear()const {return mYear;}
  inline int tmMonth()const {return mMonth - 1;}
  inline int tmDay()const {return mDay;}
  inline int tmWeek()const {return mWeek - 1;}

  static uint8_t rtcMonth(const std::tm& t) {return t.tm_mon + 1;}
  static uint8_t rtcDayOfWeek(const std::tm& t) {return t.tm_wday + 1;}

  void set(const std::tm &t) {
    mHour = t.tm_hour; mMinute = t.tm_min; mSecond = t.tm_sec;
    mYear = t.tm_year; mMonth = rtcMonth(t); mDay = t.tm_mday;
    mWeek = rtcDayOfWeek(t);
  }

  void readFromRtc();
};




#endif /* RTCSAM3XA_SRC_SAM3XA_RTCTIME_H_ */
