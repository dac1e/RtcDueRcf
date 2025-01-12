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
  void makeCETdstBeginTime(TM& time, int second, int minute, int hour) {
    // 2nd of March 2016 2:00:00h is an daylight savings begin in CET time zone.
    time.set(second, minute, hour, 27, TM::make_tm_month(TM::March), TM::make_tm_year(2016), -1);
  }

  // Provide an example instance.
  void makeCETdstEndTime(TM& time, int second, int minute, int hour) {
    // 30th of October 2016 2:00:00h is an daylight savings end in CET time zone.
    time.set(second, minute, hour, 30, TM::make_tm_month(TM::October), TM::make_tm_year(2016), -1);
  }
} // anonymous namespace

namespace RtcSam3XA_test {

void basicSetGet(Stream &log) {
  TM stime;
  std::mktime(&stime);

#if RTC_MEASURE_ACKUPD
  uint32_t timestamp_setClock = millis();
#endif
  std::time_t localTimeStart = RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);

  TM rtime;
  std::time_t localTime = RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  delay(100); // @100ms
  // Time hasn't immediately set. The RTC is set within RTC_SR_ACKUPD interrupt.
  // The latency is about 350msec.
  assert(localTime == localTimeStart);

  // After 1500 (100 + 1400) the time should be set and should be increased by one second.
  delay(1400);// @1500ms
  localTime = RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  delay(100); // @1600ms
  assert(localTime == localTimeStart + 1);

#if RTC_MEASURE_ACKUPD
  // Measure the set clock - latency an print.
  uint32_t setClockLatency = RtcSam3XA::clock.getTimestampACKUPD() - timestamp_setClock;
  log.print("--- set clock latency: ");
  log.print(setClockLatency);
  log.println("ms");
#endif

}

void dstEntry(Stream& log) {
  // Set RTC to 3 seconds before daylight savings starts
  TM stime;
  makeCETdstBeginTime(stime, 57, 59, 1);

  std::time_t localTimeStart = RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);

  TM rtime;
  std::time_t localTime = RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  delay(100); // @100ms
  // Time hasn't immediately set. The RTC is set within RTC_SR_ACKUPD interrupt.
  // The latency is about 350msec.
  assert(localTime == localTimeStart);

  // After 1500 (100 + 1400) the time should be set and should be increased by one second.
  delay(1400); // @1500ms
  localTime = RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  delay(100);  // @1600ms
  assert(localTime == localTimeStart + 1);

  delay(1900);  // @3500ms
  localTime = RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  delay(100); // @3600ms

  delay(900);  // @4500ms
  localTime = RtcSam3XA::clock.getLocalTime(rtime);
  log.println(rtime);
  delay(100); // @4600ms
}

void dstExit(Stream& log) {

}

void run(Stream& log) {
  log.print("RtcSam3XA_test::");
  log.println(__FUNCTION__);
  delay(100);

  RtcSam3XA::clock.begin(TZ::CET, RtcSam3XA::RTC_OSCILLATOR::XTAL);

  basicSetGet(log);
  dstEntry(log);
  dstExit(log);
}

} // namespace RtcSam3XA_test
