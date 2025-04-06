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

#include <stdint.h>
#include "chip.h"
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
 * \brief Convert the RTC_TIMR bcd format to hour
 *
 * \param timeReg     The contents of the RTC_TIMR register.
 * \param pucAMPM    If not null, the variable will be set to 1 if time is PM. The variable will
 *                   be set to 0 if time is AM.
 * \param pucHour    If not null, current hour is stored in this variable. The hour representation
 *                   is as follows:
 *                     In case pucAMPM is not null, the hour will be in the interval of [1 .. 12].
 *                     In case pucAMPM is null, the hour will be in the interval of [0 .. 23].
 */
void RTC_TimeRegisterToHour(uint32_t timeReg, uint8_t* const pucAMPM, uint8_t* const pucHour, const uint32_t timeReg12HrsMode )
{
    *pucHour = ((timeReg & 0x00300000) >> 20) * 10 + ((timeReg & 0x000F0000) >> 16);

    if ( timeReg12HrsMode )
    {
        // timeReg is in 12-hrs mode
        const uint8_t pm = (timeReg & RTC_TIMR_AMPM) == RTC_TIMR_AMPM;
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

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

extern void RTC_TimeRegToTime(uint32_t timeReg, uint8_t* const pucAMPM, uint8_t* const pucHour,
    uint8_t* const pucMinute, uint8_t* const pucSecond, const uint32_t timeReg12HrsMode )
{
    /* Hour */
    if ( pucHour )
    {
      RTC_TimeRegisterToHour(timeReg, pucAMPM, pucHour, timeReg12HrsMode);
    }
    else if( pucAMPM )
    {
      uint8_t hour;
      RTC_TimeRegisterToHour(timeReg, pucAMPM, &hour, timeReg12HrsMode);
    }

    /* Minute */
    if ( pucMinute )
    {
        *pucMinute = ((timeReg & 0x00007000) >> 12) * 10
                   + ((timeReg & 0x00000F00) >> 8);
    }

    /* Second */
    if ( pucSecond )
    {
        *pucSecond = ((timeReg & 0x00000070) >> 4) * 10
                   + (timeReg & 0x0000000F);
    }
}

extern void RTC_CalRegToDate(uint32_t calReg, uint16_t* const pwYear, uint8_t* const pucMonth,
    uint8_t* const pucDay, uint8_t* const pucWeek )
{
    /* Retrieve year */
    if ( pwYear )
    {
        *pwYear = (((calReg  >> 4) & 0x7) * 1000)
                 + ((calReg & 0xF) * 100)
                 + (((calReg >> 12) & 0xF) * 10)
                 + ((calReg >> 8) & 0xF);
    }

    /* Retrieve month */
    if ( pucMonth )
    {
        *pucMonth = (((calReg >> 20) & 1) * 10) + ((calReg >> 16) & 0xF);
    }

    /* Retrieve day */
    if ( pucDay )
    {
        *pucDay = (((calReg >> 28) & 0x3) * 10) + ((calReg >> 24) & 0xF);
    }

    /* Retrieve week */
    if ( pucWeek )
    {
        *pucWeek = ((calReg >> 21) & 0x7);
    }
}

extern uint32_t RTC_TimeToTimeReg(uint8_t ucHour, const uint8_t ucMinute, const uint8_t ucSecond, const uint32_t timeReg12HrsMode)
{
    uint32_t dwAmPm = 0 ;

    /* if 12-hrs mode, set AMPM bit */
    if ( timeReg12HrsMode )
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
    const uint8_t ucHour_bcd =  (ucHour%10)  |  ((ucHour/10)<<4) ;
    const uint8_t ucMin_bcd  = (ucMinute%10) | ((ucMinute/10)<<4) ;
    const uint8_t ucSec_bcd  = (ucSecond%10) | ((ucSecond/10)<<4) ;

    /* value overflow */
    if ( (ucHour_bcd & (uint8_t)(~RTC_HOUR_BIT_LEN_MASK)) |
         (ucMin_bcd & (uint8_t)(~RTC_MIN_BIT_LEN_MASK)) |
         (ucSec_bcd & (uint8_t)(~RTC_SEC_BIT_LEN_MASK)))
    {
        return RTC_INVALID_TIME_REG ;
    }
    return dwAmPm | ucSec_bcd | (ucMin_bcd << 8) | (ucHour_bcd<<16) ;
}

extern uint32_t RTC_DateToCalReg(const uint16_t wYear, const uint8_t ucMonth, const uint8_t ucDay, const uint8_t ucWeek )
{
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
        return RTC_INVALID_CAL_REG ;
    }

    /* return date register value */
    return  (uint32_t)ucCent_bcd |
            (ucYear_bcd << 8) |
            (ucMonth_bcd << 16) |
            (ucWeek_bcd << 21) |
            (ucDay_bcd << 24);
}

extern unsigned RTC_GetTimeAndDate( Rtc* const pRtc, uint8_t* const pucAMPM,
    uint8_t* const pucHour, uint8_t* const pucMinute, uint8_t* const pucSecond,
    uint16_t* const pwYear, uint8_t* const pucMonth, uint8_t* const pucDay,
    uint8_t* const pucWeek, uint8_t* const pucRtc12HrsMode )
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

    RTC_TimeRegToTime(dwTime, pucAMPM, pucHour, pucMinute, pucSecond, pRtc->RTC_MR & RTC_MR_HRMOD );
    RTC_CalRegToDate( dwDate, pwYear, pucMonth, pucDay, pucWeek );

    if(pucRtc12HrsMode) {
      *pucRtc12HrsMode = pRtc->RTC_MR & RTC_MR_HRMOD;
    }

    return (pRtc->RTC_VER & (RTC_VER_NVCAL | RTC_VER_NVTIM | RTC_VER_NVCALALR | RTC_VER_NVTIMALR));
}

