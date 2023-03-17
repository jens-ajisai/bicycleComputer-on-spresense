#include "cloud_module.h"

#include <ArduinoJson.h>
#include <RTC.h>
#include <fcntl.h>
#include <nuttx/mqueue.h>

#include <string>

extern "C" {
#include "sslutils/sslutil.h"
}

#include "GlobalStatus.h"
#include "config.h"
#include "events/EventManager.h"
#include "events/GlobalStatus_event.h"
#include "events/gnss_module_event.h"
#include "events/lte_module_event.h"
#include "memoryManager/config/mem_layout.h"
#include "my_log.h"
#include "netutils/netlib.h"
#include "netutils/webclient.h"
#include "secrets.h"

// Increase CONFIG_WEBCLIENT_MAXHTTPLINE !
// +CXD56_DMAC_SPI4_RX_MAXSIZE=2064
// +CXD56_DMAC_SPI4_TX_MAXSIZE=2064

static const char apiKey[] = SECRET_YAHOO_API_KEY;

static const char ipRequest[] = "http://whatismyip.akamai.com/";

static const char getPathWeather[] =
    "https://map.yahooapis.jp/weather/V1/"
    "place?output=json&coordinates=%3.6f,%3.6f&appid=%s";
static const char getPathIceCream[] =
    "https://map.yahooapis.jp/search/local/V1/"
    "localSearch?lat=%3.6f&lon=%3.6f&dist=%d&results=1&detail=simple"
    "&output=json&query=%s&appid=%s";
static const char getPathDistance[] =
    "https://map.yahooapis.jp/dist/V1/"
    "distance?output=json&coordinates=%3.6f,%3.6f%%20%3.6f,%3.6f&appid=%s";

static const char* icecream = "%e3%82%a2%e3%82%a4%e3%82%b9%e3%82%af%e3%83%aa%e3%83%bc%e3%83%a0";

#define DEFAULT_DISTANCE 5

#define ROOTCA_FILE "/mnt/spif/cert/root-CA-yahoo.crt"

#define HTTP_OK 200

CloudModuleEvent::storeInfo currentStoreInfo = {String(), 0, 0, 0, 0};
double currentLat, currentLng = 0;

int cloudTaskThread = -1;

#define NET_BUFFER_SIZE 4096
static char* g_netIoBuffer;

static int sink_callback(FAR char** buffer, int offset, int datend, FAR int* buflen,
                         FAR void* arg) {
  std::string* response = (std::string*)arg;

  response->append(&((*buffer)[offset]), datend - offset);

  return 0;
}

static int httpsGetRequest(const char* url, std::string* response) {
  struct webclient_context ctx;
  webclient_set_defaults(&ctx);
  ctx.method = "GET";
  ctx.buffer = g_netIoBuffer;
  ctx.buflen = NET_BUFFER_SIZE;
  ctx.sink_callback = sink_callback;
  ctx.sink_callback_arg = response;
  ctx.url = url;

  struct sslutil_tls_context ssl_ctx;
  SSLUTIL_CTX_INIT(&ssl_ctx);
  SSLUTIL_CTX_SET_CAFILE(&ssl_ctx, ROOTCA_FILE);
  ctx.tls_ops = sslutil_webclient_tlsops();
  ctx.tls_ctx = (FAR void*)&ssl_ctx;

  int ret = webclient_perform(&ctx);
  if (ret != 0) {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: webclient_perform failed with %d\n", ret);
    return 0;
  }

  ctx.buffer[NET_BUFFER_SIZE - 1] = 0;
  if (DEBUG_CLOUD)
    Log.traceln("CLOUD_MODULE: STATUS=%d, RESPONSE=%s", ctx.http_status, response->c_str());
  return ctx.http_status;
}

// https://stackoverflow.com/questions/10198985/calculating-the-distance-between-2-latitudes-and-longitudes-that-are-saved-in-a
#include <math.h>
#define earthRadiusKm 6371.0

// This function converts decimal degrees to radians
double deg2rad(double deg) { return (deg * M_PI / 180); }

//  This function converts radians to decimal degrees
double rad2deg(double rad) { return (rad * 180 / M_PI); }

/**
 * Returns the distance between two points on the Earth.
 * Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The distance between the two points in kilometers
 */
double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  u = sin((lat2r - lat1r) / 2);
  v = sin((lon2r - lon1r) / 2);
  return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

