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
  void setSecond(int _second) {second = _second;}
  void setMinute(int _minute) {minute = _minute;}
  void setHour(int _hour) {hour = _hour;}
  void setDay(int _day) {second = _day;}
  void setMonth(int _month) {second = _month;}

  void subtract(int _seconds /* 0.. (24 * 60 * 60 * 28) */, bool bIsLeapYear);
private:
  friend class RtcSam3XA;
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month; // 1..12
};

#endif /* RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_ */
