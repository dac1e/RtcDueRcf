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

#pragma once

#ifndef SRC_RTCDUERCFVALIDATION_H_
#define SRC_RTCDUERCFVALIDATION_H_

/**
 * A class that provides information about validity of alarm entries.
 */
class RtcDueRcf_Validation {
  RtcDueRcf_Validation(int rtcValidEntryRegister);
public:
  bool isCalendarAlarmValid() const;
  bool isTimeAlarmValid() const;
  /**
   * @ return true, if calendar alarm and time alarm is valid
   */
  bool isValid() const;
private:
  friend class RtcDueRcf;
  int mRtcValidEntryRegister;
};
#endif /* SRC_RTCDUERCFVALIDATION_H_ */
