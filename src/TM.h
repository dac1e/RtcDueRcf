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

// TM extends struct std::tm with constructors and convenient functions and enumerations.
// Also make the std::tm struct also printable by Streams like 'Serial'.
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

  enum DST : int {
    DST_UNKNOWN = -1, DST_FALSE = 0, DST_TRUE = 1,
  };

  // Make a tm::tm_month representation from TM_MONTH enumeration.
  static int make_tm_month(MONTH month) {return static_cast<int>(month);}

  // Make a tm::tm_year representation from an anno domini year.
  static int make_tm_year(int ad_year /* anno domini year */) { return ad_year - 1900; }

  static int make_tm_dst(DST dst) {return static_cast<int>(dst);}

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
  TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon, int ad_year, DST dst);
  TM(int tm_sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year/* year since 1900 */, int tm_isdst);

  void set(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon, int ad_year, DST dst);
  void set(int _sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year/* year since 1900 */, int tm_isdst);

  /**
   * Convert this time to a unix time stamp.
   * Prerequisite: tm_yday and tm_isdst are set correctly.
   *
   * @return The unix time stamp for the given time.
   */
  std::time_t mkgmtime() const { return mkgmtime(*this); }

  /**
   * Convert time to a unix time stamp.
   * Prerequisite: tm_yday and tm_isdst are set correctly.
   *
   * @param time The time for which to calculate the time stamp.
   *
   * @return The unix time stamp for the given time.
   */
  static std::time_t mkgmtime(const std::tm& time);

private:
  /**
   * Set a time struct from time components
   *
   * @param time The time struct to be set.
   */
  static void set(std::tm& time, int tm_sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year /* year since 1900 */, int tm_isdst);

  /**
   * Implementation of interface printable. Allows to print the time struct.
   *
   * Example:
   *  TM time; // Create a time structure with default contents.
   *  Serial.prinln(tm); // Print it.
   */
  size_t printTo(Print& p) const override;
};

inline TM::TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
    int tm_year, int tm_isdst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_isdst);
}

inline TM::TM(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon,
    int ad_year, DST dst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, make_tm_month(mon),
      make_tm_year(ad_year), make_tm_dst(dst));
}

inline void TM::set(int _sec, int tm_min, int tm_hour, int tm_mday, int tm_mon,
    int tm_year, int tm_isdst) {
  set(*this, _sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_isdst);
}

inline void TM::set(int tm_sec, int tm_min, int tm_hour, int tm_mday, MONTH mon,
    int ad_year, DST dst) {
  set(*this, tm_sec, tm_min, tm_hour, tm_mday, make_tm_month(mon),
      make_tm_year(ad_year), make_tm_dst(dst));
}

#endif /* RTCSAM3XA_SRC_TM_H_ */
