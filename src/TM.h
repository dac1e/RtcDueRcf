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

/**
 * class TM extends struct std::tm with constructors and convenient functions and enumerations.
 * TM implements the interface Printable. This allows to print this class.
 *
 * Example:
 *  TM time; // Create a time structure with default contents.
 *  Serial.prinln(tm); // Print it.
 */
class TM : public std::tm, public Printable {
public:
  // Note: The following constructors and setters will not set the tm_yday and tm_wday component.
  // Call std::mktime() passing the TM class or std::tm struct for which you want to
  // have those fields set. The return value of this function is unimportant in that case.
  // std::mktime() will also change tm_isdst according to the daylight savings setting of the
  // actual time zone. Hence the time zone information needs to be set up front. Refer to
  // RtcSam3XA::begin() and RtcSam3XA::tz_set().
  TM(int tm_sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year/* year since 1900 */, int tm_isdst);
  TM() : TM(0, 0, 0, 1, 0, make_tm_year(2000), -1) {}

  void set(int _sec, int tm_min, int tm_hour, int tm_mday,
      int tm_mon /* 0..11 */, int tm_year/* year since 1900 */, int tm_isdst);

  bool operator ==(const TM &other) const;

  // Make a tm::tm_year representation from an anno domini year.
  static int make_tm_year(int ad_year /* anno domini year */) { return ad_year - 1900; }

private:
  /**
   * Set a time struct from time components
   *
   * @param time The time struct to be set.
   */
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

#endif /* RTCSAM3XA_SRC_TM_H_ */
