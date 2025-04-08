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
#include "internal/RtcDueRcf_RtcState.h"

size_t RtcDueRcf_RtcState::printTo(Print& p) const {
  size_t result = 0;

  static const char szValidText[]  = {'i', 'v'};

  // Date and time
  result += p.print("cal:");
  result += p.print(szValidText[isCalendarValid()]);
  result += p.print(", tim:");
  result += p.print(szValidText[isTimeValid()]);

  // Calendar alarm
  result += p.print(", calalr:");
  {
    bool hasEnabledEntry = false;
    if(mRtcValidEntryRegister & (1 << RTC_RET_BITPOS_CALALR_MTHEN) ) {
      result += p.print('m');
      hasEnabledEntry = true;
    }
    if(mRtcValidEntryRegister & (1 << RTC_RET_BITPOS_CALALR_DATEEN) ) {
      result += p.print('d');
      hasEnabledEntry = true;
    }
    if(not hasEnabledEntry) {
      result += p.print('-');
    }
  }
  result += p.print(':');
  result += p.print(szValidText[isCalendarAlarmValid()]);

  // Time alarm
  result += p.print(", timalr:");
  {
    bool hasEnabledEntry = false;
    if(mRtcValidEntryRegister & (1 << RTC_RET_BITPOS_TIMALR_HOUREN) ) {
      result += p.print('h');
      hasEnabledEntry = true;
    }

    if(mRtcValidEntryRegister & (1 << RTC_RET_BITPOS_TIMALR_MINEN) ) {
      result += p.print('m');
      hasEnabledEntry = true;
    }

    if(mRtcValidEntryRegister & (1 << RTC_RET_BITPOS_TIMALR_SECEN) ) {
      result += p.print('s');
      hasEnabledEntry = true;
    }
    if(not hasEnabledEntry) {
      result += p.print('-');
    }
  }
  result += p.print(':');
  result += p.print(szValidText[isTimeAlarmValid()]);

  return result;
}

RtcDueRcf_RtcState::RtcDueRcf_RtcState(int rtcValidEntryRegister) :
    mRtcValidEntryRegister(rtcValidEntryRegister) {
}

bool RtcDueRcf_RtcState::isCalendarAlarmValid() const {
  return not (mRtcValidEntryRegister & RTC_VER_NVCALALR);
}

bool RtcDueRcf_RtcState::isCalendarAlarmEnabled() const {
  return (mRtcValidEntryRegister & RTC_RET_BITPOS_CALALR_DATEEN)
      || (mRtcValidEntryRegister & RTC_RET_BITPOS_CALALR_MTHEN);
}

bool RtcDueRcf_RtcState::isTimeAlarmValid() const {
  return not (mRtcValidEntryRegister & RTC_VER_NVTIMALR);
}

bool RtcDueRcf_RtcState::isTimeAlarmEnabled() const {
  return (mRtcValidEntryRegister & RTC_RET_BITPOS_TIMALR_SECEN)
      || (mRtcValidEntryRegister & RTC_RET_BITPOS_TIMALR_MINEN)
      || (mRtcValidEntryRegister & RTC_RET_BITPOS_TIMALR_HOUREN);
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

