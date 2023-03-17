#pragma once

#include <Camera.h>

#include "config.h"
#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/camera_sensor_event.h"
#include "events/mqtt_module_event.h"
#include "events/ui_button_event.h"

class CameraSensorClass : public EventListener {
 public:
  bool begin(void);
  bool end(void) { return eventManager.removeListener(this); }

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

  // needs to be public because the thread runs from a free function and calls this.
  CamImage takeImage(int width = 0, int height = 0, int divisor = CONFIG_JPEG_BUFFER_SIZE_DIVISOR,
                     int quality = CONFIG_JPEG_QUALITY);

 private:
  void init();
  void deinit();
  void handleButton(ButtonEvent* ev);
  void handleMqtt(MqttEvent* ev);
  void handleCamera(CameraSensorEvent* ev);
  void handleAppManager(AppManagerEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  bool mInitialized = false;
  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationUserStart;
};

extern CameraSensorClass CameraSensor;
