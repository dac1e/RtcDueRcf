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

#ifndef RTCDUERCF_SRC_TM_H_
#define RTCDUERCF_SRC_TM_H_

#include <ctime>
#include <printable.h>

/**
 * class TM extends struct std::tm with constructors and convenient functions.
 * TM implements the interface Printable. This allows to print this class.
 *
 * Print example:
 *  TM time; // Create a time structure with default contents.
 *  Serial.prinln(tm); // Print it.
 */
class TM : public std::tm, public Printable {
public:

  /**
   * Constructor initializing the std::tm structure fields without
   * the tm_yday and tm_wday component. Calling std::mktime() with
   * a pointer to TM class or std::tm struct can be used fill
   * those fields if needed.
   *
   * @param tm_sec Seconds [0..59].
   * @param tm_min Minutes [0..59].
   * @param tm_hour Hours  [0..23].
   * @param tm_mday Day within month [1..31].
   * @param tm_mon Months since January [0..11].
   * @param tm_year Year since 1900 [100..].
   * @param tm_isdst Daylight savings flag:
   *  The value is positive if daylight savings is in effect, zero if it is
   *  not in effect, and negative if no information is available.
   */
  TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
      int tm_year, int tm_isdst);

  /**
   * Constructor initializing the std::tm structure with 1st of January 2000
   * 00:00:00h and the daylight savings flag tm_dst to -1 (-1 means no
   * information is available). Will not set the tm_yday and tm_wday fields.
   * Calling std::mktime() with a pointer to TM class or std::tm struct can
   * be used fill those fields if needed. */
  TM() : TM(0, 0, 0, 1, 0, make_tm_year(2000), -1) {}

  /**
   * Modify the std::tm structure fields. Will not set the tm_yday
   * and tm_wday component.
   * Calling std::mktime() with a pointer to TM class or std::tm struct
   * will fill those fields.
   *
   * @param tm_sec Seconds [0..59].
   * @param tm_min Minutes [0..59].
   * @param tm_hour Hours  [0..23].
   * @param tm_mday Day within month [1..31].
   * @param tm_mon Months since January [0..11].
   * @param tm_year Year since 1900 [100..].
   * @param tm_isdst Daylight savings flag:
   *  The value is positive if daylight savings is in effect, zero if it is
   *  not in effect, and negative if no information is available.
   */
  void set(int _sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
      int tm_year, int tm_isdst);

  /**
   * Test if this tm structure is equal to another one. fields tm_yday and
   * tm_wday are not used for comparison, because they are optional fields
   * containing redundant time information that can be calculated from
   * other tm structure fields.
   *
   * @param other The other tm structure to be used for comparison.
   */
  bool operator ==(const std::tm &other) const;

  /**
   * Make a tm::tm_year representation from an anno domini year.
   */
  static int make_tm_year(int ad_year /* anno domini year */) { return ad_year - 1900; }

private:
  static void set(std::tm& time, int tm_sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year /* year since 1900 */, int tm_isdst);

  size_t printTo(Print& p) const override;
};


inline TM::TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
    int tm_year, int tm_isdst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_isdst);
}

inline void TM::set(int _sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
    int tm_year, int tm_isdst) {
  set(*this, _sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_isdst);
}

#endif /* RTCDUERCF_SRC_TM_H_ */
