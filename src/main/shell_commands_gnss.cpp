#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/gnss_module_event.h"
#include "config.h"

static int publish_gnssPosition(int argc, char** argv) {
  if (argc < 6) {
    shell.println("echo argument required");
    return -1;
  }

  float lat = String(argv[1]).toFloat();
  float lng = String(argv[2]).toFloat();
  float direction = String(argv[3]).toFloat();
  float speed = String(argv[4]).toFloat();
  bool fix = (argv[5] > 0);

  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_POSITION, lat, lng, speed, direction, fix));

  return EXIT_SUCCESS;
};

static int publish_gnssSetGeoFence(int argc, char** argv) {
  if (argc < 5) {
    shell.println("echo argument required");
    return -1;
  }

  float lat = String(argv[1]).toFloat();
  float lng = String(argv[2]).toFloat();
  uint16_t radius = String(argv[3]).toInt();
  int id = String(argv[4]).toInt();

  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_SET_GEO_FENCE, lat, lng, radius, (GnssEvent::GnssGeofenceId) id));

  return EXIT_SUCCESS;
};

static int publish_gnssSetGeoFenceCurrentLocation(int argc, char** argv) {
  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_SET_GEO_FENCE_CURRENT_LOCATION));

  return EXIT_SUCCESS;
};


static int publish_gnssDelGeoFence(int argc, char** argv) {
  if (argc < 2) {
    shell.println("echo argument required");
    return -1;
  }

  int id = String(argv[1]).toInt();

  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_DEL_GEO_FENCE, (GnssEvent::GnssGeofenceId) id));

  return EXIT_SUCCESS;
};

static int publish_gnssDelGeoFenceCurrentLocation(int argc, char** argv) {
  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_DEL_GEO_FENCE_CURRENT_LOCATION));

  return EXIT_SUCCESS;
};

static int publish_gnssGeoFenceTransition(int argc, char** argv) {
  if (argc < 3) {
    shell.println("echo argument required");
    return -1;
  }

  int trans = String(argv[1]).toInt();
  int id = String(argv[2]).toInt();

  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_GEO_FENCE, (GnssEvent::GnssGeofenceTransistion) trans, (GnssEvent::GnssGeofenceId) id));

  return EXIT_SUCCESS;
};

static int publish_gnssStart(int argc, char** argv) {
  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_START));

  return EXIT_SUCCESS;
};

static int publish_gnssStop(int argc, char** argv) {
  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_STOP));

  return EXIT_SUCCESS;
};

static int publish_gnssError(int argc, char** argv) {
  eventManager.queueEvent(
      new GnssEvent(GnssEvent::GNSS_EVT_ERROR));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "gnss_module.h"

static int gnss_begin(int argc, char** argv) {
  GnssWrapper.begin();

  return EXIT_SUCCESS;
};

static int gnss_end(int argc, char** argv) {
  GnssWrapper.end();

  return EXIT_SUCCESS;
};
#endif

void setup_shell_gnss() {
  shell.addCommand(F("publish_gnssPosition f<lat> f<lng> f<speed> b<fix>"), publish_gnssPosition);
  shell.addCommand(F("publish_gnssSetGeoFence f<lat> f<lng> d<rad> d<id>"), publish_gnssSetGeoFence);
  shell.addCommand(F("publish_gnssSetGeoFenceCurrentLocation"), publish_gnssSetGeoFenceCurrentLocation);
  shell.addCommand(F("publish_gnssDelGeoFence d<id>"), publish_gnssDelGeoFence);
  shell.addCommand(F("publish_gnssDelGeoFenceCurrentLocation"), publish_gnssDelGeoFenceCurrentLocation);
  shell.addCommand(F("publish_gnssGeoFenceTransition d<trans> d<id>"), publish_gnssGeoFenceTransition);
  shell.addCommand(F("publish_gnssStart"), publish_gnssStart);
  shell.addCommand(F("publish_gnssStop"), publish_gnssStop);
  shell.addCommand(F("publish_gnssError"), publish_gnssError);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("gnss_begin"), gnss_begin);
  shell.addCommand(F("gnss_end"), gnss_end);      
#endif
}
