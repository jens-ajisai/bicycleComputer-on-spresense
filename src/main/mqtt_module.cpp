#include "mqtt_module.h"

#include <ArduinoJson.h>
#include <WString.h>
#include <fcntl.h>
#include <nuttx/mqueue.h>

#include "GlobalStatus.h"
#include "config.h"
#include "events/GlobalStatus_event.h"
#include "events/camera_sensor_event.h"
#include "events/gnss_module_event.h"
#include "lte_module.h"
#include "my_log.h"
#include "secrets.h"

#include "memutils/memory_manager/MemManager.h"
#include "memoryManager/config/mem_layout.h"

// paho disconnects with CLIENT_ERROR. It does not work well at all with AWS IOT server.
#define USE_AWS_INSTEAD_OF_PAHO

// BROKER_NAME moved to secrets.h
#define BROKER_PORT 8883
#define MQTT_CLIENTID "spresense"
#define ROOTCA_FILE "/mnt/sd0/cert/root-CA.crt"
#define CERT_FILE "/mnt/sd0/cert/spresense.cert.pem"
#define KEY_FILE "/mnt/sd0/cert/spresense.private.key"

char broker[] = BROKER_NAME;
int port = BROKER_PORT;
char clientid[] = MQTT_CLIENTID;
char rootCaFile[] = ROOTCA_FILE;
char certFile[] = CERT_FILE;
char keyFile[] = KEY_FILE;
int keepAliveInterval = (30);

// Do not forget to set configuration max message size!
#define QUEUE_MAXMSG 10
#define QUEUE_MSGSIZE sizeof(struct MqttEvent::mqttPacket)

pthread_t mqttPollThread = -1;
int mqttTaskThread = -1;

String mqttTopics[MqttEvent::NumOfMqttTopics] = {
    String("jens/feeds/spresense.gnss"),   String("jens/feeds/spresense.getImage"),
    String("jens/feeds/spresense.image"),  String("jens/feeds/spresense.getAudio"),
    String("jens/feeds/spresense.audio"),  String("jens/feeds/spresense.getStatus"),
    String("jens/feeds/spresense.status"), String("jens/feeds/spresense.antiTheft"),
};

MqttEvent::MqttEvent_topic topicToEnum(String topic) {
  for (int i = 0; i < MqttEvent::NumOfMqttTopics; i++) {
    if (topic.compareTo(mqttTopics[i]) == 0) {
      if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: topicToEnum %s -> %d", topic.c_str(), i);
      return (MqttEvent::MqttEvent_topic)i;
    }
  }
  if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: topicToEnum %s", topic);
  return MqttEvent::TopicNotFound;
}

#ifdef USE_AWS_INSTEAD_OF_PAHO
#include "mqtt_module_aws.h"
#else
#include "mqtt_module_paho.h"
#endif

int mqttPoll(int argc, FAR char* argv[]) {
  while (true) {
    uint32_t start = millis();
    if (DEBUG_MQTT) {
      Log.verboseln("MQTT_MODULE: mqttPoll ENTER");
    }

    // what is a good time? yield blocks sending for a few seconds ...
    sleep(20);
    yieldClient();

    if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: mqttPoll EXIT | %dms", millis() - start);
  }
}

int mqttTask(int argc, FAR char* argv[]) {
  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::mqttTask");

  initializeConnection();

  if (mqttPollThread < 0) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, CONFIG_MQTT_MODULE_POLL_STACK_SIZE);

    pthread_create(&mqttPollThread, &attr, (pthread_startroutine_t)mqttPoll, NULL);
    assert(mqttPollThread > 0);
    pthread_setname_np(mqttPollThread, "mqttPoll");

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_MQTT;
    pthread_setaffinity_np(mqttPollThread, sizeof(cpu_set_t), &cpuset);
#endif
  }

  struct mq_attr attr = {QUEUE_MAXMSG, QUEUE_MSGSIZE, 0, 0};
  mqd_t mqfd = mq_open(QUEUE_MQTT, O_RDWR | O_CREAT, 0666, &attr);
  if (mqfd == -1) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: mqttTask: mq_open failed %d", get_errno());
    return 1;
  } else {
    if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: mqttTask: opened message queue to listen.");
  }

  unsigned int prio;

  while (true) {
    struct MqttEvent::mqttPacket* packet = NULL;
    ssize_t nbytes = mq_receive(mqfd, (char*)&packet, QUEUE_MSGSIZE, &prio);
    if (DEBUG_MQTT)
      Log.verboseln("MQTT_MODULE: mqttTask: Received topic data of size: %d, %d", nbytes,
                    get_errno());

    if (nbytes >= 0) {
      if (DEBUG_MQTT && !packet->isBinary)
        Log.traceln("MQTT_MODULE: MqttModuleClass::sendMqttMessage topic = %s, message = %s",
                    mqttTopics[packet->topic].c_str(), (char*)packet->message.data());

      if (DEBUG_MQTT && packet->isBinary)
        Log.traceln(
            "MQTT_MODULE: MqttModuleClass::sendMqttMessage topic = %s, message = BINARY len=%d",
            mqttTopics[packet->topic].c_str(), packet->message.size());

      MqttModule.sendMqttMessage(mqttTopics[packet->topic].c_str(), packet->message.data(),
                                 packet->message.size());
      free(packet);
    } else {
      sleep(1);
    }
  }

  if (mq_close(mqfd) < 0) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: mqttTask: mq_close failed %d", get_errno());
  }
  return 0;
}

