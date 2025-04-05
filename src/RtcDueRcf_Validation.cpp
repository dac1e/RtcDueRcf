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

#include "internal/core-sam-GapClose.h"
#include "RtcDueRcf_Validation.h"

RtcDueRcf_Validation::RtcDueRcf_Validation(int rtcValidEntryRegister) :
    mRtcValidEntryRegister(rtcValidEntryRegister) {
}

bool RtcDueRcf_Validation::isCalendarAlarmValid() const {
  return not (mRtcValidEntryRegister & RTC_VER_NVCALALR);
}

bool RtcDueRcf_Validation::isTimeAlarmValid() const {
  return !(mRtcValidEntryRegister & RTC_VER_NVTIMALR);
}

bool RtcDueRcf_Validation::isValid() const {
  return !(mRtcValidEntryRegister & (RTC_VER_NVCALALR | RTC_VER_NVTIMALR));
}
