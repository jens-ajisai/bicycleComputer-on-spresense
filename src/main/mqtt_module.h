#pragma once

#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/gnss_module_event.h"
#include "events/lte_module_event.h"
#include "events/mqtt_module_event.h"

#include "memutils/memory_manager/MemManager.h"

// including the camera_module causes circular dependencies
#include "Camera.h"

class MqttModuleClass : public EventListener {
 public:
  bool begin();
  bool end();

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }
  void directCall_sendImage(CamImage& img);
  void sendMqttMessage(const char* topic, uint8_t* data, size_t size);

  void init();
  void deinit();

  bool mConnected = false;

 private:

  void handleMqtt(MqttEvent* ev);
  void handleGnss(GnssEvent* ev);
  void handleAppManager(AppManagerEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  MemMgrLite::MemHandle read_buf_mh;
  MemMgrLite::MemHandle write_buf_mh;

  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationSystemStart;
  AppManagerEvent::AppConnStates requiresConn = AppManagerEvent::lteConnected;
};

extern MqttModuleClass MqttModule;
