#pragma once

#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/gnss_module_event.h"

class GnssWrapperClass : public EventListener {
 public:
  bool begin();
  bool end() { return eventManager.removeListener(this); }
  bool mInitialized = false;

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

  void init();
  void deinit();

 private:
  void handleGnss(GnssEvent* event);
  void handleAppManager(AppManagerEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }


  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationSystemStart;
  AppManagerEvent::AppConnStates requiresConn = AppManagerEvent::lteConnected;
};

extern GnssWrapperClass GnssWrapper;

int add_geofence(double targetLatD, double targetLngD, uint8_t id, uint16_t radius,
                 uint16_t deadzone = 5, uint16_t dwell_detecttime = 10);
int remove_geofence(uint8_t id);
int poll_geofence(int argc, FAR char* argv[]);
int stop_geofence();
int start_geofence();
