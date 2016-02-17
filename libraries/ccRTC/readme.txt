CC-RTC
RTC emulation library for CC3200 on the Energia Framework

Overview

This library is written with the Sparkcore "Time" library in mind which is an RTC library with many features. However additional methods still have not been implemented. You can check out the functionality here http://docs.spark.io/firmware/#libraries-timeIt

The library includes the following methods:

ccRTC.zone()
ccRTC.now()
ccRTC.year()
ccRTC.month()
ccRTC.day()
ccRTC.weekday()
ccRTC.hour()
ccRTC.minute()
** ccRTC.begin()
The library uses the PRCM library of the CC3200 source code. There is an RTC function that has been exposed in the PRCM library which has been used to get the RTC functionality.

The library follows the UNIX TIMESTAMP, so it has a range from 1st Jan 1970 to 18th Jan 2038

Limitations

As far as my tests have shown, the RTC function is accurate to only about 4 seconds per hour, that is your time will be off by about 4 seconds every hour. This is a big drawback especially if you're using this for time critical applications. For applications that don't need super accuracy, this is a good enough library provided you use an online server to sync time regularly.

Currently library is not foolproof and may be prone to bugs. However my tests haven't shown any issues.

If your're using NTP Timestamp to sync the onboard RTC Clock then the NTP timestamp needs to be converted from the year 1900 base to a 1970 base. This can be done by subtracting the total seconds from 1900 to 1970 from the NTP Timestamp to obtain the UNIX Timestamp.

Usage

1) Download and extract the ccRTC folder into the /Energia/hardware/cc3200/libraries folder.

2) In your code use #include "ccRTC.h" at the top.

3) An instance of the rtcClass called "ccRTC" is already declared, so you can directly call methods like e.g. "ccRTC.method()".

4) You have to do a mandatory call to ccRTC.begin() in your Setup() and you're good to go.

5) Ideally call ccRTC.setTime() and ccRTC.zone() in your Setup to initialize your RTC to the current time and timezone.
