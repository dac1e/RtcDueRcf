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
    time.set(second, minute, hour, 27, TM::make_tm_month(TM::March),
        TM::make_tm_year(2016), -1 /* -1: unknown */);
  }

  // Provide an example instance.
  void makeCETdstEndTime(TM& time, int second, int minute, int hour, int dst = 1 /* 1: still in dst */) {
    // 30th of October 2016 2:00:00h is an daylight savings end in CET time zone.
    time.set(second, minute, hour, 30, TM::make_tm_month(TM::October),
        TM::make_tm_year(2016), dst);
  }

  void logtime(Stream& stream, const TM& time, std::time_t localtime) {
    stream.print(time);
    stream.print(", "); stream.print(localtime);
    stream.print(", "); stream.println(time.tm_isdst);
  }
} // anonymous namespace

static void dumpTzInfo(Stream& stream) {
  __tzinfo_type *tz = __gettzinfo ();
  stream.print("__tznorth:"); stream.println(tz->__tznorth);
  for(size_t i = 0; i < 2; i++) {
    __tzrule_struct* tzrule = &tz->__tzrule[i];
    stream.print(i);
    stream.print(", ch:"); stream.print(tzrule->ch);
    stream.print(", d:"); stream.print(tzrule->d);
    stream.print(", m:"); stream.print(tzrule->m);
    stream.print(", n:"); stream.print(tzrule->n);
    stream.print(", s:"); stream.print(tzrule->s);
    stream.print(", offset:"); stream.println(tzrule->offset);
  }
}

namespace RtcSam3XA_test {

void basicSetGet(Stream &log) {
  log.print("--- RtcSam3XA_test::"); log.println(__FUNCTION__);

  TM stime;
  std::mktime(&stime);

#if RTC_MEASURE_ACKUPD
  uint32_t timestamp_setClock = millis();
#endif
  std::time_t localTimeStart = RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);

  TM rtime;
  std::time_t localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100); // @100ms
  // Time hasn't immediately set. The RTC is set within RTC_SR_ACKUPD interrupt.
  // The latency is about 350msec.
  assert(localtime == localTimeStart);

  // After 1600 (100 + 1600) the time should be set and should be increased by one second.
  delay(1500);// @1600ms
  localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100); // @1700ms
  assert(localtime == localTimeStart + 1);

#if RTC_MEASURE_ACKUPD
  // Measure the set clock - latency an print.
  uint32_t setClockLatency = RtcSam3XA::clock.getTimestampACKUPD() - timestamp_setClock;
  log.print("--- set clock latency: ");
  log.print(setClockLatency);
  log.println("ms");
#endif

}

void dstEntry(Stream& log) {
  log.print("--- RtcSam3XA_test::"); log.println(__FUNCTION__);
  delay(100); // @100ms

  constexpr int HOUR_START = 1;

  // Set RTC to 3 seconds before daylight savings starts
  TM stime;
  makeCETdstBeginTime(stime, 57, 59, HOUR_START);

  std::mktime(&stime);
  std::time_t localtimeStart = RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);

  TM rtime;
  std::time_t localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100); // @100ms
  // Time hasn't immediately set. The RTC is set within RTC_SR_ACKUPD interrupt.
  // The latency is about 350msec.
  assert(rtime.tm_hour == HOUR_START);

  // After 1600 (100 + 1400) the time should be set and should be increased by one second.
  delay(1500); // @1600ms
  localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100);  // @1700ms
  assert(rtime.tm_hour == HOUR_START);

  delay(1900);  // @3600ms
  localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100); // @3700ms
  assert(rtime.tm_hour == HOUR_START + 2);
}

void dstExit(Stream& log) {
  log.print("--- RtcSam3XA_test::"); log.println(__FUNCTION__);
  delay(100); // @100ms

  constexpr int HOUR_START = 2;

  // Set RTC to 3 seconds before daylight savings starts
  TM stime;
  makeCETdstEndTime(stime, 57, 59, HOUR_START);

  std::mktime(&stime);
  std::time_t localtimeStart = RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);

  TM rtime;
  std::time_t localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100); // @100ms
  // Time hasn't immediately set. The RTC is set within RTC_SR_ACKUPD interrupt.
  // The latency is about 350msec.
  assert(rtime.tm_hour == HOUR_START);

  // After 1600 (100 + 1600) the time should be set and should be increased by one second.
  delay(1500); // @1500ms
  localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100);  // @1700ms
  assert(rtime.tm_hour == HOUR_START);

  delay(1900);  // @3600ms
  localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(100); // @3700ms
  assert(rtime.tm_hour == HOUR_START);
}

void checkRTCisdst(TM stime, Sam3XA_RtcTime rtc) {
  std::time_t localtime = mktime(&stime);
  localtime_r(&localtime, &stime);
  rtc.set(stime);
  bool isdst = rtc.isdst();
  assert(stime.tm_isdst == isdst);
}

void checkRTCisdst(Stream& log) {
  log.print("--- RtcSam3XA_test::"); log.println(__FUNCTION__);
  delay(100); // @100ms

  Sam3XA_RtcTime rtc;
  TM stime;

  makeCETdstBeginTime(stime, 59, 59, 1);
  checkRTCisdst(stime, rtc);

  makeCETdstBeginTime(stime, 00, 00, 2);
  checkRTCisdst(stime, rtc);

  makeCETdstEndTime(stime, 59, 59, 1);
  checkRTCisdst(stime, rtc);

  makeCETdstEndTime(stime, 00, 00, 2, 0);
  checkRTCisdst(stime, rtc);
}

void run(Stream& log) {
  log.print("--- RtcSam3XA_test::"); log.println(__FUNCTION__);
  dumpTzInfo(log);
  delay(200);

  RtcSam3XA::clock.begin(TZ::CET, RtcSam3XA::RTC_OSCILLATOR::XTAL);

  dumpTzInfo(log);
  delay(200);

  checkRTCisdst(log);
  basicSetGet(log);
  dstEntry(log);
  dstExit(log);

  TM stime;
  makeCETdstBeginTime(stime, 00, 59, 1);
  std::mktime(&stime);
  std::time_t localtimeStart = RtcSam3XA::clock.setByLocalTime(stime);
  log.println(stime);
}

void loop(Stream& log) {
  TM rtime;
  std::time_t localtime = RtcSam3XA::clock.getLocalTime(rtime);
  logtime(log, rtime, localtime);
  delay(999);
}

} // namespace RtcSam3XA_test
