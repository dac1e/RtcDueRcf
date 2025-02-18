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

#include <Arduino.h>
#include "RtcDueRcf.h"
#include "TM.h"

/**
 * Set time zone to CET (Central European Time).
 *
 * Set the RTC by 1.Jan.2000 0:00:00h UTC (Greenwich meantime).
 * Read local time frequently from RTC and print on Serial.
 * Note: local time CET will be one hour ahead of UTC.
 */

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);

  TM tm(0, 0, 0, 1, 0 /* 0 = Jan. */, TM::make_tm_year(2000), -1);

  // Create UTC timestamp for 1st of January 2000 00:00:00h
  RtcDueRcf::clock.tzset(TZ::UTC);
  const std::time_t utcTimeStamp = std::mktime(&tm);

  RtcDueRcf::clock.begin(TZ::CET);
  Serial.println("**** Set UTC time to 1st of January 2000 0:00:00h ****");

  // Set UTC time to 1st of January 2000 00:00:00h. This results
  // to 1st of January 2000 1:00:00h CET (Central Europe Time).
  RtcDueRcf::clock.setTime(utcTimeStamp);
}

// The loop function is called in an endless loop
void loop()
{
  // Print out the local time every second
  TM localTime;
  {
    /**
     * Read the local time and print it.
     * Then convert the local time to
     * UTC (Greenwich meantime) and
     * print it.
     */
    RtcDueRcf::clock.getLocalTime(localTime);
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
  delay(1000);
}
