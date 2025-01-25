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
 * The returned time is always in 24 hrs mode independent of whether the RTC is running
 * in 12 hrs or 24 hrs mode.
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
extern void RTC_GetTimeAndDate( Rtc* pRtc, uint8_t* pucAMPM, uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond,
    uint16_t *pwYear, uint8_t *pucMonth, uint8_t *pucDay, uint8_t *pucWeek );

/**
 * \brief Sets the current time and date in the RTC.
 * Month, day and week values must be numbered starting from 1.
 *
 * \note In successive update operations, the user must wait at least one second
 * after resetting the UPDTIM/UPDCAL bit in the RTC_CR before setting these
 * bits again. Please look at the RTC section of the data sheet for detail.
 *
 * \param ucHour    Current hour in 12 or 24 hour mode.
 * \param ucMinute  Current minute.
 * \param ucSecond  Current second.
 * \param wYear     Current year.
 * \param ucMonth   Current month.
 * \param ucDay     Current day.
 * \param ucWeek    Day number in current week.
 *
 * \return 0 sucess, 1 fail to set
 */
extern int RTC_SetTimeAndDate( Rtc* pRtc, uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond,
    uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek );

extern int RTC_GetTimeAlarm( Rtc* pRtc, uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond );
extern int RTC_GetDateAlarm( Rtc* pRtc, uint8_t *pucMonth, uint8_t *pucDay );

#ifdef __cplusplus
}
#endif

#endif /* RTCSAM3XA_SRC_INTERNAL_CORE_SAM_GAPCLOSE_H_ */
