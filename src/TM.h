/*
 * TM.h
 *
 *  Created on: 11.01.2025
 *      Author: Wolfgang
 */

#pragma once

#ifndef RTCSAM3XA_SRC_TM_H_
#define RTCSAM3XA_SRC_TM_H_

#include <ctime>
#include <printable.h>

// Class that extends struct std::tm with constructors and convenient functions and enums.
class TM : public std::tm, public Printable {
public:
  // Enumeration that matches the tm_month representation of the tm struct.
  enum MONTH : int {
    January = 0, February, March, April, May, June, July, August, September, October, December
  };

  // Enumeration that matches the tm_wday representation of the tm struct.
  enum DAY_OF_WEEK : int {
    SUNDAY = 0, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY,
  };

  // make tm::tm_month from TM_MONTH enum.
  static int make_tm_month(MONTH month) {return static_cast<int>(month);}

  // make tm::tm_year format from a year.
  static int make_tm_year(int ad_year /* anno domini year */) { return ad_year - 1900; }

  static DAY_OF_WEEK wday(const std::tm& time) {return static_cast<DAY_OF_WEEK>(time.tm_wday);}
  static MONTH month(const std::tm& time) {return static_cast<MONTH>(time.tm_mon);}

  DAY_OF_WEEK wday() {return wday(*this);}
  MONTH month() {return month(*this);}

  // Note: The following constructors and setters will not set the tm_yday and tm_wday component.
  // You must call std::mktime() passing the TM class or std::tm struct for which you want to
  // have those fields set. The return value of this function is unimportant in that case.
  // std::mktime() will also change tm_isdst according to the daylight savings setting of the
  // actual time zone. Hence the time zone information needs to be set up front. Refer to
  // RtcSam3XA::begin() and RtcSam3XA::tz_set().
  TM();
  TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon, int ad_year, bool tm_isdst);
  TM(int tm_sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year/* year since 1900 */, bool tm_isdst);

  void set(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon, int ad_year, bool tm_isdst);

  void set(int _sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year/* year since 1900 */, bool tm_isdst);

  static void set(std::tm& time, int tm_sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year /* year since 1900 */, bool tm_isdst);

  std::time_t mkgmtime() const { return mkgmtime(*this); }
  /**
   * Convert time to a unix time stamp. Neither a time zone nor a daylight saving shift is
   * considered in this calculation. Prerequisite: tm_yday and tm_isdst are set correctly.
   * @param time The time for which to calculate the time stamp.
   *
   * @return The unix time stamp for the given time.
   */
  static std::time_t mkgmtime(const std::tm& time);

  /**
   * Add seconds to the value of this time structure. Seconds may be
   * positive or negative.
   * @param seconds The seconds to be added to this time structure.
   *
   * @return A time structure containing the value of this time
   *  increased by the given amount of seconds.
   */
  const TM operator+(const std::time_t seconds) const;

  /**
   * Test whether the value of this time structure is equal to another
   * one. Note That the tm_isdst flag must also match for equality.
   *
   * @param other The time structure to compare with.
   *
   * @return true, if both structures are equal. Otherwise false.
   */
  bool operator ==(const std::tm &other) const;

  /**
   * Implementation of interface printable.
   */
  size_t printTo(Print& p) const override;
};

inline TM::TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
    int tm_year, bool tm_isdst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_isdst);
}

inline TM::TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon,
    int ad_year, bool tm_isdst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, make_tm_month(mon),
      make_tm_year(ad_year), tm_isdst);
}

inline void TM::set(int _sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
    int tm_year, bool tm_isdst) {
  set(*this, _sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_isdst);
}

inline void TM::set(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon,
    int ad_year, bool tm_isdst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, make_tm_month(mon),
      make_tm_year(ad_year), tm_isdst);
}

#endif /* RTCSAM3XA_SRC_TM_H_ */
