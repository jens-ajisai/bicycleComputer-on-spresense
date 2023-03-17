#pragma once

#include "EventManager.h"

class AppManagerEvent : public Event {
 public:
  enum AppStates {
    ApplicationColdSleep,
    ApplicationSystemStart,
    ApplicationUserStart,
    ApplicationUserMultimedia
  };

  enum AppConnStates { lteDisconnected, lteConnected };

  enum AppManagerEventCommand { APP_MANAGER_EVT_RUNLEVEL };

  const char* getCommandString() {
    switch (mCommand) {
      case APP_MANAGER_EVT_RUNLEVEL:
        return "RUNLEVEL";
      default:
        return "Command Unknown";
    }
  }

  AppManagerEvent(AppManagerEventCommand cmd, AppStates state, AppConnStates connState) {
    mType = Event::kEventAppManager;
    mCommand = cmd;
    mState = state;
    mConnState = connState;
    mCommandString = getCommandString();
  }

  AppStates getState() { return mState; }
  AppConnStates getConnState() { return mConnState; }

 private:
  AppStates mState;
  AppConnStates mConnState;
};