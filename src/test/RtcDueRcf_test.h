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

#ifndef RTCDUERCF_SRC_TEST_RTCDUERCF_TEST_H_
#define RTCDUERCF_SRC_TEST_RTCDUERCF_TEST_H_

class Stream;

namespace RtcDueRcf_test {
  void runOfflineTests(Stream& log); // Run all tests
  void runOnlineTests(Stream& log); // Run all tests
  void loop(Stream& log);
}

#endif /* RTCDUERCF_SRC_TEST_RTCDUERCF_TEST_H_ */
