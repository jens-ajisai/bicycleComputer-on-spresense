#include <Arduino.h>
#include <SDHCI.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "sd_utils.h"

// set SIMPLE_SERIAL_SHELL_MAXARGS to 14 in the library

#include "config.h"
#include "my_log.h"

int shellTask = -1;

#include <sched.h>

static int shell_main(int argc, char* argv[]);

#include "nshlib.h"
static int nsh_main(int argc, char* argv[]) {
  int ret = nsh_consolemain(0, NULL);

  // not possible to return to Arduino Serial and shell_main!
  return ret;
}

static int showID(int argc, char** argv) {
  shell.println("Running " __FILE__ ",\nBuilt " __DATE__);
  return 0;
};

static int startUsbMsc(int argc, char** argv) {
  SdUtils.startUsbMsc();
  return 0;
};

static int stopUsbMsc(int argc, char** argv) {
  SdUtils.stopUsbMsc();
  return 0;
};

static int printFile(int argc, char** argv) {
  if (argc < 2) {
    shell.println("filename required");
    return -1;
  }

  uint8_t buffer[256] = {0};
  SdUtils.read(argv[1], buffer, sizeof(buffer));
  buffer[255] = 0;

  if (DEBUG_MAIN) Log.infoln("%s:\n%s", argv[1], buffer);

  return 0;
};

static int listDirectory(int argc, char** argv) {
  if (argc < 2) {
    SdUtils.listDirectory("");
  } else {
    SdUtils.listDirectory(argv[1]);
  }

  return 0;
};

static int nsh(int argc, char** argv) {
  Serial.end();
  int nshTask = task_create("nsh", SCHED_PRIORITY_DEFAULT, CONFIG_SHELL_MODULE_STACK_SIZE, nsh_main, NULL);

#ifdef CONFIG_SMP
  cpu_set_t cpuset = 1 << CPU_AFFINITY_SHELL;
  sched_setaffinity(nshTask, sizeof(cpu_set_t), &cpuset);
#endif

  task_delete(shellTask);
  return 0;
};

static int test_shell(int argc, char** argv) {
  if (argc < 2) {
    shell.println("echo argument required");
    return -1;
  }
  shell.println(argv[1]);

  return EXIT_SUCCESS;
};

static int shell_main(int argc, char* argv[]) {
  if (DEBUG_MAIN) Log.verboseln("SHELL IS READY");
  while (true) {
    shell.executeIfInput();
    sleep(1);
    if (DEBUG_MAIN_EXTRA) Log.verboseln("Re-enter shell_main.");
  }
}

extern void registerAllModules();
extern void unregisterAllModules();

static int shell_registerAllModules(int argc, char** argv) {
  registerAllModules();
  return EXIT_SUCCESS;
};

static int shell_unregisterAllModules(int argc, char** argv) {
  unregisterAllModules();
  return EXIT_SUCCESS;
};

extern void setup_shell_gnss();
extern void setup_shell_uitft();
extern void setup_shell_uiButton();
extern void setup_shell_mqtt();
extern void setup_shell_lte();
extern void setup_shell_imu();
extern void setup_shell_globalStatus();
extern void setup_shell_camera();
extern void setup_shell_audio();
extern void setup_shell_cloud();
extern void setup_shell_cycle();
extern void setup_shell_appManager();

void setup_shell() {
  if (DEBUG_SHELL) Log.traceln("setup_shell");

  shell.attach(Serial);

  shell.addCommand(F("id?"), showID);
  shell.addCommand(F("test_shell <echo>"), test_shell);
  shell.addCommand(F("startUsbMsc"), startUsbMsc);
  shell.addCommand(F("stopUsbMsc"), stopUsbMsc);
  shell.addCommand(F("printFile"), printFile);
  shell.addCommand(F("listDirectory"), listDirectory);
  shell.addCommand(F("nsh"), nsh);
  shell.addCommand(F("registerAllModules"), shell_registerAllModules);
  shell.addCommand(F("unregisterAllModules"), shell_unregisterAllModules);

  setup_shell_gnss();
  setup_shell_uitft();
  setup_shell_uiButton();
  setup_shell_mqtt();
  setup_shell_lte();
  setup_shell_imu();
  setup_shell_globalStatus();
  setup_shell_camera();
  setup_shell_audio();
  setup_shell_cloud();
  setup_shell_cycle();
  setup_shell_appManager();

  shellTask = task_create("shell", SCHED_PRIORITY_DEFAULT, CONFIG_SHELL_MODULE_STACK_SIZE, shell_main, NULL);

#ifdef CONFIG_SMP
  cpu_set_t cpuset = 1 << CPU_AFFINITY_SHELL;
  sched_setaffinity(shellTask, sizeof(cpu_set_t), &cpuset);
#endif
}
