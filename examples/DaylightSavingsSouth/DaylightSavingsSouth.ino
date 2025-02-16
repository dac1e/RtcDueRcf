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

#include <Arduino.h>
#include "RtcSam3XA.h"
#include "TM.h"

#include "test/RtcSam3XA_test.h"

/**
 * Demonstrate daylight savings transition on Serial monitor for time
 * zone NZST (New Zealand).
 *
 * 1) Set the local time just before daylight savings period starts.
 *
 * 2) After local time has switched to daylight savings period it stays
 * there for approx. 15 seconds.
 *
 * 3) Then set the local time just before daylight savings period ends.
 *
 * 4) After local time has switched to normal time period it stays
 * there for approximately 15 seconds.
 *
 * 5) Begin at step 1)
 */

static bool isDaylightSavings = false;
static int loopCountDown = -1;

static void setTimeJustBeforeDstEntry() {
  // Set time to 10 seconds before daylight savings starts:
  // 29th of September 2024 01:59:50h.
  Serial.println("**** Set local time to 29th of September 2024 01:59:50h ****");
  TM time(50, 59, 1, 29, 8, TM::make_tm_year(2024), -1);
  RtcSam3XA::clock.setTime(time);
}

static void setTimeJustBeforeDstExit() {
  // Set time to 10 seconds before daylight savings ends:
  // 6th of April 2025 2:59:50h.
  Serial.println("**** Set local time to 6th of April 2025 2:59:50h ****");
  TM time(50, 59, 2, 6, 3, TM::make_tm_year(2025), 1);
  RtcSam3XA::clock.setTime(time);
}

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);
  // Set time zone to New Zealand Time.
  RtcSam3XA::clock.begin(TZ::NZST);
  setTimeJustBeforeDstEntry();
  isDaylightSavings = false;
}

// The loop function is called in an endless loop
void loop()
{
  // Print out the local time every second
  TM localTime;
  {
    /**
     * Read the local time and print it. Then convert the local time to UTC
     * (Greenwich meantime) and print it.
     */
    RtcSam3XA::clock.getLocalTime(localTime);
    Serial.print("Local time: ");
    Serial.print(localTime);
    Serial.print(localTime.tm_isdst ? " Dayl. savg." : " Normal Time");

    const std::time_t utc = std::mktime(&localTime);
    TM utcTime;
    gmtime_r(&utc, &utcTime);
    Serial.print(", (UTC=");
    Serial.print(utcTime);
    Serial.println(')');
  }

  if(isDaylightSavings != localTime.tm_isdst) {
    isDaylightSavings = localTime.tm_isdst;
    loopCountDown = 15; // Set time again after 15 loops
  }

  if(loopCountDown == 0)
  {
    // 15 loops expired.
    if(isDaylightSavings) {
      // Set time again
      setTimeJustBeforeDstExit();
    } else {
      // Set time again
      setTimeJustBeforeDstEntry();
    }
  }

  if(loopCountDown >= 0) { // stop counter at -1
    --loopCountDown;
  }

  delay(1000);
}
