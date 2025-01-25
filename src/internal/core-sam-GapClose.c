/*
 * core-sam-GapClose.c
 *
 *  Created on: 07.01.2025
 *      Author: Wolfgang
 */

#include "core-sam-GapClose.h"

#include "chip.h"
#include <stdint.h>


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
 * \param ucHour      Current hour in 24-hrs mode [0..23].
 * \param ucMinute    Current minute.
 * \param ucSecond    Current second.
 * \param n12hrsepresentationn If nonzero, the returned value will be for RTC_TIMR 12-hrs mode.
 *                    Otherwise, the returned value will be for RTC_TIMR 24-hrs mode.
 *
 * \return time in 32 bit RTC_TIMR bcd format on success, 0xFFFFFFFF on fail
 */
static uint32_t time2dwTime( Rtc* const pRtc, uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond, int n12HrsRepresentationn) {
    uint32_t dwAmPm = 0 ;

    /* if 12-hrs mode, set AMPM bit */
    if ( n12HrsRepresentationn )
    {
      // RTC is running in 12-hrs mode
      if ( ucHour >= 12 )
      {
        // PM Time
        dwAmPm |= RTC_TIMR_AMPM ;
        if ( ucHour > 12 )
        {
          ucHour -= 12 ; // convert PM time to 12-hrs mode
        }
      }
      else
      {
        // AM Time
        if( ucHour == 0 ) // midnight ?
        {
          ucHour = 12; // midnight is 12:00h in 12-hrs mode
        }
      }
    }
    const uint8_t ucHour_bcd = (ucHour%10)   | ((ucHour/10)<<4) ;
    const uint8_t ucMin_bcd  = (ucMinute%10) | ((ucMinute/10)<<4) ;
    const uint8_t ucSec_bcd  = (ucSecond%10) | ((ucSecond/10)<<4) ;

    /* value overflow */
    if ( (ucHour_bcd & (uint8_t)(~RTC_HOUR_BIT_LEN_MASK)) |
         (ucMin_bcd & (uint8_t)(~RTC_MIN_BIT_LEN_MASK)) |
         (ucSec_bcd & (uint8_t)(~RTC_SEC_BIT_LEN_MASK)))
    {
        return 0xFFFFFFFF ;
    }
    return dwAmPm | ucSec_bcd | (ucMin_bcd << 8) | (ucHour_bcd<<16) ;
}

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
static uint32_t date2dwDate( Rtc* const pRtc, uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek ) {
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
 * \brief Convert the RTC_TIMR bcd format to hour
 *
 * \param dwTime     The contents of the RTC_TIMR register.
 * \param pucAMPM    If not null, the variable will be set to 1 if time is PM. The variable will
 *                   be set to 0 if time is AM.
 * \param pucHour    If not null, current hour is stored in this variable. The hour representation
 *                   is as follows:
 *                     In case pucAMPM is not null, the hour will be in the interval of [1 .. 12].
 *                     In case pucAMPM is null, the hour will be in the interval of [0 .. 23].
 */
void dwTime2Hour( Rtc* const pRtc, uint32_t dwTime, uint8_t* const pucAMPM, uint8_t* const pucHour )
{
    *pucHour = ((dwTime & 0x00300000) >> 20) * 10 + ((dwTime & 0x000F0000) >> 16);
    const int n12HourMode = (pRtc->RTC_MR & RTC_MR_HRMOD) == RTC_MR_HRMOD;
    if ( n12HourMode )
    {
        // RTC is running in 12-hrs mode
        const uint8_t pm = (dwTime & RTC_TIMR_AMPM) == RTC_TIMR_AMPM;
        if ( pucAMPM )
        {
            // Keep 12-hrs representation
            *pucAMPM = pm;
        }
        else
        {
            // Convert to 24-hrs representation.
            if ( pm )
            {
                // PM Time
                if ( *pucHour < 12 )
                {
                    *pucHour += 12; // convert PM time to 24-hrs representation.
                }
            }
            else
            {
                // AM Time
                if ( *pucHour == 12 ) // midnight ?
                {
                    *pucHour = 0; // midnight is 0:00h in 24-hrs representation.
                }
            }
        }
    }
    else if ( pucAMPM )
    {
        // RTC is running in 24-hrs mode
        *pucAMPM = *pucHour > 11;

        if( *pucHour > 12 )
        {
          *pucHour -= 12; // convert PM to 12-hrs representation.
        }
        else if( *pucHour == 0 )
        {
          *pucHour = 12; // midnight is 12:00 in 12-hrs representation.
        }
    }
}

/**
 * \brief Convert the RTC_TIMR bcd format to hour, minute and second
 *
 * \param dwTime     The contents of the RTC_TIMR register.
 * \param pucAMPM    If not null, the variable will be set to 1 if time is PM. The variable will
 *                   be set to 0 if time is AM.
 * \param pucHour    If not null, current hour is stored in this variable. The hour representation
 *                   is as follows:
 *                     In case pucAMPM is not null, the hour will be in the interval of [1 .. 12].
 *                     In case pucAMPM is null, the hour will be in the interval of [0 .. 23].
 * \param pucMinute  If not null, current minute is stored in this variable.
 * \param pucSecond  If not null, current second is stored in this variable.
 */
static void dwTime2time(Rtc* const pRtc,  uint32_t dwTime, uint8_t* const pucAMPM, uint8_t* const pucHour,
    uint8_t* const pucMinute, uint8_t* const pucSecond )
{
    /* Hour */
    if ( pucHour )
    {
      dwTime2Hour(pRtc, dwTime, pucAMPM, pucHour);
    }
    else if( pucAMPM )
    {
      uint8_t hour;
      dwTime2Hour(pRtc, dwTime, pucAMPM, &hour);
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
 * \param dwDate    Contents of the RTC_CALR register
 * \param pYwear    Current year (optional).
 * \param pucMonth  Current month (optional).
 * \param pucDay    Current day (optional).
 * \param pucWeek   Current day in current week (optional).
 */
extern void dwDate2date( uint32_t dwDate, uint16_t* const pwYear, uint8_t* const pucMonth,
    uint8_t* const pucDay, uint8_t* const pucWeek )
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

extern void RTC_GetTimeAndDate( Rtc* const pRtc, uint8_t* const pucAMPM,
    uint8_t* const pucHour, uint8_t* const pucMinute, uint8_t* const pucSecond,
    uint16_t* const pwYear, uint8_t* const pucMonth, uint8_t* const pucDay, uint8_t* const pucWeek )
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

    dwTime2time( pRtc, dwTime, pucAMPM, pucHour, pucMinute, pucSecond ) ;
    dwDate2date( dwDate, pwYear, pucMonth, pucDay, pucWeek ) ;
}

extern int RTC_SetTimeAndDate( Rtc* const pRtc,
    uint8_t ucHour, uint8_t ucMinute, uint8_t ucSecond,
    uint16_t wYear, uint8_t ucMonth, uint8_t ucDay, uint8_t ucWeek )
{
  const int isRTCin12HourMode = (pRtc->RTC_MR & RTC_MR_HRMOD) == RTC_MR_HRMOD ;
  const uint32_t dwTime = time2dwTime(pRtc, ucHour, ucMinute, ucSecond, isRTCin12HourMode) ;
  if ( dwTime == 0xFFFFFFFF )
  {
      return 1 ;
  }

  const uint32_t dwDate = date2dwDate(pRtc, wYear, ucMonth, ucDay, ucWeek);
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

  return (pRtc->RTC_VER & RTC_VER_NVCAL) ;
}

extern int RTC_GetTimeAlarm( Rtc* const pRtc, uint8_t* const pucHour, uint8_t* const pucMinute, uint8_t* const pucSecond )
{
  const uint32_t dwAlarm = pRtc->RTC_TIMALR ;

  // Get the alarm time in 24-hrs mode.
  dwTime2time(pRtc, dwAlarm, 0, pucHour, pucMinute, pucSecond);

  /* Hour */
  if ( pucHour  && !(dwAlarm & RTC_TIMALR_HOUREN) )
  {
      *pucHour = UINT8_MAX;
  }

  /* Minute */
  if ( pucMinute && !(dwAlarm & RTC_TIMALR_MINEN) )
  {
      *pucMinute = UINT8_MAX;
  }

  /* Second */
  if ( pucSecond && !(dwAlarm & RTC_TIMALR_SECEN) )
  {
      *pucSecond = UINT8_MAX;
  }

  return (pRtc->RTC_VER & RTC_VER_NVTIMALR) ;
}

extern int RTC_GetDateAlarm( Rtc* const pRtc, uint8_t* const pucMonth, uint8_t* const pucDay )
{
  const uint32_t dwAlarm = pRtc->RTC_CALALR;

  // Get the alarm date.
  dwDate2date(dwAlarm, 0, pucMonth, pucDay, 0);

  /* Month */
  if ( pucMonth && !(dwAlarm & RTC_CALALR_MTHEN) )
  {
      *pucMonth = UINT8_MAX;
  }

  /* Day */
  if ( pucDay && !(dwAlarm & RTC_CALALR_DATEEN) )
  {
      *pucDay = UINT8_MAX;
  }

  return (pRtc->RTC_VER & RTC_VER_NVCALALR) ;
}