/**
 * Returns the bearing between two points on the Earth.
 * Direct conversion from Python to c++ from
 * https://gis.stackexchange.com/questions/29239/calculate-bearing-between-two-decimal-gps-coordinates
 *
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The bearing between the two points in degrees
 */
double bearingEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, dLong, dPhi;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  dLong = lon2r - lon1r;
  dPhi = log(tan(lat2r / 2.0 + M_PI / 4.0) / tan(lat1r / 2.0 + M_PI / 4.0));
  if (abs(dLong) > M_PI) {
    if (dLong > 0.0) {
      dLong = -(2.0 * M_PI - dLong);
    } else {
      dLong = (2.0 * M_PI + dLong);
    }
  }
  return fmod((rad2deg(atan2(dLong, dPhi)) + 360.0), 360.0);
}

void CloudModuleClass::updateIceCreamDistance(double ownLat, double ownLng, double shopLat,
                                              double shopLng, CloudModuleEvent::storeInfo* info) {
  uint32_t start = millis();
  if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: updateIceCreamDistance ENTER");

  double distance = distanceEarth(ownLat, ownLng, shopLat, shopLng);
  double bearing = bearingEarth(ownLat, ownLng, shopLat, shopLng);

  if (DEBUG_CLOUD)
    Log.traceln("CLOUD_MODULE: (%D,%D) to (%D,%D) bearing = %D, distance = %D", ownLat, ownLng,
                shopLat, shopLng, bearing, distance);
  info->distance = distance;
  info->bearing = bearing + GlobalStatus.status.direction;

  if (DEBUG_CLOUD)
    Log.verboseln("CLOUD_MODULE: updateIceCreamDistance EXIT | %dms", millis() - start);
}

void CloudModuleClass::getIceCream(double lat, double lng, int dist, const char* query,
                                   CloudModuleEvent::storeInfo* info) {
  uint32_t start = millis();
  if (DEBUG_CLOUD) {
    if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: getIceCream ENTER");
  }
  char iceCreamRequest[256] = {0};
  sprintf(iceCreamRequest, getPathIceCream, lat, lng, dist, query, apiKey);
  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: %s", iceCreamRequest);

  std::string response;
  int statusCode = httpsGetRequest(iceCreamRequest, &response);

  if (statusCode == HTTP_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response.c_str());

    if ((int)doc["ResultInfo"]["Count"]) {
      const char* shopName = doc["Feature"][0]["Name"];
      if (DEBUG_CLOUD) Log.infoln("CLOUD_MODULE: Shop name:%s", shopName);

      const char* locationString = doc["Feature"][0]["Geometry"]["Coordinates"];
      String tmp = String(locationString);

      info->name = String(shopName);
      info->lat = tmp.substring(0, tmp.indexOf(',')).toFloat();
      info->lng = tmp.substring(tmp.indexOf(',') + 1, tmp.length()).toFloat();
    }
  } else {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: Errro during ice cream request: %d", statusCode);
  }
  if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: getIceCream EXIT | %dms", millis() - start);
}

void CloudModuleClass::getWeather(double lat, double lng, CloudModuleEvent::rainfallData* data) {
  uint32_t start = millis();
  if (DEBUG_CLOUD) {
    Log.verboseln("CLOUD_MODULE: getWeather ENTER");
  }
  char weatherRequest[132] = {0};
  sprintf(weatherRequest, getPathWeather, lng, lat, apiKey);
  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: %s", weatherRequest);

  std::string response;
  int statusCode = httpsGetRequest(weatherRequest, &response);

  if (statusCode == HTTP_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response.c_str());

    if ((int)doc["ResultInfo"]["Count"]) {
      for (int i = 0; i < 7; i++) {
        double t = doc["Feature"][0]["Property"]["WeatherList"]["Weather"][i]["Rainfall"];
        data->rainfallTotal += t;
      }
      if (DEBUG_CLOUD) Log.infoln("CLOUD_MODULE: Rainfall is %Dmm", data->rainfallTotal);
    }
  } else {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: Errro during weather request: %d", statusCode);
  }
  if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: getWeather EXIT | %dms", millis() - start);
}

