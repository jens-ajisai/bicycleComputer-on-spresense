#pragma once

#include "EventManager.h"

class AntiTheftEvent : public Event {
 public:
  enum AntiTheftEventCommand { ANTI_THEFT_EVT_DETECTED };

  const char* getCommandString() {
    switch (mCommand) {
      case ANTI_THEFT_EVT_DETECTED:
        return "DETECTED";
      default:
        return "Command Unknown";
    }
  }
  AntiTheftEvent(AntiTheftEventCommand cmd) {
    mType = Event::kEventAntiTheft;
    mCommand = cmd;
    mCommandString = getCommandString();
  }
};
