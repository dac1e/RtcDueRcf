/*
  RtcDueRcf - Arduino libary for Arduino Due - builtin RTC Copyright (c)
  2024 Wolfgang Schmieder.  All right reserved.

  Contributors:
  - Wolfgang Schmieder

  Project home: https://github.com/dac1e/RtcDueRcf

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
