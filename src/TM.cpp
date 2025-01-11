/*
 * TM.cpp
 *
 *  Created on: 11.01.2025
 *      Author: Wolfgang
 */

#include "TM.h"

TM::TM() : TM(0, 0, 0, 1, make_tm_month(TM::January), make_tm_year(1970), false) {
}

void TM::set(std::tm& time, int _sec, int _min, int _hour, int _mday, int _mon, int _year, bool _isdst) {
  time.tm_mday = _mday; time.tm_mon = _mon; time.tm_year = _year;
  time.tm_hour = _hour; time.tm_min = _min; time.tm_sec = _sec;
  time.tm_isdst = _isdst;
  time.tm_wday = -1; // unknown
  time.tm_yday = -1; // unknown
}

