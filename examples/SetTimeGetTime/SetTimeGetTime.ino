#include <Arduino.h>
#include "RtcSam3XA.h"
#include "TM.h"


//The setup function is called once at startup of the sketch
void setup()
{
  Serial.begin(9600);

  // Set time zone to Central European Time.
  RtcSam3XA::clock.begin(TZ::CET);

  // Set time to one and a half minute before daylight savings starts:
  // 27th of March 2016 01:58:30h
  TM time(30, 58, 1, 27, 2 /* march */, TM::make_tm_year(2016), -1);
  RtcSam3XA::clock.setLocalTime(time);
}

// The loop function is called in an endless loop
void loop()
{
  // Print out the local time every 3 seconds

  {
    /**
     * Read the local time and print it.
     */
    TM rtime;
    RtcSam3XA::clock.getLocalTime(rtime);
    Serial.print("Local time: ");
    Serial.println(rtime);
    delay(1000); // wait a second
  }

  {
    /**
     * Read the local time and print it. Then
     * convert the local time to UTC (Greenich
     * meantime) and print it.
     */
    TM rtime;
    const std::time_t rawtime = RtcSam3XA::clock.getLocalTime(rtime);
    Serial.print("Local time: ");
    Serial.println(rtime);

    TM utc;
    gmtime_r(&rawtime, &utc);
    Serial.print("UTC: ");
    Serial.println(utc);
  }
  delay(4000); // wait 4 seconds
}
