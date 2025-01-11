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

// Class that extends struct std::tm with constructors and convenient functions.
class TM : public std::tm {
public:
  // Enumeration that matches the tm_month representation of the tm struct.
  enum TM_MONTH : int {January = 0, February, March, April, May, June, July, August, September, October, December};
  static int make_tm_month(TM_MONTH month) {return static_cast<int>(month);}

  // make a tm::tm_year format from a year.
  static int make_tm_year(int ad_year /* anno domini year */) { return ad_year - 1900; }

  TM();

  TM(int _sec, int _min, int _hour, int _mday /* 1..31 */, int _mon /* 0..11 */,
      int _year /* year since 1900 */, bool _isdst)
  {
    set(*this, _sec, _min, _hour, _mday /* 1..31 */, _mon /* 0..11 */,
        _year /* year since 1900 */, _isdst);
  }

  void set(int _sec, int _min, int _hour, int _mday /* 1..31 */, int _mon /* 0..11 */,
      int _year /* year since 1900 */, bool _isdst)
  {
    set(*this, _sec, _min, _hour, _mday /* 1..31 */, _mon /* 0..11 */,
        _year /* year since 1900 */, _isdst);
  };
private:
  static void set(std::tm& time, int _sec, int _min, int _hour,
      int _mday /* 1..31 */, int _mon /* 0..11 */,
      int _year /* year since 1900 */, bool _isdst);
};


#endif /* RTCSAM3XA_SRC_TM_H_ */
