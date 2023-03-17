#include "app_manager.h"
#include "config.h"
#include "events/EventManager.h"
#include "shell_commands.h"

#include <arch/board/board.h>

#if DISPLAY_SPI != 5
#error "DISPLAY_SPI selection is wrong!!"
#endif

#include "utils.h"

/*
TODO

* fix the issue of correct initialization.
* When recording audio, sending to asw is too slow because the client is too busy to get idle and the memory runs out, allocation fails.
* Audio data contains wrong values, some overflow somewhere. Data is cut off together with a null terminator?


TODO maybe - rather not
* Use SDK for Camera to control the buffer which should map to the memory tile
* Fix the performance of the BLE log using simple FIFO
* Revise the PSM or EDRX settings for LTE. 
  (probably does not make sense, only for a device sleeping most of the time keeping the connection to lte)
* Use a simple FIFO for the BLE log.
* log the bytes with the ArduinoLog in the BLE module


*/


void setup() {
#ifdef CONFIG_SMP
  cpu_set_t cpuset = 1 << CPU_AFFINITY_MAIN;
  cpu_set_t cpusetOrig;
  sched_getaffinity(getpid(), sizeof(cpu_set_t), &cpusetOrig);
  sched_setaffinity(getpid(), sizeof(cpu_set_t), &cpuset);

  // Sleep that the CPU is changed. yield() is not enough.
  usleep(10 * 1000);
  printf("CPU: %d\n", sched_getcpu());
#endif

  // Init the shared memory globally
  if (!app_init_libraries()) {
    printf("Error: init_libraries() failure.\n");
    assert(false);
  }

  Serial.begin(115200);
  while (!Serial) {
  }

#ifndef NO_LOGS
  setup_log();
#endif

#ifndef CONFIG_REMOVE_SHELL
  setup_shell();
#endif

  AppManager.begin();
}

uint32_t last = 0;
uint32_t warningTimeout = 3 * 1000;

void loop() {
  last = millis();
  int processedEvents = eventManager.processEvent();
  if (processedEvents) {
    if (DEBUG_MAIN) Log.traceln("Processed %d listeners", processedEvents);
    if (millis() - last > warningTimeout) {
      if (DEBUG_MAIN) Log.noticeln(" *** Processing Events took %d", millis() - last);
    }
  } else {
    usleep(100 * 1000);
  }
}