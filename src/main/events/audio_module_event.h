#pragma once

#include "EventManager.h"

enum AudioState { AudioStateNotInitialized, AudioStateIdle, AudioStateRecording };

class AudioEvent : public Event {
 public:
  enum AudioEventCommand {
    AUDIO_EVT_AUDIO,
    AUDIO_EVT_AUDIO_FINISHED,
    AUDIO_EVT_GET_AUDIO,
    AUDIO_EVT_STOP_AUDIO,
    AUDIO_EVT_ERROR
  };

  const char* getCommandString() {
    switch (mCommand) {
      case AUDIO_EVT_AUDIO:
        return "AUDIO";
      case AUDIO_EVT_AUDIO_FINISHED:
        return "AUDIO_FINISHED";
      case AUDIO_EVT_GET_AUDIO:
        return "GET_AUDIO";
      case AUDIO_EVT_STOP_AUDIO:
        return "STOP_AUDIO";
      case AUDIO_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  AudioEvent(AudioEventCommand cmd) {
    mType = Event::kEventAudio;
    mCommand = cmd;
    mCommandString = getCommandString();
  }

  AudioEvent(AudioEventCommand cmd, uint16_t duration) {
    mType = Event::kEventAudio;
    mCommand = cmd;
    mDuration = duration;
    mCommandString = getCommandString();
  }

  uint16_t getDuration() { return mDuration; }

 private:
  uint16_t mDuration;
};
