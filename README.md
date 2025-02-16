# RtcSam3XA
Operate Arduinio Due RTC using C++ standard time functions. RTC Alarms follow daylight savings transitions. Perform RTC read/write operations without race conditions. Respect time zone information set be tzset() function. 

Characteristics:

  - Get and set RTC local time by using C++ standard struct std::tm.
  - Set RTC local time via an UTC time stored in the C++ standard type std::time_t.
  - Get and set RTC alarms.
  - Adjust RTC to/from daylight savings period automatically, so that alarms will appear at local time.
  - Use interrupt based setting of RTC time and date registers to avoid 350ms CPU blocking when setting time and date.
  - No RTC/CPU race conditions that can cause in RTC read or write operations with wrong results.
  
This library does avoid 'Race Conditions' between the RTC and the CPU.

Here is an example of a race condition:

An API might provide the functions getMinute() and getSecond() being called by the client.
The RTC might transition from e.g. xx:01:59 to xx:02:00 between
the 2 subsequent calls of getMinute() and getSecond().
Hence getMinutes() will retrieve 01 and getSeconds() will retrieve 00.
The combined result for minute and second will be xx::01:00 while
the RTC contains xx:02:00.

This library avoids Race Conditions by providing functions that do an atomic read/write for all the time and date fields.
