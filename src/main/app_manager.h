#pragma once

#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/cycle_sensor_event.h"
#include "events/imu_event.h"
#include "events/lte_module_event.h"
#include "events/ui_button_event.h"

class AppManagerClass : public EventListener {
 public:
  bool begin(void);
  bool end(void) { return eventManager.removeListener(this); }

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

 private:
  void setClockSpeed();
  void setAppConnState(AppManagerEvent::AppConnStates state);
  void setAppState(AppManagerEvent::AppStates state);

  void handleImu(ImuEvent* ev);
  void handleButton(ButtonEvent* ev);
  void handleCycleSensor(CycleSensorEvent* ev);
  void handleLte(LteEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  bool mInitialized = false;
  AppManagerEvent::AppStates mAppState = AppManagerEvent::ApplicationColdSleep;
  AppManagerEvent::AppConnStates mAppConnState = AppManagerEvent::lteDisconnected;
};

extern AppManagerClass AppManager;
