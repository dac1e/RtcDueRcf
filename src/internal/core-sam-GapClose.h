/*
  RtcDueRcf - Arduino libary for Arduino Due - builtin RTC Copyright (c)
  2024 Wolfgang Schmieder.  All right reserved.

  Contributors:
  - Wolfgang Schmieder

  Project home: https://github.com/dac1e/RtcDueRcf

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

#pragma once

#ifndef RTCDUERCF_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_
#define RTCDUERCF_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_

#include <include/rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Retrieves the current time and current date as stored in the RTC.
 * Month, day and week values are numbered starting at 1.
 *
 * \param pucAMPM    If not null, the variable will be set to 1 if time is PM. The variable will
 *                   be set to 0 if time is AM.
 * \param pucHour    If not null, current hour is stored in this variable. The hour representation
 *                   is as follows (independent of the mode the RTC is running in):
 *                     In case pucAMPM is not null, the hour will be in the interval of [1 .. 12].
 *                     In case pucAMPM is null, the hour will be in the interval of [0 .. 23].
 * \param pucMinute  If not null, current minute is stored in this variable.
 * \param pucSecond  If not null, current second is stored in this variable.
 * \param pwYwear    If not null, current year is stored in this variable.
 * \param pucMonth   If not null, current month is stored in this variable.
 * \param pucDay     If not null, current day is stored in this variable.
 * \param pucWeek    If not null, current week is stored in this variable.
 * \param pucRtc12HrsMode
 *                   If not null, hour mode that the RTC is running in, is stored in this variable.
 *                       0: RTC runs in 24-hrs mode.
 *                       1: RTC runs in 12-hrs mode.
 *
 *
 * \return Contents of RTC Valid Entry Register in bit[0..3].
 */
extern unsigned RTC_GetTimeAndDate( Rtc* const pRtc, uint8_t* const pucAMPM,
    uint8_t* const pucHour, uint8_t* const pucMinute, uint8_t* const pucSecond,
    uint16_t* const pwYear, uint8_t* const pucMonth, uint8_t* const pucDay,
    uint8_t* const pucWeek, uint8_t* const pucRtc12HrsMode );

/**
 * \brief Sets the current time and date in the RTC.
 * Month, day and week values must be numbered starting from 1. The passed hour
 * must be in 24 hour representation, independent of whether the RTC is running
 * in 12 hour mode, or 24-hrs mode. An adjusted ucHour may used to always
 * to match the current RTC hour mode. The RTC hour mode will not be changed.
 *
 * \note In successive update operations, the user must wait at least one second
 * after resetting the UPDTIM/UPDCAL bit in the RTC_CR before setting these
 * bits again. Please look at the RTC section of the data sheet for detail.
 *
 * \param timeReg      The contents of the RTC_TIMR register.
 * \param calReg       The contents of the RTC_CALR register.
 * \param rtc12hrsMode The hour mode in which the RTC should run:
 *                   0: 24-hrs mode.
 *                   1: 12-hrs mode.
 *
 * \return Contents of RTC Valid Entry Register in bit[0..3].
 */
extern unsigned RTC_SetTimeAndDate(Rtc *const pRtc, const uint32_t timeReg, const uint32_t calReg,
    const uint32_t rtc12hrsMode);

/**
 * \brief Retrieves the alarm time as stored in the RTC.
 *
 * The returned alarm time is always in 24-hrs representation independent of whether
 * the RTC is running in 12-hrs or 24-hrs mode.
 *
 * \param pucHour    If not null, alarm hour is stored in this variable.
 *                      If hour alarm is not enabled this variable will be UINT8_MAX.
 * \param pucMinute  If not null, alarm minute is stored in this variable.
 *                      If minute alarm is not enabled this variable will be UINT8_MAX.
 * \param pucSecond  If not null, alarm second is stored in this variable.
 *                      If second alarm is not enabled this variable will be UINT8_MAX.
 *
 * \return Contents of RTC Valid Entry Register in bit[0..3]
 */
extern unsigned RTC_GetTimeAlarm( Rtc* const pRtc, uint8_t* const pucHour, uint8_t* const pucMinute,
    uint8_t* const pucSecond );

/**
 * \brief Retrieves the alarm date as stored in the RTC.
 * Month, day and week values are numbered starting at 1.
 *
 * \param pucMonth   If not null, the alarm month is stored in this variable.
 *                      If month alarm is not enabled this variable will be UINT8_MAX.
 * \param pucDay     If not null, alarm day is stored in this variable.
 *                      If day alarm is not enabled this variable will be UINT8_MAX.
 *
 * \return Contents of RTC Valid Entry Register in bit[0..3]
 */
