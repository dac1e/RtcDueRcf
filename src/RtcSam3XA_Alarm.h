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

#ifndef RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_
#define RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_

#include <stdint.h>
#include <Printable.h>

/**
 * The class RtcSam3XA_Alarm is used to operate the alarm features of
 *  the RTC.
 *
 * An alarm consist the following specifiers second, minute, hour,
 *  day, year that determines when the alarm shall appear.
 * Any of those fields may be set to INVALID_VALUE. If a
 *  field is set to INVALID_VALUE, only other fields no set to
 *  INVALID_VALUE will determine when the alarm shall appear.
 *
 * Set alarm example:
 *
 *  // Create an alarm with all fields set to INVALID_VALUE.
 *  RtcSam3XA_Alarm alarm;
 *
 *  // Let an alarm appear whenever the hour is 13 and the second
 *  // is 40. I.e. the alarm appears every day between
 *  // [13:00 to 13:59], whenever the second transitions to 40.
 *  alarm.setHour(13)
 *  alarm.setSecond(40)
 *  RtcSam3XA::clock.setAlarm(alarm);

 *
 * RtcSam3XA_Alarm implements the interface Printable. This allows
 *  to print this class.
 *
 * Print example:
 *  RtcSam3XA_Alarm alarm;// Create an alarm structure with defaults.
 *  Serial.prinln(alarm); // Print it.
 */
class RtcSam3XA_Alarm : public Printable {
  friend class RtcSam3XA;
public:
  RtcSam3XA_Alarm();
  RtcSam3XA_Alarm(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon /* 0..11 */);

  static constexpr uint8_t INVALID_VALUE = UINT8_MAX;

  void setSecond(int tm_sec) {second = tm_sec < 60 ? tm_sec : INVALID_VALUE;}
  void setMinute(int tm_min) {minute = tm_min < 60 ? tm_min : INVALID_VALUE;}
  void setHour(int tm_hour) {hour = tm_hour < 24 ? tm_hour : INVALID_VALUE;}
  void setDay(int tm_mday) {day = tm_mday < 32 ? tm_mday : INVALID_VALUE;}
  void setMonth(int tm_mon) {month = tm_mon < 12 ? tm_mon+1 : INVALID_VALUE;}

  bool operator ==(const RtcSam3XA_Alarm &other) const;

private:
  static size_t printMember(Print &p, const uint8_t m);
  size_t printTo(Print& p) const override;

  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month; // 1..12
};
#endif /* RTCSAM3XA_SRC_RTCSAM3XA_ALARM_H_ */
