/*
 * TM.cpp
 *
 *  Created on: 11.01.2025
 *      Author: Wolfgang
 */

#include "TM.h"
#include "print.h"


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