extern unsigned RTC_SetTimeAndDate(Rtc *const pRtc, const uint32_t timeReg, const uint32_t calReg,
    const uint32_t rtc12hrsMode)
{
  const uint32_t HOUR_MASK = RTC_TIMALR_HOUREN | RTC_TIMALR_AMPM | RTC_TIMALR_HOUR_Msk;
  const uint32_t current12HrsMode = (pRtc->RTC_MR & RTC_MR_HRMOD);
  const uint32_t dwTimAlarmHour = rtc12hrsMode != current12HrsMode ? (pRtc->RTC_TIMALR & HOUR_MASK) : 0;

  /* Update calendar and time register together */
  pRtc->RTC_CR |= (RTC_CR_UPDTIM | RTC_CR_UPDCAL);
  while ((pRtc->RTC_SR & RTC_SR_ACKUPD) != RTC_SR_ACKUPD);

  pRtc->RTC_SCCR = RTC_SCCR_ACKCLR;
  pRtc->RTC_TIMR = timeReg;
  pRtc->RTC_CALR = calReg;
  pRtc->RTC_CR &= ~((uint32_t) RTC_CR_UPDTIM | (uint32_t) RTC_CR_UPDCAL);
  pRtc->RTC_SCCR |= RTC_SCCR_SECCLR; /* clear SECENV in SCCR */
  pRtc->RTC_MR = rtc12hrsMode & RTC_MR_HRMOD;

  if(rtc12hrsMode != current12HrsMode) {
    if (dwTimAlarmHour & RTC_TIMALR_HOUREN) {
      uint32_t hour = RTC_TIMALR_HOUR(dwTimAlarmHour);
      uint32_t AMPM = 0;
      // Fix alarm hour
      if (rtc12hrsMode) {
        if (dwTimAlarmHour & RTC_TIMALR_AMPM) {
          // PM Time
          if (hour < 12) {
            hour += 12; // convert PM time to 24-hrs representation.
          }
        } else {
          // AM Time
          if (hour == 12) // midnight ?
              {
            hour = 0; // midnight is 0:00h in 24-hrs representation.
          }
        }
      } else {
        // RTC is running in 24-hrs mode
        AMPM = (hour > 11) ? RTC_TIMALR_AMPM : 0;
        if (hour > 12) {
          hour -= 12; // convert PM to 12-hrs representation.
        } else if (hour == 0) {
          hour = 12; // midnight is 12:00 in 12-hrs representation.
        }
      }
      pRtc->RTC_TIMALR = (pRtc->RTC_TIMALR & ~HOUR_MASK) | RTC_TIMALR_HOUREN | AMPM | (hour << RTC_TIMALR_HOUR_Pos);
    }
  }
  return (pRtc->RTC_VER & (RTC_VER_NVCAL | RTC_VER_NVTIM | RTC_VER_NVCALALR | RTC_VER_NVTIMALR));
}

