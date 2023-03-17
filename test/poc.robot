*** Settings ***
Library             Dialogs
Library             MQTTKeywords.py
Library             SerialLibrary    encoding=ascii

Suite Setup         Multiline Setup
Test Setup          Open and Wait until boot complete
Test Teardown       Close Port

Test Tags           testplayground


*** Variables ***
${SPRESENSE_SERIAL_PORT}    /dev/cu.SLAB_USBtoUART
${MQTT_ADDRESS}             __REPLACE_ME_WITH_THE_URL__.iot.us-east-1.amazonaws.com
${MQTT_PORT}                443
${MQTT_CLIENT}              robot-tester
${MQTT_CA}                  __REPLACE_ME_WITH_THE_PATH_TO_THE_CERTIFICATES__/connect_device_package/root-CA.crt
${MQTT_CERT}                __REPLACE_ME_WITH_THE_PATH_TO_THE_CERTIFICATES__/connect_device_package/test_robot.cert.pem
${MQTT_PRIVATE}             __REPLACE_ME_WITH_THE_PATH_TO_THE_CERTIFICATES__/connect_device_package/test_robot.private.key

${MQTT_TIMEOUT}             5
${MQTT_QOS}                 0
${MQTT_TOPIC_GNSS}          jens/feeds/spresense.gnss
${MQTT_TOPIC_GETIMAGE}      jens/feeds/spresense.getImage
${MQTT_TOPIC_IMAGE}         jens/feeds/spresense.image
${MQTT_TOPIC_GETAUDIO}      jens/feeds/spresense.getAudio
${MQTT_TOPIC_AUDIO}         jens/feeds/spresense.audio
${MQTT_TOPIC_GETSTATUS}     jens/feeds/spresense.getStatus
${MQTT_TOPIC_STATUS}        jens/feeds/spresense.status


*** Test Cases ***
Query Battery Level
    [Tags]  LLD-STATUS-004
    Send shell command    publish_globalStatusTestBatteryPercentage
    ${DATA2} =    Read Until    terminator=FINISHED TEST
    Should Match Regexp    ${DATA2}    Battery Percentage \\d+\\%

Query Memory Tiles Usage
    [Tags]  LLD-STATUS-001  LLD-STATUS-002  LLD-STATUS-003
    Send shell command    publish_globalStatusTestMemory
    ${DATA} =    Read Until    terminator=FINISHED TEST
    Should Match Regexp    ${DATA}    Memory tile usage: used:\\d+K / free:\\d+K \\(Largest free:\\d+K\\)
    Should Match Regexp    ${DATA}    SD usage: used:\\d+(M|G) / free:\\d+(M|G)
    Should Match Regexp    ${DATA}    RAM usage: used:\\d+B / free:\\d+B \\(Largest free:\\d+B\\)

Collect Status Information
    [Tags]  LLD-STATUS-005  LLD-STATUS-006
    Send shell command    publish_globalStatusTestMemory
    Send shell command    publish_globalStatusTestBatteryPercentage
    Send shell command    publish_uitftOn
    Send shell command    publish_gnssGeoFenceTransition 2 1
    Send shell command    publish_cycleHeartRateConnected
    Send shell command    publish_cycleHeartSawMobile
    Send shell command    publish_lteConnected 816316417
    Send shell command    publish_gnssPosition 35.681166768461296 119.76643039925418 12.4 2.1 1
    Send shell command    publish_globalStatusTestStatusJson
    ${DATA2} =    Read Until    terminator=}
    Should Match Regexp    ${DATA2}    \\{"batteryPercentage":80,"bleConnected":1,"phoneNearby":1,"lteConnected":1,"ipAddress":816316417,"lat":35.68117,"lng":119.76643,"speed":2.1,"direction":12.4,"geoFenceStatus":2,"displayOn":1,"firmwareVersion":1,"sdCardFreeMemory":"28G","heapFreeMemory":"738304"\\}
    Send shell command    publish_uitftOff
    Send shell command    publish_gnssGeoFenceTransition 1 2
    Send shell command    publish_lteDisconnected
    Send shell command    publish_cycleHeartRateDisconnected
    Send shell command    publish_cycleHeartLostMobile
    Send shell command    publish_globalStatusTestStatusJson
    ${DATA2} =    Read Until    terminator=}
    Should Match Regexp    ${DATA2}    \\{"batteryPercentage":80,"bleConnected":0,"phoneNearby":0,"lteConnected":0,"ipAddress":0,"lat":35.68117,"lng":119.76643,"speed":2.1,"direction":12.4,"geoFenceStatus":1,"displayOn":0,"firmwareVersion":1,"sdCardFreeMemory":"28G","heapFreeMemory":"738304"\\}

