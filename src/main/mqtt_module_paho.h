#include "MQTTClient.h"

QoS subscribeQos = QOS0;
QoS sendQos = QOS0;

MQTTSocket n;
MQTTClient client;

#define MQTT_BUFFER_SIZE 360

unsigned char* writebuf;
unsigned char* readbuf;

void onMqttMessage(MessageData* md) {
  uint32_t start = millis();
  if (DEBUG_MQTT) {
    Log.verboseln("MQTT_MODULE: onMqttMessage ENTER");
  }

  MQTTMessage* message = md->message;

  if (md->topicName->lenstring.len > 100) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: Mqtt topic too long");
    return;
  }
  if (message->payloadlen > 240) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: Mqtt message too long");
    return;
  }

  char topicStr[100] = {0};
  memcpy(topicStr, md->topicName->lenstring.data, md->topicName->lenstring.len);

  char messageStr[240] = {0};
  memcpy(messageStr, message->payload, message->payloadlen);

  if (DEBUG_MQTT)
    Log.traceln("MQTT_MODULE: onMqttMessage topic = %s, message = %s", topicStr, messageStr);

  MqttEvent::MqttEvent_topic topic = topicToEnum(String(topicStr));
  MqttModule.publish(new MqttEvent(MqttEvent::MQTT_EVT_RECV, topic, String(messageStr)));
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: onMqttMessage EXIT | %dms", millis() - start);
}

void MqttModuleClass::sendMqttMessage(const char* topic, const char* message) {
  uint32_t start = millis();
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: sendMqttMessage ENTER");

  if (DEBUG_MQTT)
    Log.traceln(
        "MQTT_MODULE: MqttModuleClass::sendMqttMessage topic = %s, message = %s, connected? %d",
        topic, message, MQTTIsConnected(&client));

  MQTTMessage pubmsg;

  memset(&pubmsg, 0, sizeof(pubmsg));
  pubmsg.qos = sendQos;
  pubmsg.retained = 0;
  pubmsg.dup = 0;
  pubmsg.payload = (void*)message;
  pubmsg.payloadlen = strlen(message);

  MQTTPublish(&client, topic, &pubmsg);

  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: sendMqttMessage EXIT | %dms", millis() - start);
}

void initializeConnection() {
  uint32_t start = millis();
  if (DEBUG_MQTT) {
    Log.verboseln("MQTT_MODULE: initializeConnection ENTER");
  }

  // first do a cleanup in case of re-initialize
  MQTTDisconnect(&client);
  MQTTSocketDisconnect(&n);
  MQTTSocketFin(&n);

  int use_ssl = 1;
  n.pRootCALocation = rootCaFile;
  n.pDeviceCertLocation = certFile;
  n.pDevicePrivateKeyLocation = keyFile;

  MQTTSocketInit(&n, use_ssl);
  MQTTSocketConnect(&n, broker, port);
  MQTTClientInit(&client, &n, 10000, writebuf, MQTT_BUFFER_SIZE, readbuf, MQTT_BUFFER_SIZE);

  char randomClientId[32];
  rand(); // for some reason the first call to rand() returns 0
  snprintf(randomClientId, 32, "%s-%d", clientid, rand());

  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.willFlag = 0;
  data.MQTTVersion = 5;
  data.clientID.cstring = randomClientId;
  data.username.cstring = NULL;
  data.password.cstring = NULL;

  data.keepAliveInterval = keepAliveInterval;
  data.cleansession = 1;
  if (DEBUG_MQTT)
    Log.traceln("MQTT_MODULE: Connecting to %s %d ID %s", broker, port, randomClientId);

  if (MQTTConnect(&client, &data) != SUCCESS) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: MQTT connection failed!");
    return;
  }

  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: You're connected to the MQTT broker!");

  MQTTSubscribe(&client, mqttTopics[MqttEvent::MQTT_TOPIC_GET_AUDIO].c_str(), subscribeQos,
                onMqttMessage);
  MQTTSubscribe(&client, mqttTopics[MqttEvent::MQTT_TOPIC_GET_IMAGE].c_str(), subscribeQos,
                onMqttMessage);
  MQTTSubscribe(&client, mqttTopics[MqttEvent::MQTT_TOPIC_GET_STATUS].c_str(), subscribeQos,
                onMqttMessage);

  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: initializeConnection EXIT | %dms", millis() - start);
}

void yieldClient() {
  if (!MQTTIsConnected(&client)) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: Mqtt client is disconnected. Reconnect");
    initializeConnection();
  }
}

void disconnectClient() {
  MQTTDisconnect(&client);
  MQTTSocketDisconnect(&n);
  MQTTSocketFin(&n);
}
