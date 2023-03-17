#pragma once

#include "EventManager.h"

class LteEvent : public Event {
 public:
  enum LteEventCommand { LTE_EVT_CONNECTED, LTE_EVT_DISCONNECTED, LTE_EVT_IP, LTE_EVT_ERROR };

  const char* getCommandString() {
    switch (mCommand) {
      case LTE_EVT_CONNECTED:
        return "CONNECTED";
      case LTE_EVT_DISCONNECTED:
        return "DISCONNECTED";
      case LTE_EVT_IP:
        return "IP";
      case LTE_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  LteEvent(LteEventCommand cmd, const char* ipAddress = "") {
    mType = Event::kEventLte;
    mCommand = cmd;
    mCommandString = getCommandString();
    strncpy(_ipAddress, ipAddress, 16);
  }

  const char* getIpAddress() { return _ipAddress; }

 private:
  char _ipAddress[16];
};