Save GNSS Information to SD Card on Update
    [Tags]  LLD-STATUS-008
    Send shell command    publish_gnssPosition 35.681166768461296 119.76643039925418 12.4 2.1 1
    Read Until    terminator=topic = jens/feeds/spresense.status    timeout=80
    ${MESSAGE} =    Get Message
    Should Match Regexp    ${MESSAGE}    \\{"batteryPercentage":\\d+,"bleConnected":\\d+,"phoneNearby":\\d+,"lteConnected":\\d+,"ipAddress":,"lat":\\d+.\\d+,"lng":\\d+.\\d+,"speed":0.0,"direction":\\d+.\\d+,"geoFenceStatus":\\d+,"displayOn":\\d+,"firmwareVersion":\\d+,"sdCardFreeMemory":"\\d+G","heapFreeMemory":"\\d+"\\}
    Disconnect

Save Status Information to SD Card
    [Tags]  LLD-STATUS-008  TODO
    Send shell command    publish_globalStatusTestStatusJson
    ${DATA2} =    Read Until    terminator=FINISHED TEST
    TODO set all the status information!!!
    Should Match Regexp    ${DATA2}    {"batteryPercentage": 0,"bleConnected": 0,"phoneNearby": 0,"lteConnected": 0,"ipAddress": ,"lat": 0.000000,"lng": 0.000000,"speed": 0.000000,"direction": 0.000000,"geoFenceStatus": 0,"displayOn": 0,"firmwareVersion": 1,"sdCardFreeMemory": "heapFreeMemory": }


Send Status Information with MQTT Periodically
    [Tags]  LLD-STATUS-007  TODO
    Send shell command    publish_globalStatusTestStatusJson
    ${DATA2} =    Read Until    terminator=FINISHED TEST
    TODO set all the status information!!!
    Should Match Regexp    ${DATA2}    {"batteryPercentage": 0,"bleConnected": 0,"phoneNearby": 0,"lteConnected": 0,"ipAddress": ,"lat": 0.000000,"lng": 0.000000,"speed": 0.000000,"direction": 0.000000,"geoFenceStatus": 0,"displayOn": 0,"firmwareVersion": 1,"sdCardFreeMemory": "heapFreeMemory": }

00:00:15.217 TRACE Wrote file[97]: gnss/GNSS        11


On IMU_EVT_MOVED Application goes into ApplicationSystemStart
    Send shell command    publish_imuMoved
    ${DATA} =    Read Until    terminator=AppManagerClass::setAppState 1
    Should Contain    ${DATA}    setAppState 1


Send GNSS Information with MQTT on Update
    [Tags]  LLD-STATUS-008  TODO
    Connect    ${MQTT_ADDRESS}    port=${MQTT_PORT}  client_id=${MQTT_CLIENT}    clean_session=False  ca=${MQTT_CA}  cert=${MQTT_CERT}  private=${MQTT_PRIVATE}
    Subscribe    topic=${MQTT_TOPIC_STATUS}
    Read Until    terminator=topic = jens/feeds/spresense.status    timeout=80
    ${MESSAGE} =    Get Message
    Should Match Regexp    ${MESSAGE}    \\{"batteryPercentage":\\d+,"bleConnected":\\d+,"phoneNearby":\\d+,"lteConnected":\\d+,"ipAddress":,"lat":\\d+.\\d+,"lng":\\d+.\\d+,"speed":0.0,"direction":\\d+.\\d+,"geoFenceStatus":\\d+,"displayOn":\\d+,"firmwareVersion":\\d+,"sdCardFreeMemory":"\\d+G","heapFreeMemory":"\\d+"\\}
    Disconnect





