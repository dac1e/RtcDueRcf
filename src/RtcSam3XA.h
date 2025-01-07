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

class RtcSam3XA {
  // Global interrupt handler forwards to RtcSam3XA_Handler()
  friend void ::RTC_Handler();
  volatile bool mSetTimeRequest;
  Sam3XA_RtcTime mSetTimeCache;

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

  enum RTC_OSCILLATOR {RC = 0, XTAL = 1};

  void begin(RTC_OSCILLATOR source = RTC_OSCILLATOR::XTAL);
  void set_time(const std::tm &t);
  void get_time(tm &td);

  static RtcSam3XA rtClock;
};

#endif /* RTCSAM3XA_SRC_RTCSAM3XA_H_ */
