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

#include <assert.h>
#include <print.h>

#include "RtcSam3XA_Alarm.h"

size_t RtcSam3XA_Alarm::printTo(Print &p) const {
  size_t result = 0;
  result += p.print("sec:"); result += printMember(p, second);
  result += p.print(" min:"); result += printMember(p, minute);
  result += p.print(" hour:"); result += printMember(p, hour);
  result += p.print(" day:"); result += printMember(p, day);
  result += p.print(" month:"); result += printMember(p, month);
  return result;
}

bool RtcSam3XA_Alarm::operator ==(const RtcSam3XA_Alarm &other) const {
  return second == other.second && minute == other.minute && hour == other.hour && day == other.day
      && month == other.month;
}

size_t RtcSam3XA_Alarm::printMember(Print &p, const uint8_t m) {
  if (m != INVALID_VALUE) {
    return p.print(m);
  }
  return p.print("--");
}

RtcSam3XA_Alarm::RtcSam3XA_Alarm() :
    second(INVALID_VALUE), minute(INVALID_VALUE), hour(INVALID_VALUE), day(INVALID_VALUE), month(INVALID_VALUE) {
}

RtcSam3XA_Alarm::RtcSam3XA_Alarm(int tm_sec, int tm_min, int tm_hour, int tm_mday, int tm_mon)
  : second(tm_sec < 60 ? tm_sec : INVALID_VALUE), minute(tm_min < 60 ? tm_min : INVALID_VALUE)
  , hour(tm_hour < 24 ? tm_hour : INVALID_VALUE), day(tm_mday < 32 ? tm_mday : INVALID_VALUE)
  , month(tm_mon < 12 ? tm_mon+1 : INVALID_VALUE) {
}

