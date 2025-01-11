/*
 * core-sam-GapClose.c
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "chip.h"
#include <stdint.h>

#include "core-sam-GapClose.h"

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/

#define RTC_HOUR_BIT_LEN_MASK   0x3F
#define RTC_MIN_BIT_LEN_MASK    0x7F
#define RTC_SEC_BIT_LEN_MASK    0x7F
#define RTC_CENT_BIT_LEN_MASK   0x7F
#define RTC_YEAR_BIT_LEN_MASK   0xFF
#define RTC_MONTH_BIT_LEN_MASK  0x1F
#define RTC_DATE_BIT_LEN_MASK   0x3F
#define RTC_WEEK_BIT_LEN_MASK   0x07

/*----------------------------------------------------------------------------
 *        Internal functions
 *----------------------------------------------------------------------------*/

/**
 * \brief calculate the RTC_TIMR bcd format.
 *
 * \param ucHour    Current hour in 12 or 24 hour mode.
 * \param ucMinute  Current minute.
 * \param ucSecond  Current second.
 *
 * \return time in 32 bit RTC_TIMR bcd format on success, 0xFFFFFFFF on fail
 */
static uint32_t calculate_dwTime( Rtc* pRtc, uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond) {
    uint32_t dwAmpm=0 ;
    uint8_t ucHour_bcd ;
    uint8_t ucMin_bcd ;
    uint8_t ucSec_bcd ;

    /* if 12-hour mode, set AMPM bit */
    if ( (pRtc->RTC_MR & RTC_MR_HRMOD) == RTC_MR_HRMOD )
    {
        if ( ucHour > 12 )
        {
            ucHour -= 12 ;
            dwAmpm |= RTC_TIMR_AMPM ;
        }
    }
    ucHour_bcd = (ucHour%10)   | ((ucHour/10)<<4) ;
    ucMin_bcd  = (ucMinute%10) | ((ucMinute/10)<<4) ;
    ucSec_bcd  = (ucSecond%10) | ((ucSecond/10)<<4) ;

    /* value overflow */
    if ( (ucHour_bcd & (uint8_t)(~RTC_HOUR_BIT_LEN_MASK)) |
         (ucMin_bcd & (uint8_t)(~RTC_MIN_BIT_LEN_MASK)) |
         (ucSec_bcd & (uint8_t)(~RTC_SEC_BIT_LEN_MASK)))
    {
        return 0xFFFFFFFF ;
    }
    return dwAmpm | ucSec_bcd | (ucMin_bcd << 8) | (ucHour_bcd<<16) ;
}

/**
 * \brief calculate the RTC_CALR bcd format.
 *
 * \param wYear  Current year.
 * \param ucMonth Current month.
 * \param ucDay   Current day.
 * \param ucWeek  Day number in current week.
 *
 * \return date in 32 bit RTC_CALR bcd format on success, 0xFFFFFFFF on fail
 */
static uint32_t calculate_dwDate( Rtc* pRtc, uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek ) {
    uint8_t ucCent_bcd ;
    uint8_t ucYear_bcd ;
    uint8_t ucMonth_bcd ;
    uint8_t ucDay_bcd ;
    uint8_t ucWeek_bcd ;

    ucCent_bcd  = ((wYear/100)%10) | ((wYear/1000)<<4);
    ucYear_bcd  = (wYear%10) | (((wYear/10)%10)<<4);
    ucMonth_bcd = ((ucMonth%10) | (ucMonth/10)<<4);
    ucDay_bcd   = ((ucDay%10) | (ucDay/10)<<4);
    ucWeek_bcd  = ((ucWeek%10) | (ucWeek/10)<<4);

    /* value over flow */
    if ( (ucCent_bcd & (uint8_t)(~RTC_CENT_BIT_LEN_MASK)) |
         (ucYear_bcd & (uint8_t)(~RTC_YEAR_BIT_LEN_MASK)) |
         (ucMonth_bcd & (uint8_t)(~RTC_MONTH_BIT_LEN_MASK)) |
         (ucWeek_bcd & (uint8_t)(~RTC_WEEK_BIT_LEN_MASK)) |
         (ucDay_bcd & (uint8_t)(~RTC_DATE_BIT_LEN_MASK))
       )
    {
        return 0xFFFFFFFF ;
    }

    /* return date register value */
    return  (uint32_t)ucCent_bcd |
            (ucYear_bcd << 8) |
            (ucMonth_bcd << 16) |
            (ucWeek_bcd << 21) |
            (ucDay_bcd << 24);
}