uint32_t CloudModuleClass::getIP() {
  uint32_t start = millis();
  if (DEBUG_CLOUD) {
    Log.verboseln("CLOUD_MODULE: getIP ENTER");
  }

  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: %s", ipRequest);

  std::string response;
  int statusCode = httpsGetRequest(ipRequest, &response);

  if (statusCode == HTTP_OK) {
    CloudModule.publish(new LteEvent(LteEvent::LTE_EVT_IP, response.c_str()));
  } else {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: Errro during IP request: %d", statusCode);
  }
  if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: getIP EXIT | %dms", millis() - start);

  return 0;
}

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
uint32_t CloudModuleClass::getDistanceOnline(double lat0, double lng0, double lat1, double lng1) {
  uint32_t start = millis();
  if (DEBUG_CLOUD) {
    Log.verboseln("CLOUD_MODULE: getDistanceOnline ENTER");
  }
  char distanceRequest[256] = {0};
  sprintf(distanceRequest, getPathDistance, lng0, lat0, lng1, lat1, apiKey);
  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: %s", distanceRequest);

  std::string response;
  int statusCode = httpsGetRequest(distanceRequest, &response);

  if (statusCode == HTTP_OK) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, response.c_str());

    if ((int)doc["ResultInfo"]["Count"]) {
      double distance = doc["Feature"][0]["Geometry"]["Distance"];
      if (DEBUG_CLOUD) Log.infoln("CLOUD_MODULE: Distance is %Dm", distance * 1000);
      return (round(distance * 1000));
    }
  } else {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: Errro during distance request: %d", statusCode);
  }
  if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: getDistanceOnline EXIT | %dms", millis() - start);
  return -1;
}
#endif

#define QUEUE_MAXMSG 10
#define QUEUE_MSGSIZE sizeof(int)

int cloudTask(int argc, FAR char* argv[]) {
  uint32_t start = millis();
  if (DEBUG_CLOUD) {
    Log.verboseln("CLOUD_MODULE: cloudTask ENTER");
  }

  struct mq_attr attr = {QUEUE_MAXMSG, QUEUE_MSGSIZE, 0, 0};
  mqd_t mqfd = mq_open(QUEUE_CLOUD, O_RDWR | O_CREAT, 0666, &attr);
  if (mqfd == -1) {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: cloudTask: mq_open failed %d", get_errno());
    return 1;
  } else {
    if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: cloudTask: opened message queue to listen.");
  }

  CloudModule.publish(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_IP));
  CloudModule.publish(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_ICECREAM));
  CloudModule.publish(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_WEATHER));

  if (DEBUG_CLOUD) Log.verboseln("CLOUD_MODULE: cloudTask EXIT | %dms", millis() - start);

  int message;

  while (true) {
    message = -1;
    ssize_t nbytes = mq_receive(mqfd, (char*)&message, sizeof(message), 0);
    if (DEBUG_CLOUD)
      Log.verboseln("CLOUD_MODULE: cloudTask: Received message: %d bytes: %d", message, nbytes);

    if (nbytes >= 0) {
      if (message == CloudModuleEvent::CLOUDMODULE_EVT_GET_ICECREAM) {
        if (currentLat > 0 && currentLng > 0) {
          CloudModule.getIceCream(currentLat, currentLng, DEFAULT_DISTANCE, icecream,
                                  &currentStoreInfo);
          CloudModule.updateIceCreamDistance(currentLat, currentLng, currentStoreInfo.lat,
                                             currentStoreInfo.lng, &currentStoreInfo);
          CloudModule.publish(
              new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_ICECREAM, currentStoreInfo));
          CloudModule.publish(new GnssEvent(GnssEvent::GNSS_EVT_SET_GEO_FENCE, currentLat,
                                            currentLng, 5000,
                                            GnssEvent::GEOFENCE_ID_FOR_SHOP_UPDATE));
        }
      } else if (message == CloudModuleEvent::CLOUDMODULE_EVT_GET_WEATHER) {
        if (currentLat > 0 && currentLng > 0) {
          CloudModuleEvent::rainfallData data = {0, 0, 0, 0, 0, 0, 0, 0, 0};
          CloudModule.getWeather(currentLat, currentLng, &data);
          CloudModule.publish(
              new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_WEATHER, data.rainfallTotal));
        }
      } else if (message == CloudModuleEvent::CLOUDMODULE_EVT_GET_IP) {
        CloudModule.getIP();
      }
    } else {
      sleep(1);
    }
  }

  if (mq_close(mqfd) < 0) {
    if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: cloudTask: ERROR mq_close failed");
  }
  return 0;
}

