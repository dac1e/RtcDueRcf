# RtcSam3XA
Operate Arduinio Due RTC using C++ standards. Avoid read/write race conditions.

Characteristics:

-Get and set RTC local time by using C++ standard struct std::tm.
-Set RTC local time by the UTC time (Greenich mean time) stored in the C++ standard type std::time_t.
-Get and set RTC alarms.
-Adjust RTC to/from daylight savings period automatically, so that alarms will appear as expected.
-Avoid RTC/CPU race conditions that can cause in RTC read / write operations with wrong results.
-Use interrupt based setting of RTC time and date registers to avoid blocking the CPU for approximately
	350ms when setting time and date.

Unlike some other RTC libraries this library does avoid Race Conditions between the RTC and the CPU.

Here is an example of a race condition:

A librariy might provide the functions getMinute() and getSecond().
The RTC might transition from e.g. xx:01:59 to xx:02:00 between
the 2 subsequent calls of getMinute() and getSeconds().
Hence getMinutes() will retrieve 01 and getSeconds() will retrieve 00.
The combined result for minute and second will be xx::01:00 while
the RTC contains xx:02:00.

The solution of avoiding such race conditions is to only provide functions
that do an atomic read/write for all the time and date fields in one shot.