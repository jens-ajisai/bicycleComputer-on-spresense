#pragma once

#include <Arduino.h>

#include "events/EventManager.h"
#include "events/ui_tft_event.h"

#include "events/cycle_sensor_event.h"
#include "events/lte_module_event.h"
#include "events/audio_module_event.h"
#include "events/cloud_module_event.h"
#include "events/GlobalStatus_event.h"
#include "events/gnss_module_event.h"
#include "events/ui_button_event.h"
#include "events/app_manager_event.h"

#include "memutils/memory_manager/MemManager.h"

#include "my_log.h"

class UiTftClass : public EventListener {
 public:
  bool begin(void);
  bool end(void);

  boolean publish(Event *ev) { return eventManager.queueEvent(ev); }

  MemMgrLite::MemHandle cam_mh;
  MemMgrLite::MemHandle disp_mh;

 private:
  void init();
  void deinit();
  void initDraw();
  void handleGnss(GnssEvent *ev);
  void handleGlobalStatus(GlobalStatusEvent *ev);
  void handleCycleSensor(CycleSensorEvent *ev);
  void handleLte(LteEvent *ev);
  void handleAudio(AudioEvent *ev);
  void handleCloud(CloudModuleEvent *ev);
  void handleUiTft(UiTftEvent *ev);
  void handleAppManager(AppManagerEvent* ev);
  void eventHandler(Event *event);
  virtual void operator()(Event *event) override { eventHandler(event); }

  bool mInitialized = false;
  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationUserStart;
};

extern UiTftClass UiTft;

void directCall_updateCameraImage(uint8_t *image_buffer, int width, int height);
void updateDebugGnss(int noSat);