void MqttModuleClass::init() {
  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::init");
  if (mqttTaskThread < 0) {
    mqttTaskThread = task_create("mqttTaskThread", SCHED_PRIORITY_DEFAULT,
                                 CONFIG_MQTT_MODULE_STACK_SIZE, mqttTask, NULL);
    assert(mqttTaskThread > 0);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_MQTT;
    sched_setaffinity(mqttTaskThread, sizeof(cpu_set_t), &cpuset);
#endif
  }

  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::init complete");
}

void MqttModuleClass::deinit() {
  if (mqttPollThread) {
    pthread_cancel(mqttPollThread);
    mqttPollThread = -1;
  }
  if (mqttTaskThread) {
    disconnectClient();
    task_delete(mqttTaskThread);
    mqttTaskThread = -1;
  }
  mq_unlink(QUEUE_MQTT);
}

static void send_message(struct MqttEvent::mqttPacket* packet) {
  uint32_t start = millis();
  if (DEBUG_MQTT) {
    Log.verboseln("MQTT_MODULE: send_message ENTER");
  }

  // O_NONBLOCK to quickly fail and not get stuck if message queue is not yet created.
  // mqfd used instead of member variable initialized.
  mqd_t mqfd = mq_open(QUEUE_MQTT, O_WRONLY | O_NONBLOCK);
  if (mqfd == -1) {
    if (DEBUG_MQTT)
      Log.warningln("MQTT_MODULE: Failed opening message queue. Mqtt not yet connected? errno: %d",
                    get_errno());
    free(packet);
    return;
  } else {
    if (mq_send(mqfd, (char*)&packet, QUEUE_MSGSIZE, 1) < 0) {
      free(packet);
    }

    if (mq_close(mqfd) < 0) {
      if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: send_message(cloud): ERROR mq_close failed");
    }
  }
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: send_message EXIT | %dms", millis() - start);
}

void MqttModuleClass::handleGnss(GnssEvent* ev) {
  if (ev->getCommand() == GnssEvent::GNSS_EVT_POSITION) {
    DynamicJsonDocument doc(256);
    doc["lat"] = ev->getLat();
    doc["lng"] = ev->getLng();
    doc["speed"] = ev->getSpeed();
    doc["direction"] = ev->getDirection();
    String message;
    serializeJson(doc, message);
    MqttModule.publish(
        new MqttEvent(MqttEvent::MQTT_EVT_SEND, MqttEvent::MQTT_TOPIC_GNSS, message));
  }
}

void MqttModuleClass::handleMqtt(MqttEvent* ev) {
  if (ev->getCommand() == MqttEvent::MQTT_EVT_SEND) {
    if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::handleMqtt MQTT_EVT_SEND");
    send_message(ev->getPacket());
  } else if (ev->getCommand() == MqttEvent::MQTT_EVT_RECV) {
    if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::handleMqtt MQTT_EVT_RECV");

    if (ev->getTopic() == MqttEvent::MQTT_TOPIC_GET_STATUS) {
      MqttModule.publish(new MqttEvent(MqttEvent::MQTT_EVT_SEND, MqttEvent::MQTT_TOPIC_STATUS,
                                       GlobalStatus.getStatusJson()));
    }
  }
}

void MqttModuleClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt && ev->getConnState() >= requiresConn) {
      init();
    } else {
//      deinit();
    }
  }
}

void MqttModuleClass::eventHandler(Event* event) {
  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;

    case Event::kEventMqtt:
      handleMqtt(static_cast<MqttEvent*>(event));
      break;

    case Event::kEventGnss:
      handleGnss(static_cast<GnssEvent*>(event));
      break;

    default:
      break;
  }
}

bool MqttModuleClass::begin() {
  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::begin");
  bool ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventMqtt, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);

#ifndef USE_AWS_INSTEAD_OF_PAHO
  if (write_buf_mh.allocSeg(S0_MQTT_PAHO_BUF_POOL, MQTT_PAHO_BUFFER_SIZE)) {
    Log.errorln("Failed to allocate memory for mqtt write buffer");
    return 0;
  }
  if (read_buf_mh.allocSeg(S0_MQTT_PAHO_BUF_POOL, MQTT_PAHO_BUFFER_SIZE)) {
    Log.errorln("Failed to allocate memory for mqtt read buffer");
    return 0;
  }

  readbuf = (unsigned char*)read_buf_mh.getPa();
  writebuf = (unsigned char*)write_buf_mh.getPa();
#endif

  return ret;
}

bool MqttModuleClass::end(void) {
#ifndef USE_AWS_INSTEAD_OF_PAHO
  write_buf_mh.freeSeg();
  read_buf_mh.freeSeg();
#endif
  return eventManager.removeListener(this);
}

#define TRANSFER_CHUNCK_SIZE 1500

void MqttModuleClass::directCall_sendImage(CamImage& img) {
  Log.traceln("MQTT_MODULE: Send image of size %d", img.getImgBuffSize());

  uint32_t transactionLength = 0;
  uint8_t* pos = img.getImgBuff();

  for (uint32_t offset = 0; offset < img.getImgBuffSize(); offset += transactionLength) {
    transactionLength = (img.getImgBuffSize() - offset) > TRANSFER_CHUNCK_SIZE
                            ? TRANSFER_CHUNCK_SIZE
                            : (img.getImgBuffSize() - offset);

    MqttModule.sendMqttMessage(mqttTopics[MqttEvent::MQTT_TOPIC_IMAGE].c_str(), pos,
                               transactionLength);
    pos += transactionLength;
  }
}

MqttModuleClass MqttModule;
