#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/ui_tft_event.h"
#include "config.h"

static int publish_uitftOn(int argc, char** argv) {
  eventManager.queueEvent(new UiTftEvent(UiTftEvent::UITFT_EVT_ON));

  return EXIT_SUCCESS;
};

static int publish_uitftOff(int argc, char** argv) {
  eventManager.queueEvent(new UiTftEvent(UiTftEvent::UITFT_EVT_OFF));

  return EXIT_SUCCESS;
};

static int publish_uitftMsgBox(int argc, char** argv) {
  if (argc < 2) {
    shell.println("echo argument required");
    return -1;
  }

  String message = String(argv[1]);
  eventManager.queueEvent(new UiTftEvent(UiTftEvent::UITFT_EVT_MSG_BOX, message));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "ui_tft.h"

static int uitft_begin(int argc, char** argv) {
  UiTft.begin();

  return EXIT_SUCCESS;
};

static int uitft_end(int argc, char** argv) {
  UiTft.end();

  return EXIT_SUCCESS;
};
#endif


void setup_shell_uitft() {
  shell.addCommand(F("publish_uitftOn"), publish_uitftOn);
  shell.addCommand(F("publish_uitftOff"), publish_uitftOff);
  shell.addCommand(F("publish_uitftMsgBox"), publish_uitftMsgBox);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("uitft_begin"), uitft_begin);
  shell.addCommand(F("uitft_end"), uitft_end);    
#endif
}
