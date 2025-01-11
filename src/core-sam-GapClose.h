/*
 * core-sam-GapClose.h
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#ifndef RTCSAM3XA_SRC_CORE_SAM_GAPCLOSE_H_
#define RTCSAM3XA_SRC_CORE_SAM_GAPCLOSE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void RTCgapclose_GetTimeAndDate( Rtc* pRtc, uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond, uint16_t *pwYear, uint8_t *pucMonth, uint8_t *pucDay, uint8_t *pucWeek );
extern int RTCgapclose_SetTimeAndDate( Rtc* pRtc, uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond, uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek );

extern int RTCgapclose_GetTimeAlarm( Rtc* pRtc, uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond );
extern int RTCgapclose_GetDateAlarm( Rtc* pRtc, uint8_t *pucMonth, uint8_t *pucDay );

#ifdef __cplusplus
}
#endif

#endif /* RTCSAM3XA_SRC_CORE_SAM_GAPCLOSE_H_ */
