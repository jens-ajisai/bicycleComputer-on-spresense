#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/audio_module_event.h"
#include "config.h"

static int publish_audioAudio(int argc, char** argv) {
  eventManager.queueEvent(new AudioEvent(AudioEvent::AUDIO_EVT_AUDIO));

  return EXIT_SUCCESS;
};

static int publish_audioGetAudio(int argc, char** argv) {
  if (argc < 2) {
    shell.println("duration argument required");
    return -1;
  }

  int duration = String(argv[1]).toInt();  
  eventManager.queueEvent(new AudioEvent(AudioEvent::AUDIO_EVT_GET_AUDIO, duration));

  return EXIT_SUCCESS;
};

static int publish_audioError(int argc, char** argv) {
  eventManager.queueEvent(new AudioEvent(AudioEvent::AUDIO_EVT_ERROR));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "audio_module.h"

static int audio_begin(int argc, char** argv) {
  AudioModule.begin();

  return EXIT_SUCCESS;
};

static int audio_end(int argc, char** argv) {
  AudioModule.end();

  return EXIT_SUCCESS;
};
#endif

void setup_shell_audio() {
  shell.addCommand(F("publish_audioAudio"), publish_audioAudio);
  shell.addCommand(F("publish_audioGetAudio"), publish_audioGetAudio);
  shell.addCommand(F("publish_audioError"), publish_audioError);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("audio_begin"), audio_begin);
  shell.addCommand(F("audio_end"), audio_end);     
#endif
}
