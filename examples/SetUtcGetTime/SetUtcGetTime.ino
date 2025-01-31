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

/**
 * Set time zone to CET (Central European Time).
 * Set the RTC to UTC 1.1.2000 0:00:00h. Read local time
 * and print on Serial.
 * Note CET local time will be one hour ahead
 * respectively 2 hours ahead, when daylight savings
 * period is entered.
 */

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);

  // Set time zone to Central European Time.
  RtcSam3XA::clock.begin(TZ::CET);

  // Note: The RTC does not accept any year lower than 2000.
  // Set time to 1st of January 2000 0:00:00h UTC which is
  // 1st of January 2000 1:00:00h CET (Central Europe Time).
  Serial.println("**** 1st of January 2000 0:00:00h UTC ****");
  const std::time_t timestamp = 946684800;
  RtcSam3XA::clock.setUTC(timestamp);
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
    const std::time_t utc = RtcSam3XA::clock.getLocalTimeAndUTC(localTime);
    Serial.print("Local time: ");
    Serial.print(localTime);
    Serial.print(localTime.tm_isdst ? " Dayl. savg." : " Normal Time");

    TM utcTime;
    gmtime_r(&utc, &utcTime);
    Serial.print(", (UTC=");
    Serial.print(utcTime);
    Serial.println(')');
  }
  delay(1000);
}
