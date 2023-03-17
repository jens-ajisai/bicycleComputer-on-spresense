#pragma once

#include "EventManager.h"

enum ButtonState { ButtonStatePressed, ButtonStateReleased };

class ButtonEvent : public Event {
 public:
  enum ButtonEventCommand { BUTTON_EVT_PRESSED, BUTTON_EVT_RELEASED };

  const char* getCommandString() {
    switch (mCommand) {
      case BUTTON_EVT_PRESSED:
        return "PRESSED";
      case BUTTON_EVT_RELEASED:
        return "RELEASED";
      default:
        return "Command Unknown";
    }
  }

  ButtonEvent(ButtonEventCommand cmd) {
    mType = Event::kEventUiButton;
    mCommand = cmd;
    mCommandString = getCommandString();
  }
};
