#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_version.h"

AWS_IoT_Client client;

void waitForClientIdle() {
  ClientState state = aws_iot_mqtt_get_client_state(&client);
  while (CLIENT_STATE_CONNECTED_IDLE != state) {
    Log.traceln("ClientState is %d", state);
    usleep(100 * 1000);
    state = aws_iot_mqtt_get_client_state(&client);
  }
}

void onMqttMessage(AWS_IoT_Client* pClient, char* topicName, uint16_t topicNameLen,
                   IoT_Publish_Message_Params* params, void* pData) {
  uint32_t start = millis();
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: onMqttMessage ENTER");

  if (DEBUG_MQTT)
    Log.traceln("MQTT_MODULE: onMqttMessage topic = %s, message = %s", topicName, params->payload);


  // The topic is not null terminated.
  if (topicNameLen > 64) {
    if (DEBUG_MQTT) Log.errorln("MQTT_MODULE: Mqtt topic too long");
    return;
  }

  char topicStr[64] = {0};
  memcpy(topicStr, topicName, topicNameLen);

  MqttEvent::MqttEvent_topic topic = topicToEnum(String(topicStr));
  MqttModule.publish(new MqttEvent(MqttEvent::MQTT_EVT_RECV, topic, String((char*)params->payload)));
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: onMqttMessage EXIT | %dms", millis() - start);
}



void MqttModuleClass::sendMqttMessage(const char* topic, uint8_t* data, size_t size) {
  IoT_Error_t rc;

  uint32_t start = millis();
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: sendMqttMessage ENTER");

  if (DEBUG_MQTT) Log.traceln("MQTT_MODULE: MqttModuleClass::sendMqttMessage topic = %s", topic);

  IoT_Publish_Message_Params paramsQOS0;

  paramsQOS0.qos = QOS0;
  paramsQOS0.payload = (void*)data;
  paramsQOS0.payloadLen = size;
  paramsQOS0.isRetained = 0;

  waitForClientIdle();
  rc = aws_iot_mqtt_publish(&client, topic, strlen(topic), &paramsQOS0);

  if (rc == MQTT_REQUEST_TIMEOUT_ERROR) {
    Log.errorln("QOS1 publish ack not received.\n");
    return;
  }

  if (rc != SUCCESS) {
    Log.errorln("An error occurred of the publish : %d.\n", rc);
    return;
  }

  if (DEBUG_MQTT)
    Log.verboseln("MQTT_MODULE: sendMqttMessage EXIT %d | %dms", rc, millis() - start);
}

static void disconnectCallbackHandler(AWS_IoT_Client* pClient, void* data) {
  Log.warningln("MQTT_MODULE: MQTT Disconnect");
  IoT_Error_t rc = FAILURE;

  if (NULL == pClient) {
    return;
  }

  IOT_UNUSED(data);

  if (aws_iot_is_autoreconnect_enabled(pClient)) {
    Log.traceln("MQTT_MODULE: Auto Reconnect is enabled, Reconnecting attempt will start now");
  } else {
    Log.warningln("MQTT_MODULE: Auto Reconnect not enabled. Starting manual reconnect...");
    rc = aws_iot_mqtt_attempt_reconnect(pClient);
    if (NETWORK_RECONNECTED == rc) {
      Log.warningln("MQTT_MODULE: Manual Reconnect Successful");
    } else {
      Log.warningln("MQTT_MODULE: Manual Reconnect Failed - %d", rc);
    }
  }
}

void initializeConnection() {
  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: initializeConnectionAWS ENTER");

  IoT_Error_t rc = FAILURE;

  IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

  mqttInitParams.enableAutoReconnect = false;  // We enable this later below
  mqttInitParams.pHostURL = broker;
  mqttInitParams.port = port;
  mqttInitParams.pRootCALocation = rootCaFile;
  mqttInitParams.pDeviceCertLocation = certFile;
  mqttInitParams.pDevicePrivateKeyLocation = keyFile;
  mqttInitParams.mqttCommandTimeout_ms = 20000;
  mqttInitParams.tlsHandshakeTimeout_ms = 10000;
  mqttInitParams.isSSLHostnameVerify = true;
  mqttInitParams.disconnectHandler = disconnectCallbackHandler;
  mqttInitParams.disconnectHandlerData = NULL;

  rc = aws_iot_mqtt_init(&client, &mqttInitParams);
  if (SUCCESS != rc) {
    Log.errorln("aws_iot_mqtt_init returned error : %d ", rc);
    return;
  }

  char randomClientId[32];
  rand();  // for some reason the first call to rand() returns 0
  snprintf(randomClientId, 32, "%s-%d", clientid, rand());

  connectParams.keepAliveIntervalInSec = 600;
  connectParams.isCleanSession = true;
  connectParams.MQTTVersion = MQTT_3_1_1;
  connectParams.pClientID = randomClientId;
  connectParams.clientIDLen = (uint16_t)strlen(randomClientId);
  connectParams.isWillMsgPresent = false;

  Log.traceln("MQTT_MODULE: Connecting...");
  rc = aws_iot_mqtt_connect(&client, &connectParams);
  if (SUCCESS != rc) {
    Log.errorln("Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
    return;
  }

  rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
  if (SUCCESS != rc) {
    Log.errorln("MQTT_MODULE: Unable to set Auto Reconnect to true - %d", rc);
    return;
  }

  Log.traceln("MQTT_MODULE: Subscribing...");
  rc = aws_iot_mqtt_subscribe(&client, mqttTopics[MqttEvent::MQTT_TOPIC_GET_AUDIO].c_str(),
                              strlen(mqttTopics[MqttEvent::MQTT_TOPIC_GET_AUDIO].c_str()), QOS0,
                              onMqttMessage, NULL);
  if (SUCCESS != rc) {
    Log.errorln("MQTT_MODULE: Error subscribing : %d ", rc);
    return;
  }

  rc = aws_iot_mqtt_subscribe(&client, mqttTopics[MqttEvent::MQTT_TOPIC_GET_IMAGE].c_str(),
                              strlen(mqttTopics[MqttEvent::MQTT_TOPIC_GET_IMAGE].c_str()), QOS0,
                              onMqttMessage, NULL);
  if (SUCCESS != rc) {
    Log.errorln("MQTT_MODULE: Error subscribing : %d ", rc);
    return;
  }

  rc = aws_iot_mqtt_subscribe(&client, mqttTopics[MqttEvent::MQTT_TOPIC_GET_STATUS].c_str(),
                              strlen(mqttTopics[MqttEvent::MQTT_TOPIC_GET_STATUS].c_str()), QOS0,
                              onMqttMessage, NULL);
  if (SUCCESS != rc) {
    Log.errorln("MQTT_MODULE: Error subscribing : %d ", rc);
    return;
  }

  if (DEBUG_MQTT) Log.verboseln("MQTT_MODULE: initializeConnectionAWS EXIT %d", rc);
}

void yieldClient() {
  waitForClientIdle();
  IoT_Error_t rc = aws_iot_mqtt_yield(&client, 100);
  if (SUCCESS != rc) {
    Log.errorln("aws_iot_mqtt_yield returned error : %d ", rc);
  }
}

void disconnectClient() { aws_iot_mqtt_disconnect(&client); }

void initializeClientBuffer() {}