#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/ui_button_event.h"
#include "config.h"

static int publish_uiButtonPressed(int argc, char** argv) {
  eventManager.queueEvent(new ButtonEvent(ButtonEvent::BUTTON_EVT_PRESSED));

  return EXIT_SUCCESS;
};

static int publish_uiButtonReleased(int argc, char** argv) {
  eventManager.queueEvent(new ButtonEvent(ButtonEvent::BUTTON_EVT_RELEASED));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "ui_button.h"

static int uiButton_begin(int argc, char** argv) {
  UiButton.begin();

  return EXIT_SUCCESS;
};

static int uiButton_end(int argc, char** argv) {
  UiButton.end();

  return EXIT_SUCCESS;
};
#endif


void setup_shell_uiButton() {
  shell.addCommand(F("publish_uiButtonPressed"), publish_uiButtonPressed);
  shell.addCommand(F("publish_uiButtonReleased"), publish_uiButtonReleased);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("uiButton_begin"), uiButton_begin);
  shell.addCommand(F("uiButton_end"), uiButton_end);    
#endif
}
