/*
 * RtcSam3XA_test.cpp
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "../RtcSam3XA.h"
#include "../TM.h"
#include "RtcSam3XA_test.h"

namespace {

  void makeCETdstBeginTime( TM& time) {
    // 2nd of March 2016 2:00:00h is an daylight savings begin in CET time zone.
    time.set(0, 0, 2, 27, TM::make_tm_month(TM::March), TM::make_tm_year(2016), -1);
  }

  void makeCETdstEndTime( TM& time) {
    // 30th of October 2016 2:00:00h is an daylight savings end in CET time zone.
    time.set(0, 0, 2, 30, TM::make_tm_month(TM::October), TM::make_tm_year(2016), -1);
  }
} // anonymous namespace

namespace RtcSam3XA_test {

  void run() {
    RtcSam3XA::clock.begin(TZ::CET, RtcSam3XA::RTC_OSCILLATOR::XTAL);

    tm time;
    RtcSam3XA::clock.getLocalTime(time);
  }
}
