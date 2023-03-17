#ifndef __Event_h
#define __Event_h

#include <vector>

#include "../my_log.h"

class EventManager;

class Event {
 public:

  enum EventType {
    kEventUnknown = 10,
    kEventAll,
    kEventAppManager,
    kEventUiButton,
    kEventCamera,
    kEventMqtt,
    kEventGnss,
    kEventLte,
    kEventUiTft,
    kEventAudio,
    kEventCloudModule,
    kEventImu,
    kEventGlobalStatus,
    kEventCycleSensor,
    kEventImageBuffer,
    kEventAntiTheft
  };

  static const char* eventTypeToString(EventType type) {
    switch (type) {
      case kEventAppManager:
        return "EvAPPMANAGER";
      case kEventUiButton:
        return "EvBUTTON";
      case kEventCamera:
        return "EvCAMERA";
      case kEventMqtt:
        return "EvMQTT";
      case kEventGnss:
        return "EvGNSS";
      case kEventLte:
        return "EvLTE";
      case kEventUiTft:
        return "EvUITFT";
      case kEventAudio:
        return "EvAUDIO";
      case kEventCloudModule:
        return "EvCLOUDMODULE";
      case kEventImu:
        return "EvIMU";
      case kEventGlobalStatus:
        return "EvGLOBALSTATUS";
      case kEventCycleSensor:
        return "EvCYCLESENSOR";
      case kEventImageBuffer:
        return "EvIMAGEBUFFER";
      case kEventAntiTheft:
        return "EVANTITHEFT";
      default:
        return "EvUNKNOWN";
    }
  }

  Event() {}
  ~Event() {}

  Event(EventType type) {
    mType = type;
    mCommand = 0;
    mCommandString = String();
  }

  EventType getType() { return mType; }
  int getCommand() { return mCommand; }
  const char* getCommandString() { return mCommandString.c_str(); }
  int getData() { return mData; }

  bool isRemote = false;

 protected:
  EventType mType;
  int mCommand;
  String mCommandString;
  int mData;
};

#endif