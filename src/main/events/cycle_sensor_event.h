#pragma once

#include "EventManager.h"

enum CycleSensorState { CycleSensorStateConnected, CycleSensorStateDisconnected };

class CycleSensorEvent : public Event {
 public:
  enum CycleSensorCommand {
    CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED,
    CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED,
    CYCLE_SENSOR_EVT_HEART_RATE_READING,
    CYCLE_SENSOR_EVT_SAW_MOBILE,
    CYCLE_SENSOR_EVT_LOST_MOBILE,
    CYCLE_SENSOR_EVT_ERROR
  };

  const char* getCommandString() {
    switch (mCommand) {
      case CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED:
        return "HEART_RATE_CONNECTED";
      case CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED:
        return "HEART_RATE_DISCONNECTED";
      case CYCLE_SENSOR_EVT_HEART_RATE_READING:
        return "HEART_RATE_READING";
      case CYCLE_SENSOR_EVT_SAW_MOBILE:
        return "SAW_MOBILE";
      case CYCLE_SENSOR_EVT_LOST_MOBILE:
        return "LOST_MOBILE";
      case CYCLE_SENSOR_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  CycleSensorEvent(CycleSensorCommand cmd, int heartRate = 0) {
    mType = Event::kEventCycleSensor;
    mCommand = cmd;
    mHeartRate = heartRate;
    mCommandString = getCommandString();
  }

  int getHeartRate() { return mHeartRate; }

 private:
  int mHeartRate;
};
