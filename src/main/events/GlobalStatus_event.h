#pragma once

#include "EventManager.h"

#define FIRMWARE_VERSION 1

class GlobalStatusEvent : public Event {
 public:
  enum GlobalStatusEventCommand {
    GLOBALSTATUS_EVT_UPDATE,
    GLOBALSTATUS_EVT_ERROR,
    GLOBALSTATUS_EVT_TEST_MEMORY,
    GLOBALSTATUS_EVT_TEST_STATUS_JSON,
    GLOBALSTATUS_EVT_TEST_BATTERY_PERCENTAGE
  };

  const char* getCommandString() {
    switch (mCommand) {
      case GLOBALSTATUS_EVT_UPDATE:
        return "UPDATE";
      case GLOBALSTATUS_EVT_ERROR:
        return "ERROR";
      case GLOBALSTATUS_EVT_TEST_STATUS_JSON:
        return "TEST_STATUS_JSON";
      case GLOBALSTATUS_EVT_TEST_BATTERY_PERCENTAGE:
        return "TEST_BATTERY_PERCENTAGE";
      default:
        return "Command Unknown";
    }
  }

  struct gStat {
    int batteryPercentage;
    bool bleConnected;
    bool phoneNearby;
    bool lteConnected;
    char ipAddress[16];
    double lat;
    double lng;
    double speed;
    double direction;
    int geoFenceStatus;
    bool displayOn;
    int firmwareVersion = FIRMWARE_VERSION;
    char sdCardFreeMemory[10];
    char heapFreeMemory[10];
  };

  GlobalStatusEvent(GlobalStatusEventCommand cmd, gStat stat) {
    mType = Event::kEventGlobalStatus;
    mCommand = cmd;
    mCommandString = getCommandString();
    mStat = stat;
  }

  GlobalStatusEvent(GlobalStatusEventCommand cmd) {
    mType = Event::kEventGlobalStatus;
    mCommand = cmd;
    mCommandString = getCommandString();
  }

  gStat getStatus() { return mStat; }

 private:
  gStat mStat = {};
};
