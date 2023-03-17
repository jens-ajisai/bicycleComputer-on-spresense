#pragma once

#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/cloud_module_event.h"
#include "events/gnss_module_event.h"
#include "events/lte_module_event.h"

#include "memutils/memory_manager/MemManager.h"

class CloudModuleClass : public EventListener {
 public:
  bool begin(void);
  bool end(void);

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }
  uint32_t getIP();

  void getWeather(double lat, double lng, CloudModuleEvent::rainfallData* data);
  void getIceCream(double lat, double lng, int dist, const char* query,
                   CloudModuleEvent::storeInfo* info);
  void updateIceCreamDistance(double ownLat, double ownLng, double shopLat, double shopLng,
                              CloudModuleEvent::storeInfo* info);
 private:
  void init();
  void deinit();
  uint32_t getDistanceOnline(double lat0, double lng0, double lat1, double lng1);
  void handleCloudModule(CloudModuleEvent* ev);
  void handleLte(LteEvent* ev);
  void handleGnss(GnssEvent* ev);
  void handleAppManager(AppManagerEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  MemMgrLite::MemHandle net_mh;

  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationSystemStart;
  AppManagerEvent::AppConnStates requiresConn = AppManagerEvent::lteConnected;
};

extern CloudModuleClass CloudModule;
