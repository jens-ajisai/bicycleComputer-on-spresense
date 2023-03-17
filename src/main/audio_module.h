#pragma once

#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/audio_module_event.h"
#include "events/mqtt_module_event.h"
#include "events/ui_button_event.h"

class AudioModuleClass : public EventListener {
 public:
  bool begin(void);
  bool end(void) { return eventManager.removeListener(this); }

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

 private:
  void init();
  void deinit();
  void handleAppManager(AppManagerEvent* ev);
  void handleButton(ButtonEvent* ev);
  void handleAudio(AudioEvent* ev);
  void handleMqtt(MqttEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  void startRecording(uint16_t duration = 10);

  bool mInitialized = false;
  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationUserStart;
};

extern AudioModuleClass AudioModule;
