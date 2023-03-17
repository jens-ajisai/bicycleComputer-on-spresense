#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/lte_module_event.h"
#include "config.h"

static int publish_lteConnected(int argc, char** argv) {
  if (argc < 2) {
    shell.println("echo argument required");
    return -1;
  }

  if(strlen(argv[1]) < 15) {
    eventManager.queueEvent(new LteEvent(LteEvent::LTE_EVT_CONNECTED, argv[1]));
  }

  return EXIT_SUCCESS;
};

static int publish_lteDisconnected(int argc, char** argv) {
  eventManager.queueEvent(new LteEvent(LteEvent::LTE_EVT_DISCONNECTED));

  return EXIT_SUCCESS;
};

static int publish_lteError(int argc, char** argv) {
  eventManager.queueEvent(new LteEvent(LteEvent::LTE_EVT_ERROR));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "lte_module.h"

static int lte_begin(int argc, char** argv) {
  LteWrapper.begin();

  return EXIT_SUCCESS;
};

static int lte_end(int argc, char** argv) {
  LteWrapper.end();

  return EXIT_SUCCESS;
};
#endif

void setup_shell_lte() {
  shell.addCommand(F("publish_lteConnected"), publish_lteConnected);
  shell.addCommand(F("publish_lteDisconnected"), publish_lteDisconnected);
  shell.addCommand(F("publish_lteError"), publish_lteError);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("lte_begin"), lte_begin);
  shell.addCommand(F("lte_end"), lte_end);   
#endif
}