void CloudModuleClass::init() {
  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: CloudModuleClass::initCloudModule");
  if (cloudTaskThread < 0) {
    cloudTaskThread = task_create("cloudTask", SCHED_PRIORITY_DEFAULT,
                                  CONFIG_CLOUD_MODULE_STACK_SIZE, cloudTask, NULL);
    assert(cloudTaskThread > 0);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_CLOUD;
    sched_setaffinity(cloudTaskThread, sizeof(cpu_set_t), &cpuset);
#endif
  }

  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: CloudModuleClass::initCloudModule complete");
}

void CloudModuleClass::deinit() {
  if (cloudTaskThread) {
    task_delete(cloudTaskThread);
    cloudTaskThread = -1;
  }
  // not yet tested!
  mq_unlink(QUEUE_CLOUD);
}

static void send_message(int message) {
  // O_NONBLOCK to quickly fail and not get stuck if message queue is not yet created.
  // Instead of member initialized.
  mqd_t mqfd = mq_open(QUEUE_CLOUD, O_WRONLY | O_NONBLOCK);
  if (mqfd == -1) {
    if (DEBUG_CLOUD)
      Log.warningln("CLOUD_MODULE: Failed opening message queue. Cloud not yet ready? errno: %d",
                    get_errno());
    return;
  } else {
    mq_send(mqfd, (char*)&message, sizeof(message), 1);
    if (mq_close(mqfd) < 0) {
      if (DEBUG_CLOUD) Log.errorln("CLOUD_MODULE: send_message(cloud): ERROR mq_close failed");
    }
  }
}

void CloudModuleClass::handleCloudModule(CloudModuleEvent* ev) {
  if (ev->getCommand() == CloudModuleEvent::CLOUDMODULE_EVT_GET_ICECREAM) {
    send_message(ev->getCommand());
  } else if (ev->getCommand() == CloudModuleEvent::CLOUDMODULE_EVT_GET_WEATHER) {
    send_message(ev->getCommand());
  } else if (ev->getCommand() == CloudModuleEvent::CLOUDMODULE_EVT_GET_IP) {
    send_message(ev->getCommand());
  }
}

void CloudModuleClass::handleGnss(GnssEvent* ev) {
  if (ev->getCommand() == GnssEvent::GNSS_EVT_GEO_FENCE) {
    if (ev->getGeoFenceId() == GnssEvent::GEOFENCE_ID_FOR_SHOP_UPDATE &&
        ev->getGeoFenceTransition() == GnssEvent::GEOFENCE_TRANSITION_EXIT) {
      CloudModule.publish(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_ICECREAM));
    }
  }
  if (ev->getCommand() == GnssEvent::GNSS_EVT_POSITION) {
    currentLat = ev->getLat();
    currentLng = ev->getLng();
    if (currentStoreInfo.lat + currentStoreInfo.lng > 0.0) {
      updateIceCreamDistance(ev->getLat(), ev->getLng(), currentStoreInfo.lat, currentStoreInfo.lng,
                             &currentStoreInfo);
      publish(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_ICECREAM, currentStoreInfo));
    }
  }
}

void CloudModuleClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt && ev->getConnState() >= requiresConn) {
      init();
    } else {
//      deinit();
    }
  }
}

void CloudModuleClass::eventHandler(Event* event) {
  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: CloudModuleClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;

    case Event::kEventGnss:
      handleGnss(static_cast<GnssEvent*>(event));
      break;

    case Event::kEventCloudModule:
      handleCloudModule(static_cast<CloudModuleEvent*>(event));
      break;

    default:
      break;
  }
}

bool CloudModuleClass::begin() {
  if (DEBUG_CLOUD) Log.traceln("CLOUD_MODULE: CloudModuleClass::begin");
  bool ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventCloudModule, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);

  if (net_mh.allocSeg(S0_NET_BUF_POOL, NET_BUFFER_SIZE)) {
    Log.errorln("Failed to allocate memory for mqtt read buffer");
    return 0;
  }

  g_netIoBuffer = (char*)net_mh.getPa();

  return ret;
}

bool CloudModuleClass::end(void) {
  net_mh.freeSeg();
  return eventManager.removeListener(this);
}

CloudModuleClass CloudModule;
