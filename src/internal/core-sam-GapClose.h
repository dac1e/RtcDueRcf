/*
 * core-sam-GapClose.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#pragma once

#ifndef RTCSAM3XA_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_
#define RTCSAM3XA_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_

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
 */
extern void RTC_GetTimeAndDate( Rtc* const pRtc, uint8_t* const pucAMPM,
    uint8_t* const pucHour, uint8_t* const pucMinute, uint8_t* const pucSecond,
    uint16_t* const pwYear, uint8_t* const pucMonth, uint8_t* const pucDay,
    uint8_t* const pucWeek );

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
 * \param ucHour    Current hour in 24-hrs representation.
 * \param ucMinute  Current minute.
 * \param ucSecond  Current second.
 * \param wYear     Current year.
 * \param ucMonth   Current month.
 * \param ucDay     Current day.
 * \param ucWeek    Day number in current week.
 *
 * \return 0 sucess, 1 fail to set
 */
extern int RTC_SetTimeAndDate( Rtc* const pRtc, uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond,
    uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek );

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
 * \return 0 if time alarm is valid. Otherwise 1.
 */
extern int RTC_GetTimeAlarm( Rtc* const pRtc, uint8_t* const pucHour, uint8_t* const pucMinute,
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
 * \return 0 if date alarm is valid. Otherwise 1.
 */
extern int RTC_GetDateAlarm( Rtc* const pRtc, uint8_t* const pucMonth, uint8_t* const pucDay );

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
 * \return 0 success, 1 fail to set
 */
extern int RTC_SetTimeAndDateAlarm( Rtc* const pRtc, uint8_t ucHour, uint8_t ucMinute,
    uint8_t ucSecond, uint8_t ucMonth, uint8_t ucDay) ;

#ifdef __cplusplus
}
#endif

#endif /* RTCSAM3XA_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_ */
