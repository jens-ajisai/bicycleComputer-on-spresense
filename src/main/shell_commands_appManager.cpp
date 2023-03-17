#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "config.h"
#include "events/app_manager_event.h"

static int publish_appManagerRunlevel(int argc, char** argv) {
  if (argc < 3) {
    shell.println("state argument required");
    return -1;
  }

  int appState = String(argv[1]).toInt();
  int connState = String(argv[2]).toInt();
  eventManager.queueEvent(new AppManagerEvent(AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL,
                                              (AppManagerEvent::AppStates)appState,
                                              (AppManagerEvent::AppConnStates)connState));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "app_manager.h"

static int appManager_begin(int argc, char** argv) {
  AppManager.begin();

  return EXIT_SUCCESS;
};

static int appManager_end(int argc, char** argv) {
  AppManager.end();

  return EXIT_SUCCESS;
};
#endif

void setup_shell_appManager() {
  shell.addCommand(F("publish_appManagerRunlevel"), publish_appManagerRunlevel);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("appManager_begin"), appManager_begin);
  shell.addCommand(F("appManager_end"), appManager_end);
#endif
}
