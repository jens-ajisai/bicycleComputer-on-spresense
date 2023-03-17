from awscrt import mqtt
import sys
import threading
import time
from uuid import uuid4
import json

from awscrt import io, http, auth
from awsiot import mqtt_connection_builder, mqtt5_client_builder

from robot.api import logger

received_message = ""
received_topic = ""
received_all_event = threading.Event()

# Callback when connection is accidentally lost.
def on_connection_interrupted(connection, error, **kwargs):
    logger.warn("Connection interrupted. error: {}".format(error))

# Callback when an interrupted connection is re-established.
def on_connection_resumed(connection, return_code, session_present, **kwargs):
    logger.warn("Connection resumed. return_code: {} session_present: {}".format(return_code, session_present))

    if return_code == mqtt.ConnectReturnCode.ACCEPTED and not session_present:
        logger.warn("Session did not persist. Resubscribing to existing topics...")
        resubscribe_future, _ = connection.resubscribe_existing_topics()

        # Cannot synchronously wait for resubscribe result because we're on the connection's event-loop thread,
        # evaluate result with a callback instead.
        resubscribe_future.add_done_callback(on_resubscribe_complete)

def on_resubscribe_complete(resubscribe_future):
        resubscribe_results = resubscribe_future.result()
        logger.warn("Resubscribe results: {}".format(resubscribe_results))

        for topic, qos in resubscribe_results['topics']:
            if qos is None:
                sys.exit("Server rejected resubscribe to topic: {}".format(topic))


# Callback when the subscribed topic receives a message
def on_message_received(topic, payload, dup, qos, retain, **kwargs):
    logger.warn("Received message from topic '{}': {}".format(topic, payload))
    global received_topic
    global received_message
    received_topic = topic
    received_message = payload
    received_all_event.set()


class MQTTKeywords(object):
    mqtt_connection = None

    def __init__(self):
        pass

    def connect(self, broker, port, client_id, clean_session, ca, cert, private):
        credentials_provider = auth.AwsCredentialsProvider.new_default_chain()
        self.mqtt_connection = mqtt_connection_builder.mtls_from_path(
            endpoint=broker,
            port=port,
            cert_filepath=cert,
            pri_key_filepath=private,
            ca_filepath=ca,
            on_connection_interrupted=on_connection_interrupted,
            on_connection_resumed=on_connection_resumed,
            client_id=client_id,
            clean_session=clean_session,
            keep_alive_secs=30,
            proxy_options=None)


        connect_future = self.mqtt_connection.connect()

        # Future.result() waits until a result is available
        connect_future.result()
        logger.warn("Connected!")

    def publish(self, topic, message, qos=mqtt.QoS.AT_LEAST_ONCE):
        logger.warn("topic={}, messsage={}, qos={}".format(topic, message, qos))
        self.mqtt_connection.publish(
            topic=topic,
            payload=message,
            qos=qos)
        time.sleep(1)
        logger.warn("publish!")

    def subscribe(self, topic, qos=mqtt.QoS.AT_LEAST_ONCE):
        logger.warn("Subscribing to topic '{}'...".format(topic))
        subscribe_future, packet_id = self.mqtt_connection.subscribe(
            topic=topic,
            qos=qos,
            callback=on_message_received)

        subscribe_result = subscribe_future.result()
        logger.warn("Subscribed with {}".format(str(subscribe_result['qos'])))

    def get_message(self, timeout=5):
        received_all_event.wait(timeout)
        global received_message
        return received_message

    def disconnect(self):
        disconnect_future = self.mqtt_connection.disconnect()
        disconnect_future.result()

if __name__ == '__main__':
    mqttClient = MQTTKeywords()
    mqttClient.connect()
    mqttClient.publish()
    mqttClient.subscribe()

    print("received message '{}' from {}.".format(received_message, received_topic))

    mqttClient.disconnect()
