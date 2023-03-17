#pragma once

#include "EventManager.h"

#include <vector>

class MqttEvent : public Event {
 public:
  enum MqttEvent_topic {
    MQTT_TOPIC_GNSS,
    MQTT_TOPIC_GET_IMAGE,
    MQTT_TOPIC_IMAGE,
    MQTT_TOPIC_GET_AUDIO,
    MQTT_TOPIC_AUDIO,
    MQTT_TOPIC_GET_STATUS,
    MQTT_TOPIC_STATUS,
    MQTT_TOPIC_ANTI_THEFT,
    NumOfMqttTopics,
    TopicNotFound
  };

  enum MqttEventCommand { MQTT_EVT_SEND, MQTT_EVT_RECV };

  const char* getCommandString() {
    switch (mCommand) {
      case MQTT_EVT_SEND:
        return "SEND";
      case MQTT_EVT_RECV:
        return "RECV";
      default:
        return "Command Unknown";
    }
  }

  struct mqttPacket {
    MqttEvent_topic topic = MqttEvent_topic::TopicNotFound;
    bool isBinary = false;
    std::vector<uint8_t> message;
  };

  MqttEvent(MqttEventCommand cmd, MqttEvent_topic topic, uint8_t* data, size_t len,
            bool isBinary = true) {
    mType = Event::kEventMqtt;
    mCommand = cmd;
    mCommandString = getCommandString();

    packet = new mqttPacket;

    packet->topic = topic;
    packet->isBinary = isBinary;
    packet->message.assign(data, data + len);
  }

  MqttEvent(MqttEventCommand cmd, MqttEvent_topic topic, String message)
      : MqttEvent(cmd, topic, (uint8_t*)message.c_str(), (size_t)message.length(), false) {}

  MqttEvent_topic getTopic() { return packet->topic; }

  String getMessage() {
    if (packet->isBinary) {
      return String();
    } else {
      return String((char*)packet->message.data());
    }
  }

  struct mqttPacket* getPacket() { return packet; }

 private:
  struct mqttPacket* packet;
};
