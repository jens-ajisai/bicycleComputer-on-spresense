#pragma once

#include "EventManager.h"

class UiTftEvent : public Event {
 public:
  enum UiTftEventCommand { UITFT_EVT_ON, UITFT_EVT_OFF, UITFT_EVT_MSG_BOX };

  const char* getCommandString() {
    switch (mCommand) {
      case UITFT_EVT_ON:
        return "ON";
      case UITFT_EVT_OFF:
        return "OFF";
      case UITFT_EVT_MSG_BOX:
        return "MSG_BOX";
      default:
        return "Command Unknown";
    }
  }

  UiTftEvent(UiTftEventCommand cmd, String message = String()) {
    mType = Event::kEventUiTft;
    mCommand = cmd;
    mCommandString = getCommandString();
    _message = message;
  }

  String getMessage() { return _message; }

  String _message;
};
