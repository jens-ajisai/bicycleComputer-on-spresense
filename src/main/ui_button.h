#pragma once

#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/ui_button_event.h"

class UiButtonClass : public EventListener {
 public:
  bool begin(void);
  bool end(void) { return eventManager.removeListener(this); }

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

 private:
  void init();
  void deinit();
  void handleAppManager(AppManagerEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  bool mInitialized = false;
  AppManagerEvent::AppStates initializeAt = AppManagerEvent::ApplicationSystemStart;
};

extern UiButtonClass UiButton;
