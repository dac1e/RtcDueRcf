/*
 * RtcSam3XA.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#ifndef RTCSAM3XA_SRC_RTCSAM3XA_H_
#define RTCSAM3XA_SRC_RTCSAM3XA_H_

#include <ctime>
#include <include/rtc.h>
#include "Sam3XA_RtcTime.h"

class RtcSam3XA {
  // Global interrupt handler forwards to RtcSam3XA_Handler()
  friend void ::RTC_Handler();
  volatile bool mSetTimeRequest;
  Sam3XA_RtcTime mSetTimeCache;
  void(*mSecondCallback)(void*);
  void* mSecondCallbackPararm;
  void(*mAlarmCallback)(void*);
  void* mAlarmCallbackPararm;

  // Interrupt handler
  void RtcSam3XA_Handler();
  RtcSam3XA();

public:
  // Public child class tm extends struct std::tm with constructors and convenient functions.
  class tm : public std::tm {
    static int tmDayofWeek (uint16_t _year, int _month, int _day);
  public:
    tm();
    tm(int _day, int _m, int _year, int _hours, int _minutes, int _seconds, bool _sl);
    void set(Sam3XA_RtcTime& td);
  };

  class AlarmTime {
    friend class RtcSam3XA;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month; // Jan = 0
  public:
    AlarmTime() : second(UINT8_MAX), minute(UINT8_MAX), hour(UINT8_MAX), day(UINT8_MAX), month(UINT8_MAX) {}
    void setSecond(int _second) {second = _second;}
    void setMinute(int _minute) {minute = _minute;}
    void setHour(int _hour) {hour = _hour;}
    void setDay(int _day) {second = _day;}
    void setMonth(int _month) {second = _month;}
  };

  enum RTC_OSCILLATOR {RC = 0, XTAL = 1};

  void begin(RTC_OSCILLATOR source = RTC_OSCILLATOR::XTAL);
  void set_time(const std::tm &t);
  void get_time(tm &td);

  void setAlarmCallback(void (*alarmCallback)(void*), void *alarmCallbackParam = nullptr);
  void setAlarm(const AlarmTime& alarmTime);
  void clearAlarm(){setAlarm(AlarmTime());}

  void setSecondCallback(void (*secondCallback)(void*), void *secondCallbackParam = nullptr);

  static RtcSam3XA rtClock;
};
#endif /* RTCSAM3XA_SRC_RTCSAM3XA_H_ */