/**
 * \brief Convert the RTC_TIMR bcd format to hour, minute and second.
 *
 * \param pucHour    If not null, current hour is stored in this variable.
 * \param pucMinute  If not null, current minute is stored in this variable.
 * \param pucSecond  If not null, current second is stored in this variable.
 */
static void dwTime2time( uint32_t dwTime, uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond )
{
    /* Hour */
    if ( pucHour )
    {
        *pucHour = ((dwTime & 0x00300000) >> 20) * 10
                 + ((dwTime & 0x000F0000) >> 16);

        if ( (dwTime & RTC_TIMR_AMPM) == RTC_TIMR_AMPM )
        {
            *pucHour += 12 ;
        }
    }

    /* Minute */
    if ( pucMinute )
    {
        *pucMinute = ((dwTime & 0x00007000) >> 12) * 10
                   + ((dwTime & 0x00000F00) >> 8);
    }

    /* Second */
    if ( pucSecond )
    {
        *pucSecond = ((dwTime & 0x00000070) >> 4) * 10
                   + (dwTime & 0x0000000F);
    }
}

/**
 * \brief Convert the RTC_CALR bcd format to year, month, day, week.
 *
 * \param pYwear  Current year (optional).
 * \param pucMonth  Current month (optional).
 * \param pucDay  Current day (optional).
 * \param pucWeek  Current day in current week (optional).
 */
extern void dwDate2date( uint32_t dwDate, uint16_t *pwYear, uint8_t *pucMonth, uint8_t *pucDay, uint8_t *pucWeek )
{
    /* Retrieve year */
    if ( pwYear )
    {
        *pwYear = (((dwDate  >> 4) & 0x7) * 1000)
                 + ((dwDate & 0xF) * 100)
                 + (((dwDate >> 12) & 0xF) * 10)
                 + ((dwDate >> 8) & 0xF);
    }

    /* Retrieve month */
    if ( pucMonth )
    {
        *pucMonth = (((dwDate >> 20) & 1) * 10) + ((dwDate >> 16) & 0xF);
    }

    /* Retrieve day */
    if ( pucDay )
    {
        *pucDay = (((dwDate >> 28) & 0x3) * 10) + ((dwDate >> 24) & 0xF);
    }

    /* Retrieve week */
    if ( pucWeek )
    {
        *pucWeek = ((dwDate >> 21) & 0x7);
    }
}

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Retrieves the current time and current date as stored in the RTC in several variables.
 * Month, day and week values are numbered starting at 1.
 *
 * \param pucHour    If not null, current hour is stored in this variable.
 * \param pucMinute  If not null, current minute is stored in this variable.
 * \param pucSecond  If not null, current second is stored in this variable.
 * \param pwYwear    If not null, current year is stored in this variable.
 * \param pucMonth   If not null, current month is stored in this variable.
 * \param pucDay     If not null, current day is stored in this variable.
 * \param pucWeek    If not null, current week is stored in this variable.
 */
extern void RTCgapclose_GetTimeAndDate( Rtc* pRtc,
    uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond,
    uint16_t *pwYear, uint8_t *pucMonth, uint8_t *pucDay, uint8_t *pucWeek )
{
    uint32_t dwTime;
    uint32_t dwDate;
    // Re-read time and date as long as we get unstable time.
    do
    {
      dwTime = pRtc->RTC_TIMR;
      do
      {
        dwDate = pRtc->RTC_CALR;
      }
      while ( dwDate != pRtc->RTC_CALR );
    }
    while( dwTime != pRtc->RTC_TIMR );

    dwTime2time( dwTime, pucHour, pucMinute, pucSecond ) ;
    dwDate2date( dwDate, pwYear, pucMonth, pucDay, pucWeek ) ;
}

