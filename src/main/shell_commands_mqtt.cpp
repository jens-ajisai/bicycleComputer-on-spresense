#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/mqtt_module_event.h"
#include "config.h"

static int publish_mqttSend(int argc, char** argv) {
  if (argc < 3) {
    shell.println("echo argument required");
    return -1;
  }

  int topic = String(argv[1]).toInt();
  String message = String(argv[2]);

  if (topic >= MqttEvent::NumOfMqttTopics) {
    shell.println("invalid mqtt topic id");
    return -1;
  }

  eventManager.queueEvent(new MqttEvent(MqttEvent::MQTT_EVT_SEND,
                                   (MqttEvent::MqttEvent_topic)topic, message));

  return EXIT_SUCCESS;
};

static int publish_mqttRecv(int argc, char** argv) {
  if (argc < 3) {
    shell.println("echo argument required");
    return -1;
  }

  int topic = String(argv[1]).toInt();
  String message = String(argv[2]);

  if (topic >= MqttEvent::NumOfMqttTopics) {
    shell.println("invalid mqtt topic id");
    return -1;
  }

  eventManager.queueEvent(new MqttEvent(MqttEvent::MQTT_EVT_RECV,
                                   (MqttEvent::MqttEvent_topic)topic, message));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "mqtt_module.h"

static int mqtt_begin(int argc, char** argv) {
  MqttModule.begin();

  return EXIT_SUCCESS;
};

static int mqtt_end(int argc, char** argv) {
  MqttModule.end();

  return EXIT_SUCCESS;
};
#endif



void setup_shell_mqtt() {
  shell.addCommand(F("publish_mqttSend d<topic> s<msg>"), publish_mqttSend);
  shell.addCommand(F("publish_mqttRecv d<topic> s<msg>"), publish_mqttRecv);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("mqtt_begin"), mqtt_begin);
  shell.addCommand(F("mqtt_end"), mqtt_end);   
#endif
}
