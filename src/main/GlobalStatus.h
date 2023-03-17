#pragma once

#include "events/EventManager.h"
#include "events/GlobalStatus_event.h"
#include "events/app_manager_event.h"
#include "events/cycle_sensor_event.h"
#include "events/gnss_module_event.h"
#include "events/lte_module_event.h"
#include "events/ui_tft_event.h"
#define FIRMWARE_VERSION 1

class GlobalStatusClass : public EventListener {
 public:
  bool begin(void);
  bool end(void) { return eventManager.removeListener(this); }

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

  static int getBatteryPercentage();
  GlobalStatusEvent::gStat status = {};
  String getStatusJson();

 private:
  void deinit();
  void init();

  void handleLte(LteEvent* ev);
  void handleCycleSensor(CycleSensorEvent* ev);
  void handleGnss(GnssEvent* ev);
  void handleUiTft(UiTftEvent* ev);
  void handleAppManager(AppManagerEvent* ev);
  void handleGlobalStatus(GlobalStatusEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  static int voltageToBatteryPercentage(int voltage);

  bool mInitialized = false;
  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationSystemStart;
};

extern GlobalStatusClass GlobalStatus;

#include <GNSS.h>
#include <LTE.h>
#include <RTC.h>
void setup_system_time();
void printClock(RtcTime& rtc);
void setTimeByGNSS(SpGnssTime* time);
void setTimeByLTE(LTE* lteAccess);

struct memoryInfo {
  char total[10];
  char free[10];
  char freeContinuous[10];
  char used[10];
};

memoryInfo printMemory();
memoryInfo updateFreeMemory();
memoryInfo updateFreeDisk();
