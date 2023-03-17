#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "config.h"
#include "events/cloud_module_event.h"

static int publish_cloudGetWeather(int argc, char** argv) {
  eventManager.queueEvent(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_WEATHER));

  return EXIT_SUCCESS;
};

static int publish_cloudGetIceCream(int argc, char** argv) {
  eventManager.queueEvent(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_ICECREAM));

  return EXIT_SUCCESS;
};

static int publish_cloudWeather(int argc, char** argv) {
  if (argc < 2) {
    shell.println("echo argument required");
    return -1;
  }

  double rainfall = String(argv[1]).toDouble();
  eventManager.queueEvent(
      new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_WEATHER, rainfall));

  return EXIT_SUCCESS;
};

static int publish_cloudIceCream(int argc, char** argv) {
  if (argc < 4) {
    shell.println("echo argument required");
    return -1;
  }

  CloudModuleEvent::storeInfo info;
  info.name = String(argv[1]);
  info.distance = String(argv[2]).toDouble();
  info.bearing = String(argv[3]).toDouble();

  eventManager.queueEvent(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_ICECREAM, info));

  return EXIT_SUCCESS;
};

static int publish_cloudError(int argc, char** argv) {
  eventManager.queueEvent(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_ERROR));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "cloud_module.h"

static int cloud_begin(int argc, char** argv) {
  CloudModule.begin();

  return EXIT_SUCCESS;
};

static int cloud_end(int argc, char** argv) {
  CloudModule.end();

  return EXIT_SUCCESS;
};
#endif

void setup_shell_cloud() {
  shell.addCommand(F("publish_cloudGetWeather"), publish_cloudGetWeather);
  shell.addCommand(F("publish_cloudGetIceCream"), publish_cloudGetIceCream);
  shell.addCommand(F("publish_cloudWeather f<rain>"), publish_cloudWeather);
  shell.addCommand(F("publish_cloudIceCream s<name> d<dist> f<heading>"), publish_cloudIceCream);
  shell.addCommand(F("publish_cloudError"), publish_cloudError);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("cloud_begin"), cloud_begin);
  shell.addCommand(F("cloud_end"), cloud_end);
#endif
}