/**
 * \brief Sets the current time and date in the RTC.
 * Month, day and week values must be numbered starting from 1.
 *
 * \note In successive update operations, the user must wait at least one second
 * after resetting the UPDTIM/UPDCAL bit in the RTC_CR before setting these
 * bits again. Please look at the RTC section of the datasheet for detail.
 *
 * \param ucHour    Current hour in 12 or 24 hour mode.
 * \param ucMinute  Current minute.
 * \param ucSecond  Current second.
 * \param wYear  Current year.
 * \param ucMonth Current month.
 * \param ucDay   Current day.
 * \param ucWeek  Day number in current week.
 *
 * \return 0 sucess, 1 fail to set
 */
extern int RTCgapclose_SetTimeAndDate( Rtc* pRtc,
    uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond,
    uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek )
{
  const uint32_t dwTime = calculate_dwTime(pRtc, ucHour, ucMinute, ucSecond) ;
  if ( dwTime == 0xFFFFFFFF )
  {
      return 1 ;
  }

  const uint32_t dwDate = calculate_dwDate(pRtc, wYear, ucMonth, ucDay, ucWeek);
  /* value over flow */
  if ( dwDate == 0xFFFFFFFF)
  {
      return 1 ;
  }

  /* Update calendar and time register together */
  pRtc->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  while ((pRtc->RTC_SR & RTC_SR_ACKUPD) != RTC_SR_ACKUPD) ;

  pRtc->RTC_SCCR = RTC_SCCR_ACKCLR;
  pRtc->RTC_TIMR = dwTime ;
  pRtc->RTC_CALR = dwDate ;
  pRtc->RTC_CR &= ~((uint32_t)RTC_CR_UPDTIM | (uint32_t)RTC_CR_UPDCAL) ;
  pRtc->RTC_SCCR |= RTC_SCCR_SECCLR; /* clear SECENV in SCCR */

  return (int)(pRtc->RTC_VER & RTC_VER_NVCAL) ;
}

extern int RTCgapclose_GetTimeAlarm( Rtc* pRtc, uint8_t *pucHour, uint8_t *pucMinute, uint8_t *pucSecond )
{
  const uint32_t dwAlarm = pRtc->RTC_TIMALR ;

  /* Hour */
  if ( pucHour )
  {
      if( dwAlarm & RTC_TIMALR_HOUREN )
      {
        *pucHour = ((dwAlarm & 0x00030000) >> 20) * 10 + ((dwAlarm & 0x000F0000) >> 16);
      }
      else
      {
        *pucHour = UINT8_MAX;
      }
  }

  /* Minute */
  if ( pucMinute )
  {
      if( dwAlarm & RTC_TIMALR_MINEN )
      {
          *pucMinute = ((dwAlarm & 0x00007000) >> 12) * 10 + ((dwAlarm & 0x00000F00) >> 8);
      }
      else
      {
        *pucMinute = UINT8_MAX;
      }
  }

  /* Second */
  if ( pucSecond )
  {
      if ( dwAlarm & RTC_TIMALR_SECEN )
      {
          *pucSecond = ((dwAlarm & 0x00000070) >> 4) * 10 + (dwAlarm & 0x0000000F);
      }
      else
      {
        *pucSecond = UINT8_MAX;
      }
  }

  return (int)(pRtc->RTC_VER & RTC_VER_NVTIMALR) ;
}

extern int RTCgapclose_GetDateAlarm( Rtc* pRtc, uint8_t *pucMonth, uint8_t *pucDay )
{
  const uint32_t dwAlarm = pRtc->RTC_CALALR;

  /* Compute alarm field value */
  if ( pucMonth )
  {
      if(dwAlarm & RTC_CALALR_MTHEN)
      {
        *pucMonth = ((dwAlarm & 0x00100000) >> 20) * 10 + ((dwAlarm & 0x000F0000) >> 16);
      }
      else
      {
        *pucMonth = UINT8_MAX;
      }
  }

  if ( pucDay )
  {
      if( dwAlarm & RTC_CALALR_DATEEN )
      {
        *pucDay = ((dwAlarm & 0x30000000) >> 28) * 10 + ((dwAlarm & 0x0F000000) >> 24);
      }
      else
      {
        *pucDay = UINT8_MAX;
      }
  }

  return (int)(pRtc->RTC_VER & RTC_VER_NVCALALR) ;
}

