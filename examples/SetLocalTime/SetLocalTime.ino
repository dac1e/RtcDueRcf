/*
  RtcDueRcf - Arduino libary for RtcDueRcf - builtin RTC Copyright (c)
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
#include "RtcDueRcf.h"
#include "TM.h"

/*
 * Set time zone to CET (Central European Time). Set
 * the RTC to an arbitrary time and date of 31st of
 * January 2000 12:15:30h. Read local time and print
 * on Serial.
 */

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);

  // Set time zone to Central European Time.
  RtcDueRcf::clock.begin(TZ::CET);

  Serial.println("**** Set local time to 31st of January 2000 12:15:30h ****");
  TM time(30, 15, 12, 31, 0 /* 0 = Jan. */, TM::make_tm_year(2000), -1);
  RtcDueRcf::clock.setTime(time);
}

// The loop function is called in an endless loop
void loop()
{
  // Print out the local time every second
  TM time;
  {
    /**
     * Read the local time and print it.
     */
    RtcDueRcf::clock.getLocalTime(time);
    Serial.print("Local time: ");
    Serial.print(time);
    Serial.println(time.tm_isdst ? " Dayl. savg." : " Normal Time");
  }
  delay(1000);
}
