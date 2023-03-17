#pragma once

#include "EventManager.h"

class CloudModuleEvent : public Event {
 public:
  struct storeInfo {
    String name;
    double lat;
    double lng;
    double distance;
    double bearing;
  };

  struct rainfallData {
    uint64_t date;
    u_int16_t rainfall00;
    u_int16_t rainfall10;
    u_int16_t rainfall20;
    u_int16_t rainfall30;
    u_int16_t rainfall40;
    u_int16_t rainfall50;
    u_int16_t rainfall60;
    u_int16_t rainfallTotal;
  };

  enum CloudModuleEventCommand {
    CLOUDMODULE_EVT_GET_WEATHER,
    CLOUDMODULE_EVT_GET_ICECREAM,
    CLOUDMODULE_EVT_GET_IP,
    CLOUDMODULE_EVT_WEATHER,
    CLOUDMODULE_EVT_ICECREAM,
    CLOUDMODULE_EVT_ERROR
  };

  const char* getCommandString() {
    switch (mCommand) {
      case CLOUDMODULE_EVT_GET_WEATHER:
        return "GET_WEATHER";
      case CLOUDMODULE_EVT_GET_ICECREAM:
        return "GET_ICECREAM";
      case CLOUDMODULE_EVT_GET_IP:
        return "GET_IP";
      case CLOUDMODULE_EVT_WEATHER:
        return "WEATHER";
      case CLOUDMODULE_EVT_ICECREAM:
        return "ICECREAM";
      case CLOUDMODULE_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  CloudModuleEvent(CloudModuleEventCommand cmd) {
    mType = Event::kEventCloudModule;
    mCommand = cmd;
    mCommandString = getCommandString();
  }

  CloudModuleEvent(CloudModuleEventCommand cmd, double rainfall) {
    mType = Event::kEventCloudModule;
    mCommand = cmd;
    mRainfall = rainfall;
    mCommandString = getCommandString();
  }

  CloudModuleEvent(CloudModuleEventCommand cmd, storeInfo info) {
    mType = Event::kEventCloudModule;
    mCommand = cmd;
    mInfo = info;
  }

  double getRainfall() { return mRainfall; }
  storeInfo getStoreInfo() { return mInfo; }

 private:
  double mRainfall;
  storeInfo mInfo;
};
