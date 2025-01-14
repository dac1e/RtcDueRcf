/*
 * RtcSam3XA.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#pragma once

#ifndef RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_
#define RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_

#include <stdint.h>

class RtcSam3XA_Alarm {
public:
  static constexpr uint8_t INVALID_VALUE = UINT8_MAX;

  RtcSam3XA_Alarm();
  RtcSam3XA_Alarm(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon /* 0..11 */);

  void setSecond(int tm_sec) {second = tm_sec < 60 ? tm_sec : INVALID_VALUE;}
  void setMinute(int tm_min) {minute = tm_min < 60 ? tm_min : INVALID_VALUE;}
  void setHour(int tm_hour) {hour = tm_hour < 24 ? tm_hour : INVALID_VALUE;}
  void setDay(int tm_mday) {second = tm_mday < 32 ? tm_mday : INVALID_VALUE;}
  void setMonth(int tm_mon /* 0..11 */ ) {month = tm_mon < 12 ? tm_mon+1 : INVALID_VALUE;}

  void subtract(int _seconds /* 0.. (24 * 60 * 60 * 28) */, bool bIsLeapYear);
  void add(int _seconds /* 0.. (24 * 60 * 60 * 28) */, bool bIsLeapYear);

  bool operator==(const RtcSam3XA_Alarm& other) const {
    return second == other.second && minute == other.minute && hour == other.hour
        && day == other.day && month == other.month;
  }
private:
  RtcSam3XA_Alarm gaps2zero() const;
  RtcSam3XA_Alarm zero2gaps() const;
  friend class RtcSam3XA;
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month; // 1..12
};

#endif /* RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_ */
