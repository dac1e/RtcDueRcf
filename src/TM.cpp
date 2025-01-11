/*
 * TM.cpp
 *
 *  Created on: 11.01.2025
 *      Author: Wolfgang
 */

#include "TM.h"
#include "print.h"

namespace {

/**
 * Calculate whether a given year is a leap year.
 * @param tm_year The offset to 1900 of the year to be used for
 *  the calculation. This is the same format as the tm_year
 *  part of the std::tm;
 * @return the number of leap years since the given year.
 *  This number includes the given tm_year if it is a leap year.
 */
int isLeapYear(int tm_year) {
  const int year = tm_year + 1900;
  const int result = ( !(year % 4) && ( (year % 100) || !(year % 400) ) );;
  return result;
}

/**
 * Calculate the number of leap years since 1970 for a given year.
 * @param tm_year The offset to 1900 of the year to be used for
 *  the calculation. This is the same format as the tm_year part
 *  of the std::tm;
 * @return 1 if the given year is a leap year. Otherwise 0.
 */
int leapYearsSince1970 (int tm_year) {
  const int year = tm_year + 1900;
  const int yearsDiv4count   = (year-1968 /* first year that divides by   4 without rest. */) /   4;
  const int yearsDiv100count = (year-1900 /* first year that divides by 100 without rest. */) / 100;
  const int yearsDiv400count = (year-1600 /* first year that divides by 400 without rest. */) / 400;
  return yearsDiv4count - yearsDiv100count + yearsDiv400count;
}

/**
 * Calculate the time difference between local daylight saving
 * time and local standard time.
 * @param time The tm structure that holds the tm_isdst flag to
 *  be evaluated for the return value.
 * @return The time difference in seconds if the tm_isdst flag
 *  in time is is > 0. Otherwise 0.
 */
std::time_t dstdiff(const std::tm& time) {
  if(time.tm_isdst > 0) {
    const __tzinfo_type * tzinfo =  __gettzinfo();
    const int tz_offset = tzinfo->__tzrule[0].offset;
    const int tz_dstoffset = tzinfo->__tzrule[1].offset;
    const int dst_diff = tz_dstoffset - tz_offset;
    return dst_diff;
  }
  return 0;
}

} // anonymous namespace


std::time_t TM::mkgmtime(const std::tm& time) {
  using time_t = std::time_t;
  const bool leapYear = isLeapYear(time.tm_year);
  const time_t leapYearsBeforeThisYear = leapYearsSince1970(time.tm_year) - leapYear;
  const time_t yearOffset = time.tm_year - 70;
  const time_t result = time.tm_sec + (time.tm_min + (time.tm_hour + (time.tm_yday +
      leapYearsBeforeThisYear + yearOffset * 365) * 24) * 60) * 60 + dstdiff(time);
  return result;
}

TM::TM() : TM(0, 0, 0, 1, make_tm_month(TM::January), make_tm_year(2000), false) {
}

void TM::set(std::tm& time, int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon, int tm_year, bool tm_isdst) {
  time.tm_mday = tm_mday; time.tm_mon = tm_mon; time.tm_year = tm_year;
  time.tm_hour = tm_hour; time.tm_min = tm_min; time.tm_sec = tm_sec;
  time.tm_isdst = tm_isdst;
  time.tm_wday = -1; // unknown
  time.tm_yday = -1; // unknown
}

size_t TM::printTo(Print& p) const {
  char buffer[26];
  asctime_r(this, buffer);

  buffer[24] = '\0'; // remove /n
  p.print(buffer);
}