Send Status Information with MQTT Periodically
    [Tags]  LLD-STATUS-007  NEW
    Connect    ${MQTT_ADDRESS}    port=${MQTT_PORT}  client_id=${MQTT_CLIENT}    clean_session=False  ca=${MQTT_CA}  cert=${MQTT_CERT}  private=${MQTT_PRIVATE}
    Subscribe    topic=${MQTT_TOPIC_GNSS}
    Send shell command    publish_imuMoved
    Read Until    terminator=MqttModuleClass::init complete
    ${MESSAGE} =    Get Message
    Should Match Regexp    ${MESSAGE}    Battery Percentage \\d+\\%
    Disconnect


Send GNSS Information with MQTT on Update
    [Tags]  LLD-STATUS-008  TODO
    Connect    ${MQTT_ADDRESS}    port=${MQTT_PORT}  client_id=${MQTT_CLIENT}    clean_session=False  ca=${MQTT_CA}  cert=${MQTT_CERT}  private=${MQTT_PRIVATE}
    Subscribe    topic=${MQTT_TOPIC_STATUS}
    Read Until    terminator=topic = jens/feeds/spresense.status    timeout=80
    ${MESSAGE} =    Get Message
    Should Match Regexp    ${MESSAGE}    \\{"batteryPercentage":\\d+,"bleConnected":\\d+,"phoneNearby":\\d+,"lteConnected":\\d+,"ipAddress":,"lat":\\d+.\\d+,"lng":\\d+.\\d+,"speed":0.0,"direction":\\d+.\\d+,"geoFenceStatus":\\d+,"displayOn":\\d+,"firmwareVersion":\\d+,"sdCardFreeMemory":"\\d+G","heapFreeMemory":"\\d+"\\}
    Disconnect



Publish and Verify Response
    [Tags]  MQTT
    [Setup]    Connect    ${MQTT_ADDRESS}    port=${MQTT_PORT}  client_id=${MQTT_CLIENT}    clean_session=False  ca=${MQTT_CA}  cert=${MQTT_CERT}  private=${MQTT_PRIVATE}
    Publish    topic=${MQTT_TOPIC_GETSTATUS}    message=Hello   qos=${MQTT_QOS}
    Subscribe    topic=${MQTT_TOPIC_GETSTATUS}    qos=${MQTT_QOS}
    Sleep   1s
    [Teardown]    Disconnect

Execute Manual Step
    Execute Manual Step    Please check that the GPS coordinates are shown on the display    default_error=PASS

# https://robotframework.org/robotframework/#user-guide
# https://github.com/robotframework/QuickStartGuide/blob/master/QuickStart.rst

# robotidy    common.robot
# robotidy    --diff --no-overwrite common.robot

# pip install robotframework-tidy
# pip install robotframework-seriallibrary
# pip install robotframework-mqttlibrary
# brew install python-tk


*** Keywords ***
Multiline Setup
    Spresense is connected to USB
    Configure Spresense Serial console

Spresense is connected to USB
    ${PORTS} =    List Com Ports
    ${PORT_NAMES} =    List Com Port Names
    Com Port Should Exist Regexp    ${SPRESENSE_SERIAL_PORT}
    Log to console    Available PORTS are ${PORT_NAMES}

Configure Spresense Serial console
    Add Port    /dev/cu.SLAB_USBtoUART
    Set Port Parameter    baudrate    115200
    Set Port Parameter    timeout    10
    ${PORT} =    Get Current Port Locator

Open and Wait until boot complete
    Open Port
    Read Until    terminator=SHELL IS READY

Send shell command
    [Arguments]    ${command}
    Write Data    ${command}\r\n
    Sleep   0.3s    Sending too many shell commands at once is not supported. Give time to process the command.
