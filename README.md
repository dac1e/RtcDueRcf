# RtcDueRcf

Operate Arduino Due builtin RTC. 

Characteristics:
  - Get and set RTC local time by using C++ standard struct std::tm.
  - Set RTC local time via an UTC time stored in the C++ standard type std::time_t.
  - Get and set RTC alarms.
  - Adjust RTC to/from daylight savings period automatically, so that alarms will appear correctly.
  - Use interrupt based setting of RTC time and date registers to avoid 350ms CPU blocking when setting time and date.
  - Avoid RTC/CPU race conditions that can cause RTC read and write operations with wrong results.
  
In contrast to the RTCDue library, this library operates Arduinio Due RTC with a 'Race Condition Free' API.

Explanation of Race Condition:
For example, the RTCDue API provides the functions getMinute() and getSecond(). The RTC might transition from e.g. xx:01:59 to xx:02:00 between the 2 subsequent calls of getMinute() and getSecond(). Hence getMinutes() will retrieve 01 and getSeconds() will retrieve 00. The combined result for minute and second will be xx::01:00 while the RTC contains xx:02:00.
