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

/**
 * Demonstrate RTC alarm in combination with daylight savings transition.
 *
 * Set the local time just before daylight savings period starts and
 * set tje alarm at the hour when the daylight saving period starts:
 *  3:**:00h
 * Upon alarm, set the local time just before daylight saving period ends,
 * and set alarm to the hour, when the daylight saving period ends:
 *  2:**:00
 */

static void setTimeJustBeforeDstEntry() {
  // Set time to 10 seconds before daylight savings starts:
  // 27th of March 2016 01:59:50h.
  Serial.println("**** Set local time to 27th of March 2016 01:59:50h ****");
  TM time(50, 59, 1, 27, 2, TM::make_tm_year(2016), -1);
  RtcDueRcf::clock.setTime(time);
}

static void setTimeJustBeforeDstExit() {
  // Set time to 70 seconds before daylight savings ends:
  // 30th of October 2016 2:58:50h.
  Serial.println("**** Set local time to 30th of October 2016 2:58:50h ****");
  TM time(50, 58, 2, 30, 9, TM::make_tm_year(2016), 1);
  RtcDueRcf::clock.setTime(time);
}

static void setAlarm(Stream& log, uint8_t hour, uint8_t second = 0) {
  RtcDueRcf_Alarm alarm;
  alarm.setHour(hour);
  alarm.setSecond(second);
  RtcDueRcf::clock.setAlarm(alarm);
  log.print("Set alarm to ");
  log.print(hour);
  if(second < 10) {
    log.print(":**:0");
  } else {
    log.print(":**:");
  }
  log.println(second);
}

class AlarmReceiver {
  volatile bool mAlarm = false;
  int mAlarmCounter = 0;
  void onAlarm() {
    // This function runs within an interrupt context,
    // so we keep it short and just set a flag.
    mAlarm = true;
    mAlarmCounter++;
  }
public:
  void checkAlarm() {
    if(mAlarm) {
      mAlarm = false;

      // Print Alarm
      Serial.print("Alarm ");
      Serial.println(mAlarmCounter);

      // Toggle between Alarm on dst entry and alarm on dst exit
      if((mAlarmCounter % 2) == 0) {
        // Set time just before dst entry 1:59::50.
        setTimeJustBeforeDstEntry();
        // Set alarm to time 3:**:00. Next expected alarm is at 3:00:00h
        setAlarm(Serial, 3);


      } else {
        // Set time just before dst exit 2:58:50.
        setTimeJustBeforeDstExit();
        // Set alarm to time 2:**:00. Next expected alarm is at 2:59:00h
        setAlarm(Serial, 2);
      }
    }
  }

  static void alarmHandler(void* param) {
    // param is pointer to alarmReceiver.
    AlarmReceiver * const pAlarmReceiver = static_cast<AlarmReceiver*>(param);
    pAlarmReceiver->onAlarm();
  }
};

AlarmReceiver alarmReceiver;

//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);

  // Set time zone to Central European Time.
  RtcDueRcf::clock.begin(TZ::CET);

  RtcDueRcf::clock.setAlarmCallback(alarmReceiver.alarmHandler,
      &alarmReceiver /* callback parameter is pointer to the alarmReceiver. */);

  // Set time just before dst entry 1:59::50.
  setTimeJustBeforeDstEntry();

  // Set alarm to time 3:**:00.
  setAlarm(Serial, 3);
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

  alarmReceiver.checkAlarm();
  delay(1000);
}
