/*
 * TM.cpp
 *
 *  Created on: 11.01.2025
 *      Author: Wolfgang
 */

#include <print.h>
#include "TM.h"

void TM::set(std::tm& time, int tm_sec, int tm_min, int tm_hour, int tm_mday
    , int tm_mon, int tm_year, int tm_isdst) {
  time.tm_mday = tm_mday; time.tm_mon = tm_mon; time.tm_year = tm_year;
  time.tm_hour = tm_hour; time.tm_min = tm_min; time.tm_sec = tm_sec;
  time.tm_isdst = tm_isdst;
  time.tm_wday = -1; // unknown
  time.tm_yday = -1; // unknown
}

bool TM::operator ==(const std::tm &other) const {
  return tm_sec == other.tm_sec && tm_min == other.tm_min && tm_hour == other.tm_hour && tm_mday == other.tm_mday
      && tm_year == other.tm_year && tm_isdst == other.tm_isdst;
}

size_t TM::printTo(Print& p) const {
  char buffer[26];
  asctime_r(this, buffer);

  buffer[24] = '\0'; // remove /n
  return p.print(buffer);
}
