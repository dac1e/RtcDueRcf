/*
 * RtcSam3XA_test.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "../RtcSam3XA.h"
#include "../TM.h"
#include "RtcSam3XA_test.h"
#include "Arduino.h"

namespace {

  // Provide an example instance.
  void makeCETdstBeginTime( TM& time) {
    // 2nd of March 2016 2:00:00h is an daylight savings begin in CET time zone.
    time.set(0, 0, 2, 27, TM::make_tm_month(TM::March), TM::make_tm_year(2016), -1);
  }

  // Provide an example instance.
  void makeCETdstEndTime( TM& time) {
    // 30th of October 2016 2:00:00h is an daylight savings end in CET time zone.
    time.set(0, 0, 2, 30, TM::make_tm_month(TM::October), TM::make_tm_year(2016), -1);
  }
} // anonymous namespace

namespace RtcSam3XA_test {

  void run(Stream& log) {
    log.print("RtcSam3XA_test::");
    log.println(__FUNCTION__);
    delay(100);

    RtcSam3XA::clock.begin(TZ::CET, RtcSam3XA::RTC_OSCILLATOR::XTAL);

    TM stime;
    std::mktime(&stime);

#if RTC_MEASURE_ACKUPD
    uint32_t timestamp_setClock = millis();
#endif

    RtcSam3XA::clock.setByLocalTime(stime);
    log.println(stime);

    TM rtime1;
    RtcSam3XA::clock.getLocalTime(rtime1);
    log.println(rtime1);
    delay(1000);

#if RTC_MEASURE_ACKUPD
    uint32_t setClockLatency = RtcSam3XA::clock.getTimestampACKUPD() - timestamp_setClock;
    log.print("--- set clock latency: ");
    log.print(setClockLatency);
    log.println("ms");
#endif

    TM rtime2;
    RtcSam3XA::clock.getLocalTime(rtime2);
    log.println(rtime2);
  }
}
