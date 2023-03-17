#include <Arduino.h>
#include <LTE.h>
#include <GNSS.h>
#include <RTC.h>
#include <sched.h>

#include "my_log.h"

#define MY_TIMEZONE_IN_SECONDS (9 * 60 * 60)  // JST

void printClock(RtcTime &rtc) {
  if (DEBUG_TIME) Log.traceln("%04d/%02d/%02d %02d:%02d:%02d", rtc.year(), rtc.month(),
              rtc.day(), rtc.hour(), rtc.minute(), rtc.second());
}

// This is not used, we use LTE to get the time
void setTimeByGNSS(SpGnssTime *time) {
  // Check if the acquired UTC time is accurate
  if (time->year >= 2000) {
    RtcTime now = RTC.getTime();
    // Convert SpGnssTime to RtcTime
    RtcTime gps(time->year, time->month, time->day, time->hour, time->minute,
                time->sec, time->usec * 1000);
#ifdef MY_TIMEZONE_IN_SECONDS
    // Set the time difference
    gps += MY_TIMEZONE_IN_SECONDS;
#endif
    int diff = now - gps;
    if (abs(diff) >= 1) {
      RTC.setTime(gps);
    }
  }
}

void setTimeByLTE(LTE *lteAccess) {
  unsigned long currentTime;
  while (0 == (currentTime = lteAccess->getTime())) {
    sleep(1);
    if (DEBUG_LTE) Log.verboseln("Re-enter setTimeByLTE.");
  }
  RtcTime rtc(currentTime);
  RTC.setTime(rtc);

  RtcTime now = RTC.getTime();
  if (DEBUG_LTE)
    Log.traceln("%d/%d/%d %d:%d:%d", now.year(), now.month(), now.day(),
              now.hour(), now.minute(), now.second());
}

void setup_system_time() {
  if (DEBUG_TIME) Log.traceln("setup_system_time");  

  //  We do not use any alarm set by RTC, so we do not need to start the alarm daemon
  //  begin is not required for setTime and getTime
  //  RTC.begin();

  // Set the temporary RTC time
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RTC.setTime(compiledDateTime);
}
