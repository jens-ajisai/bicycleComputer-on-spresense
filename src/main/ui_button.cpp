#include "ui_button.h"

#include "config.h"
#include "events/EventManager.h"
#include "my_log.h"

void button_state_change() {
  ButtonState state = (ButtonState)digitalRead(BUTTON_PIN);
  switch (state) {
    case ButtonStatePressed:
      if (DEBUG_BUTTON) Log.traceln("Button state PRESSED %d", state);
      break;
    case ButtonStateReleased:
      if (DEBUG_BUTTON) Log.traceln("Button state RELEASED %d", state);
      UiButton.publish(new ButtonEvent(ButtonEvent::BUTTON_EVT_RELEASED));
      break;

    default:
      break;
  }
}

void UiButtonClass::init() {
  if (!mInitialized) {
    if (DEBUG_BUTTON) Log.traceln("UiButtonClass::init");
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(BUTTON_PIN, button_state_change, CHANGE);
    if (DEBUG_BUTTON) Log.traceln("UiButtonClass::init complete");
    mInitialized = true;
  }
}

void UiButtonClass::deinit() {
  if (mInitialized) {
    detachInterrupt(BUTTON_PIN);
    mInitialized = false;
  }
}

void UiButtonClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
 //     deinit();
    }
  }
}

void UiButtonClass::eventHandler(Event* event) {
  if (DEBUG_BUTTON) Log.traceln("UiButtonClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;

    default:
      break;
  }
}

bool UiButtonClass::begin() {
  if (DEBUG_BUTTON) Log.traceln("UiButtonClass::begin");
  return eventManager.addListener(Event::kEventAppManager, this);
}

UiButtonClass UiButton;
