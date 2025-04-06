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

#include "Print.h"
#include "internal/core-sam-GapClose.h"
#include "RtcDueRcf_RtcState.h"


size_t RtcDueRcf_RtcState::printTo(Print& p) const {
  size_t result = 0;

  static const char szText[] = {'i', 'v'};

  result += p.print("cal:");
  result += p.print(szText[isCalendarValid()]);
  result += p.print(", tim:");
  result += p.print(szText[isTimeValid()]);
  result += p.print(", calalr:");
  result += p.print(szText[isCalendarAlarmValid()]);
  result += p.print(", timalr:");
  result += p.print(szText[isTimeAlarmValid()]);
  return result;
}

RtcDueRcf_RtcState::RtcDueRcf_RtcState(int rtcValidEntryRegister) :
    mRtcValidEntryRegister(rtcValidEntryRegister) {
}

bool RtcDueRcf_RtcState::isCalendarAlarmValid() const {
  return not (mRtcValidEntryRegister & RTC_VER_NVCALALR);
}

bool RtcDueRcf_RtcState::isTimeAlarmValid() const {
  return !(mRtcValidEntryRegister & RTC_VER_NVTIMALR);
}

bool RtcDueRcf_RtcState::isAlarmValid() const {
  return isTimeAlarmValid() && isCalendarAlarmValid();
}

bool RtcDueRcf_RtcState::isCalendarValid() const {
  return not (mRtcValidEntryRegister & RTC_VER_NVCAL);
}

bool RtcDueRcf_RtcState::isTimeValid() const {
  return !(mRtcValidEntryRegister & RTC_VER_NVTIM);
}

