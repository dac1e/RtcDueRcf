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
#include <assert.h>

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

void basicSetGet(Stream &log) {
  TM stime;
  std::mktime(&stime);

#if RTC_MEASURE_ACKUPD
  uint32_t timestamp_setClock = millis();
#endif
  RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);

  TM rtime;
  RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);

  // Time hasn't immediately set. The RTC is set within RTC_SR_ACKUPD interrupt.
  // The latency is about 350msec.
  assert(stime == rtime);

  // After 1500 the time should be set and should be increased by one second.
  delay(1500);
  RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  assert(stime + 1 == rtime);

#if RTC_MEASURE_ACKUPD
  // Measure the set clock - latency an print.
  uint32_t setClockLatency = RtcSam3XA::clock.getTimestampACKUPD() - timestamp_setClock;
  log.print("--- set clock latency: ");
  log.print(setClockLatency);
  log.println("ms");
#endif

}

void run(Stream& log) {
  log.print("RtcSam3XA_test::");
  log.println(__FUNCTION__);
  delay(100);

  RtcSam3XA::clock.begin(TZ::CET, RtcSam3XA::RTC_OSCILLATOR::XTAL);

basicSetGet(log);
  }
}