extern unsigned RTC_GetDateAlarm( Rtc* const pRtc, uint8_t* const pucMonth, uint8_t* const pucDay );

/**
 * \brief Sets a time alarm and date on the RTC.
 * The match is performed only on the provided variables;
 * Passing a null-pointer disables the corresponding field match.
 * Setting all pointers to 0 disables the time alarm.
 *
 * \param ucHour    If not UINT8_MAX, the time alarm will hour-match this value.
 * \param ucMinute  If not UINT8_MAX, the time alarm will minute-match this value.
 * \param ucSecond  If not UINT8_MAX, the time alarm will second-match this value.
 *
 * \param ucMonth   If not UINT8_MAX, the RTC alarm will month-match this value.
 * \param ucDay     If not UINT8_MAX, the RTC alarm will day-match this value.
 *
 * \return Contents of RTC Valid Entry Register in bit[0..3]
 */
extern unsigned RTC_SetTimeAndDateAlarm( Rtc* const pRtc, uint8_t ucHour, uint8_t ucMinute,
    uint8_t ucSecond, uint8_t ucMonth, uint8_t ucDay) ;


/**
 * \brief calculate the RTC_TIMR bcd format.
 *
 * \param ucHour      Current hour in 24-hrs mode [0..23].
 * \param ucMinute    Current minute.
 * \param ucSecond    Current second.
 * \param timeReg12HrsMode
 *                    0: The returned value will be for RTC_TIMR 24-hrs mode.
 *                    1: The returned value will be for RTC_TIMR 12-hrs mode.
 *
 * \return time in 32 bit RTC_TIMR bcd format on success, 0xFFFFFFFF on fail
 */
#define RTC_INVALID_TIME_REG 0xFFFFFFFF
extern uint32_t RTC_TimeToTimeReg(uint8_t ucHour, const uint8_t ucMinute, const uint8_t ucSecond,
    const uint32_t timeReg12HrsMode);

/**
 * \brief Convert the RTC_TIMR bcd format to hour, minute and second
 *
 * \param timeReg    The contents of the RTC_TIMR register.
 * \param pucAMPM    If not null, the variable will be set to 1 if time is PM. The variable will
 *                   be set to 0 if time is AM.
 * \param pucHour    If not null, current hour is stored in this variable. The hour representation
 *                   is as follows:
 *                     In case pucAMPM is not null, the hour will be in the interval of [1 .. 12].
 *                     In case pucAMPM is null, the hour will be in the interval of [0 .. 23].
 * \param pucMinute  If not null, current minute is stored in this variable.
 * \param pucSecond  If not null, current second is stored in this variable.
 * \param timeReg12HrsMode
 *                   0: Contents of the timer register is 24-hrs mode.
 *                   1: Contents of the timer register is 12-hrs mode.
 */
extern void RTC_TimeRegToTime(uint32_t timeReg, uint8_t* const pucAMPM, uint8_t* const pucHour,
    uint8_t* const pucMinute, uint8_t* const pucSecond, const uint32_t timeReg12HrsMode );

/**
 * \brief calculate the RTC_CALR bcd format.
 *
 * \param wYear   Current year.
 * \param ucMonth Current month.
 * \param ucDay   Current day.
 * \param ucWeek  Day number in current week.
 *
 * \return date in 32 bit RTC_CALR bcd format on success, 0xFFFFFFFF on fail
 */
#define RTC_INVALID_CAL_REG 0xFFFFFFFF
extern uint32_t RTC_DateToCalReg(const uint16_t wYear, const uint8_t ucMonth, const uint8_t ucDay, const uint8_t ucWeek );


/**
 * \brief Convert the RTC_CALR bcd format to year, month, day, week.
 *
 * \param calReg    Contents of the RTC_CALR register
 * \param pYwear    Current year (optional).
 * \param pucMonth  Current month (optional).
 * \param pucDay    Current day (optional).
 * \param pucWeek   Current day in current week (optional).
 */
extern void RTC_CalRegToDate( uint32_t calReg, uint16_t* const pwYear, uint8_t* const pucMonth,
    uint8_t* const pucDay, uint8_t* const pucWeek );

#ifdef __cplusplus
}
#endif

#endif /* RTCDUERCF_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_ */