extern unsigned RTC_SetTimeAndDateAlarm( Rtc* const pRtc, uint8_t ucHour,
    uint8_t ucMinute, uint8_t ucSecond, uint8_t ucMonth, uint8_t ucDay)
{
  RTC_DisableIt(RTC, RTC_IER_ALREN);

  {
    const uint8_t hour   = ucHour   != UINT8_MAX ? ucHour   : 0;
    const uint8_t minute = ucMinute != UINT8_MAX ? ucMinute : 0;
    const uint8_t second = ucSecond != UINT8_MAX ? ucSecond : 0;

    const int n12HourMode = (pRtc->RTC_MR & RTC_MR_HRMOD) == RTC_MR_HRMOD;
    uint32_t dwAlarmTime = RTC_TimeToTimeReg(hour, minute, second, n12HourMode);

    if( ucHour != UINT8_MAX )
    {
      dwAlarmTime |= RTC_TIMALR_HOUREN;
    }

    if( ucMinute != UINT8_MAX )
    {
      dwAlarmTime |= RTC_TIMALR_MINEN;
    }

    if( ucSecond != UINT8_MAX )
    {
      dwAlarmTime |= RTC_TIMALR_SECEN;
    }

    pRtc->RTC_TIMALR = dwAlarmTime;
  }

  {
    const uint8_t month = ucMonth != UINT8_MAX ? ucMonth : 0;
    const uint8_t day = ucDay != UINT8_MAX ? ucDay : 0;
    uint32_t dwAlarmDate = RTC_DateToCalReg(0, month, day, 0);

    if( ucMonth  != UINT8_MAX )
    {
      dwAlarmDate |= RTC_CALALR_MTHEN;
    }

    if( ucDay  != UINT8_MAX )
    {
      dwAlarmDate |= RTC_CALALR_DATEEN;
    }

    pRtc->RTC_CALALR = dwAlarmDate;
  }

  RTC_ClearSCCR(pRtc, RTC_SCCR_ALRCLR);
  RTC_EnableIt(pRtc, RTC_IER_ALREN);

  return (pRtc->RTC_VER & (RTC_VER_NVCAL | RTC_VER_NVTIM | RTC_VER_NVCALALR | RTC_VER_NVTIMALR));
}

extern unsigned RTC_GetTimeAlarm( Rtc* const pRtc, uint8_t* const pucHour, uint8_t* const pucMinute, uint8_t* const pucSecond )
{
  const uint32_t dwAlarmTime = pRtc->RTC_TIMALR ;

  // Get the alarm time in 24-hrs mode.
  RTC_TimeRegToTime(dwAlarmTime, 0, pucHour, pucMinute, pucSecond, pRtc->RTC_MR & RTC_MR_HRMOD);

  /* Hour */
  if ( pucHour  && !(dwAlarmTime & RTC_TIMALR_HOUREN) )
  {
      *pucHour = UINT8_MAX;
  }

  /* Minute */
  if ( pucMinute && !(dwAlarmTime & RTC_TIMALR_MINEN) )
  {
      *pucMinute = UINT8_MAX;
  }

  /* Second */
  if ( pucSecond && !(dwAlarmTime & RTC_TIMALR_SECEN) )
  {
      *pucSecond = UINT8_MAX;
  }

  return (pRtc->RTC_VER & (RTC_VER_NVCAL | RTC_VER_NVTIM | RTC_VER_NVCALALR | RTC_VER_NVTIMALR));
}

extern unsigned RTC_GetDateAlarm( Rtc* const pRtc, uint8_t* const pucMonth, uint8_t* const pucDay )
{
  const uint32_t dwAlarm = pRtc->RTC_CALALR;

  // Get the alarm date.
  RTC_CalRegToDate(dwAlarm, 0, pucMonth, pucDay, 0);

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

  return (pRtc->RTC_VER & (RTC_VER_NVCAL | RTC_VER_NVTIM | RTC_VER_NVCALALR | RTC_VER_NVTIMALR));
}

