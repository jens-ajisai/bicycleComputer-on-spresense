#pragma once

#include <Camera.h>

#include "EventManager.h"

class CameraSensorEvent : public Event {
 public:
  enum cameraSensorEventCommand {
    CAMERA_EVT_RECORD_STREAM_ENABLE,
    CAMERA_EVT_RECORD_STREAM_DISABLE,
    // The image is too large to copy the data
    //    CAMERA_EVT_IMAGE_READY,
    CAMERA_EVT_REQUEST_IMAGE,
    CAMERA_EVT_ERROR
  };

  const char* getCommandString() {
    switch (mCommand) {
      case CAMERA_EVT_RECORD_STREAM_ENABLE:
        return "RECORD_STREAM_ENABLE";
      case CAMERA_EVT_RECORD_STREAM_DISABLE:
        return "RECORD_STREAM_DISABLE";
      case CAMERA_EVT_REQUEST_IMAGE:
        return "REQUEST_IMAGE";
      case CAMERA_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  CameraSensorEvent(cameraSensorEventCommand cmd) {
    mType = Event::kEventCamera;
    mCommand = cmd;
    mCommandString = getCommandString();
  }

  CameraSensorEvent(cameraSensorEventCommand cmd, CamImage img) {
    mType = Event::kEventCamera;
    mCommand = cmd;
    mImage = img;
    mCommandString = getCommandString();
  }

 private:
  CamImage mImage;
};
