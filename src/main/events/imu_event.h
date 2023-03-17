#pragma once

#include "EventManager.h"

class ImuEvent : public Event {
 public:
  enum ImuEventCommand {
    IMU_EVT_ORIENTATION,
    IMU_EVT_FALL,
    IMU_EVT_MOVED,
    IMU_EVT_STILL,
    IMU_EVT_ERROR
  };

  const char* getCommandString() {
    switch (mCommand) {
      case IMU_EVT_ORIENTATION:
        return "ORIENTATION";
      case IMU_EVT_FALL:
        return "FALL";
      case IMU_EVT_MOVED:
        return "MOVED";
      case IMU_EVT_STILL:
        return "STILL";
      case IMU_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  ImuEvent(ImuEventCommand cmd) {
    mType = Event::kEventImu;
    mCommand = cmd;
    mCommandString = getCommandString();
  }
};
