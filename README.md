# Bike computer
# Story
## Project Outline
This project aims to build a bicycle computer using the Sony Spresense main board in combination with the LTE extension board and various other peripherals.
The main features are
* Capture a low-resolution video stream and show it on a display with an option to take high-resolution images for storage on an SD card
* Capture mono audio and encode it with the OPUS codec packaged with the OGG container format for high compression to send it via an LTE-M connection or record it to an SD card.
* Track the location via GNSS and combine the location with weather data and points of interest (POI) data received from cloud services via the LTE connection.
* Connect to bicycle sensors (currently heart rate) via Bluetooth Low Energy, show the data on the display and record it.
* Remote access to the camera, an audio live stream and various data incl. location via MQTT
* Theft detection and notification via GNSS geofence, acceleration sensor and smartphone nearby monitoring

## Motivation
After doing a project with nRF52 / nRF91-based MCUs on Zephyr, I wanted to see how other MCUs with their development environments work to extend my skills and have a comparison.

My attention was put to the Spresense board after reading about where it is in use (in space!) and the book "Starting low power edge AI with SPRESENSE" by Ota Yoshinori  [O'Reilly Japan - SPRESENSEではじめるローパワーエッジAI](https://www.oreilly.co.jp/books/9784873119670/) 

I tried to build a battery-based birdhouse camera with an Arducam Mini 2MP Plus before. Unfortunately, power consumption (leakage via SPI pins when using an NPN transistor) and reliability (randomly images are tinted getting worse with each new picture) were improvable (they have a new cam by now, cannot say what went wrong, I made intensive research on it), I wanted to give it a try to the Spresense board as it is advertised as being very low power. It also has camera capabilities and Sony is doing cameras very well.

After thinking about what kind of project would use the unique features of Spresense best, I came to the bicycle computer which is something I wanted to build for many years but did not find the fitting hardware yet. There seems to be not anything similar you can buy out there which made it more valuable to start.
I set myself the goal to try to get the most out of what Spresense offers as (unique) features. Also, I put myself as the guiding constraint to do an Arduino Environment based project.

Online research on dash cams for cars revealed that the camera quality at low light and lenses for a wide recording are differentiating selling points. The camera is a main part of the bicycle computer, but in this category, I cannot win. There are only 2 cameras to select from. Therefore I want to have cloud connectivity and connectivity to bicycle sensors as my differentiating points. Smart logic for theft detection and accident detection specific to bicycles is another point.

## Why use Sony Spresense?
* Most important is a certain amount of computing power (and multiple cores) and memory to handle video & audio paired with low power consumption. This is a rather unique combination taking the onboard video, audio and GNSS into account.
* The second is active support. The documentation is very well written and the Arduino environment with the rich examples that all worked out of the box is a great start. The BSP is still updated and there is help in the forum. As a plus, full support on the underlying RTOS Nuttx is available through an easy recompilation of the Arduino SDK.

A very good comparison can be found in the documentation, so I will comment here only briefly. [ [Spresense Hardware Documents](https://developer.sony.com/develop/spresense/docs/hw_docs_en.html#_spresense_hardware_comparison) ]
* A Raspberry Pi (even the zero) consumes too much power for a battery, other MCUs are limited in power or memory or have no camera/audio interface prepared.
* Given the feature set of video, audio, and GNSS(, cellular, Bluetooth, display, acceleration, magnetometer) the comparison target actually could be a smartphone. It was a valid option for this project.

What came to my mind was that
* With Spresense I can have a modular approach and freely add components
* likely a lower energy consumption can be achieved
* it is a different "world" of development between an app and an MCU IoT project.

## Challenges encountered
**No connectivity like Wifi, BLE, Cellular, Ethernet or similar** is available on the Spresense main board. LTE-M is well supported by the official LTE extension board. So that was quite easy. For BLE, which I integrated, there was no option available. There is a BLE extension board. However (1) it already entered end-of-life, (2) there is no open documentation and (3) the only sample does only UART over BLE. I created my extension board with firmware and slightly modified the ArduinoBLE library to work with it.

**constraints on the memory**

Although 1.5 MB RAM and 8 MB Flash sound like a lot of memory for an MCU, it is rather challenging due to some.
By end of 2022, a new BLE extension board came out together with BLE support in NuttX integrating the NRF SDK (That is the old deprecated SDK, not the current Zephyr OS-based NRF Connect SDK)

* The feature that the whole program is copied to RAM to be executed from there reduces the available memory by the size of the program (ROM) size. I am using lvgl. A proper configuration to not compile unused features saved a lot of memory. Still, I have some icon images inside my program, which I reduced to 2-bit black and white images to save memory. What is still not solved is the memory requirement for the font. I want to display the name of the ice cream shop nearby. I have no control over which letters I need. As the name is in Japanese, there are potentially about 2000 letters (characters) that I need to prepare. The solution would be to save the glyph information to flash and implement the getter functions.
* Regarding the font or icons, direct access to Flash is restricted due to “security reasons”. So there is no such Flash helper feature available for Arduino.
* Sub cores require at least 1 memory tile (128KB) if not 2 memory tiles to run. A lot of memory gets "lost" if you want to distribute your modules over all 6 cores with ASMP. Some features like audio (similar to neural networks) need 2 tiles for the mp3 encoder and another 2 tiles for memory buffers. That is already 512KB. Then you need also buffers on the main cores which adds up to a lot of memory. Lowering audio quality does not reduce this memory consumption. I could reduce 1 tile by setting the Define MEMORY_UTIL_TINY. I reduced to 1 tile by using a 48 KHz sampling rate and by encoding with the OPUS codec in my program, not by DSPs. For mono audio, I need far less than 128 KB of buffers, so I reserve the remaining memory for the buffers of my program for network, audio and rendering so that this memory is not lost.
	* Update: I switched the ASMP feature off and use 48kHz without resampling and without any DSP. Still the audio features need message buffers. I specified memory areas for several buffers I use in the application and request this memory via the Memory manager.
* The camera needs much memory for high-quality images. Adjusting the resolution, JPEG quality and jpeg buffer size via the jpgbufsize_divisor is key here.

**Parallel execution of multiple modules needs some precaution**. 

Nuttx enables you to run threads. However, there are at least 2 pitfalls.
* The Arduino delay() function (or sometimes a busy loop using millis()) which is also used in several libraries fully blocks all threads. The spresense SDK did not map the delay() function to sleep() which would allow execution of another thread while waiting. This wastes performance, time and energy. So I always used sleep and modify libraries.
* The Arduino GNSS library uses Nuttx signals to notify a GNSS ready. This signal aborts all waiting polls for file descriptors and all sleep over all threads. I commented this signal out and polled myself because I know I set it to a position every second. The correct solution is to set the signal mask correctly for each thread and not inherit from the main thread that calls GNSS.begin().
* I introduced a definition CONFIG_REMOVE_NICE_TO_HAVE. I remove non-core functionality to save memory.

Going **multi-core is complex**.

The main usage for ASMP on Spresense is to offload computationally demanding algorithms like audio signal processing or neural network computation. ASMP cannot be used for nearly all other Spresense features because only CPU 0 can communicate to the System and GNSS CPUs. This even includes access to flash memory. So there is no execution from flash possible.
* Activating SMP (which means automatic scheduling over all cores by Nuttx) leads to frequent random crashes. After weeks of debugging and research, I finally managed to use the SMP feature. The big difference is if GNSS is on or not. Without GNSS, SMP works well because when communicating to the system CPU, the driver switches automatically the task to CPU core 0. Regarding GNSS, see the GNSS module chapter for details.
	* I can see that there are many fixes for the stability of SMP over the last few years.  Some are merged in January 2023. Some tasks in the NuttX Github about SMP are still open, so take care you have all required fixes merged in. I hope they will get into a future release of the Spresense SDK.

**A slow refresh rate of the camera stream to the display**

* Due to the slower speed of SPI3 on the LTE extension board and because TFT driver libraries send pixels one by one to stay compatible with other Arduino-based MCUs, the refresh rate is so slow that you can see how the screen draws line by line. Movements to the side show a tearing effect. By using the SPI5 on the Spresense board, enabling DMA support and pushing all pixels at once, the refresh rate increased by factor 10, which means no tearing and no line drawing is visible anymore.

**Transferring large data to the cloud** (images)

* It seems like the LTE library can only send about 2000 bytes at a time. I iterate over the data and split it into chunks. The configuration CXD56_DMAC_SPI4_TX_MAXSIZE is related here. It needs to be raised. I could test that AWS IoT accepts a 200KB image which was sent with roughly 16KB/s.

Setting up an **audio streaming server**

* The main issue was that my LTE-M SIM card contract does not support remote access. There is a NAT (Network Address Translation) in between that does not allow reaching the device from outside. You can only initiate connections from the device. This feature would have cost me more than 20 times my monthly fee to activate. I did not do that. I assume that is the reason there is no webserver example available for Spresense except one in Spresense SDK (examples/multi_webcamera) using socket API (very low level). You know that remote access is not supported when getIPAddress() returns a different address than asking whatismyip.akamai.com for the IP address.
* Also, the data rate of the audio is too high for streaming using mp3. The OPUS codec which promises lower rates is not available and I was advised that real-time encoding might not work out. I was playing a little with the Audio libraries by Phil Schatzmann (which are for ESP32 and do not compile out of the box for Spresense). I got some data out of the OPUS codec, but could not verify it. Also, it was on the limit of real-time. I put it aside for now.
* Finally, I integrated the libopus and was able to stream the data via MQTT to AWS IoT. With an input of 48KHz and a complexity setting of 0 using the VOIP setting, the encoding is possible in real time when done on a dedicated core. However, after packaging it into Ogg packages and sending it via MQTT was not real-time anymore. Some tuning is left to do on the transmission part.
 
**ArduinoBLE needed adaptions and was causing crashes**

* Initially, BLE did not work because the nRF modules do not have an address on their own and require setting up a random address. It was required to debug the HCI messages using the Bluetooth specification. Decoding the messages with Wireshark was useful. Several locations were adapted to use a random address for advertising and connection. Additionally crashes occurred. The issue tracker on GitHub revealed several entries about occasional crashes. However, applying suggested fixes did not help. After a long debugging session I found that inside BLEUuid.cpp there is a strlen(str) where str can be NULL. Spresense SDK does not set CONFIG_LIBC_ARCH_STRLEN and defines its function for strlen() where it tries to dereference NULL. ArduinoBLE does not seem to do checks for corrupted or unexpected data, so I am still a little worried. However, I did not see any crashes after handling the NULL case myself.

**Event-driven program flow.**

* I did not find any Arduino library where I could have events with a flexible number of data that have variable sizes e.g. for string values. Therefore I took the best candidate and augmented it.
	* By now the original library was changed a lot and stripped down additionally that one can say there is no relation anymore.

**Using C++ after years of C programming**

* I wonder how well my code is written concerning best practice C++ patterns because I found myself several times trying to do some C stuff.
	* What I struggle with is that entry points of NuttX tasks must be free functions. Consequently functions inside the classes must be public to be callable …

## Wiring the hardware together
Please have a look at the diagram below for complete wiring. Only pins that are in use are named. Colours do not have special meaning except that I tried to use blackish for ground and reddish for plus power lines.
![Bicycle Computer schematic hardware](/doc/diagrams/bike-cam-schematic-hardware.drawio.png)
Hardware wiring

![Bicycle Computer Overview](/doc/images/BicycleComputer-Overview.jpeg)
Top View of all Components

![Bicycle Computer Bottom](/doc/images/BicycleComputer-Bottom.jpeg)
Bottom View of all Components

![Bicycle Computer Side 1](/doc/images/BicycleComputer-Side1.jpeg)
Side View 1

![Bicycle Computer Side 2](/doc/images/BicycleComputer-Side2.jpeg)
Side View 2

![Bicycle Computer Side 3](/doc/images/BicycleComputer-Side3.jpeg)
Side View 3

![Bicycle Computer KlickFix](/doc/images/BicycleComputer-KlickFix.jpeg)
KlickFix Parts

![Bicycle Computer Update](/doc/images/BicycleComputer-Update.jpeg)
Updated version with the GNSS antenna attached and with the additional level shifters

Special mentions about the hardware setup
* Set the voltage selector on the LTE extension board to 3.3V. We will use that for the display.
* The pinout of Seeedstudio XIAO BLE is custom as I modified it with the firmware. The pin labelled 3.3V on the board is VIN. The nRF8240 MCU works also on 1.8V. Therefore it is fine to attach the Spresense 1.8V to the XIAO BLE 3.3V. Take care that you NEVER attach the USB-C at the same time. Powering the XIAO BLE will make the 3.3V pin a 3.3V output, which destroys the Spresense board.
* The white addon board was an Adafruit Perma-Proto Mint Tin Size Breadboard PCB that I cut into a fitting size. I highly recommend a breadboard with connected lines, not just circles. I wasted a board incl. an XIAO BLE because I was not able to connect the holes with pins on both sides …Take care that the board does not interfere with the GPS antenna as recommended in the documentation. I am about 1-2 mm over the line.
* The display connection is mixed up between the main board and the extension board. The SPI on the main board is much faster than on the LTE extension board. That is why MOSI and SCK are connected to the main board. The 1.8V signal works on the display. The other lines can be any pin. As I have 2 shields, and no free pins, I used the ones on the extension board. Also, the backlight control has to match the display voltage. Make sure you do not connect the MISO to the main board as this would be 3.3V destroying the main board.
* I tried to attach an external GPS antenna as my signal was poor and I could not get a fix at all sometimes. I cut the R29 and R33 and connected the R31 according to the  [documentation](https://developer.sony.com/develop/spresense/docs/hw_docs_lte_en.html#_how_to_use_the_external_antenna_for_gnss). GNSS signal improved. However, I accidentally tore off the antenna connector (which was not soldered well). To my surprise, GNSS is still working with the improved signal. The only explanation for this would be that I failed to cut R33 or somehow connected it again while connecting the R31 next to it. 
* By now I re-attached the antenna. 
	* Adding additional flux before soldering made the connector stick to the correct place a little and let the solder flow much better. That was the key. Flux inside the lead is not enough.
	* Second, selecting the correct glue is also important. Alon Alpha is not suitable. Some gel works better.
	* I checked all connections with a multimeter consulting the schematics. I did not make any mistakes with the modifications. All locations were cut or bridged correctly.

* Attaching the connection to the battery is a little easier. If you use a USB power bank as recommended in the docs, you won’t need the battery connector.
* The Qwiic adapter in use is probably hard to get outside of Japan. All it is doing is level shifting on the i2c lines. There are also i2c lines on the LTE extension board muxed with the PWM pins. Either you use these or you get your level shifter. I made a good experience with the  [Adafruit 4-channel I2C-safe Bi-directional Logic Level Converter - BSS138](https://www.adafruit.com/product/757.). Just keep in mind that due to the pull-ups on the Spresense main and extension board, some i2c devices might not work well. See also [ [I2C on LTE extension board | Sony's Developer World forum](https://forum.developer.sony.com/topic/708/i2c-on-lte-extension-board/9) ]
* I tried to use pins on the LTE extension board as wakeup pins, but for some reason, they either always triggered wakeup immediately or never. I tried to set internal pullups/down, change the code etc. It works for pins on the Spresense board, hence I added this additional level shifter. According to [Cold sleep wakeup by pin on extension board not possible?](https://forum.developer.sony.com/topic/744/cold-sleep-wakeup-by-pin-on-extension-board-not-possible), it should be possible on the extension board. I did not try it again as it was working on my setup.


## About the Prototype Setup
As I had many small parts I decided to fix them together on an acryl sheet. I tried to figure out the dimensions of each board and arranged them using nylon spacers. (not brass to avoid interference with the antennas) I sketched on paper and opened holes to attach the spacers. My wife's father supported me with opening the holes and cutting the material. Initially, I made a mistake and the IMU board covered the LTE antenna, so I had additional holes. Luckily the IMU board and the display had the same dimensions. That is why the display is positioned there. When mounted on the bicycle I noticed it should be better rotated 90 degrees so that it faces up to be better visible. I did not change that yet due to time limitations and because it would cover the onboard GPS antenna that I still use (because the external antenna was stipped off)

The mounting to the bicycle is done via a Klickfix connector. I had a bag which I use for over 10 years. Luckily the holder could be removed from the bag easily by opening a screw and attached to an aluminium sheet folded by 90 degrees by adding 3 holes to it. The reason to use this connector was just that it was available (and reliable). There is no specific reason to use an aluminium sheet. It was just available. The holes are rather big because I made a mistake with the initial placement.

The sharp edges increase the risk of injury in case of an accident.

A real case is to be designed, but I do not own a 3D printer and do not have much experience in housing design, so I made this a low priority for now and will do that later.

I doubt you want to do exactly the same design, but to improve it (display position, edges, material,...). Also, the bag seems to be quite expensive just to get the connection.

There is no thought about battery placement or maybe later about where to attach solar panels and which size.

## Multi-Core on Spresense
The first version used ASMP to utilize other cores. The handling for Bluetooth and the IMU was on the second core.
However, I decided to remove the ASMP feature because of
* the additional complexity introduced to send events between cores. The messaging functionality occasionally crashes for example in the case of sending messages while the other side is not listening.
* functionality on the sub-cores is restricted. Most of the Spresense features are not supported.
* the features on the sub-core are waiting most of the time. They do not use many resources, so they could be also on the main core.
* each core uses a multiple of 128KB memory, which leaves memory unused.
* trying to move the display rendering to the sub-core failed. It was not stable. 
* I thought it was not worth all the effort given the restrictions and decided to stop using ASMP.

After completing the features, I switched to SMP. In theory, the restrictions on the sub-core do not apply because any task could be switched to the main core (means CPU core 0) dynamically. It works for all features, except GNSS. When GNSS is enabled the system is extremely unstable. Details mentioned in the chapter /Going multi-core is complex/.

## Requirements and Design Documentation
I was looking at how I could manage requirements so that they could be traced to the design, implementation and tests. My goal was to practice writing requirements here.

I decided to go with StrictDoc because I wanted the format to be plain text for the ability to diff versions and manage versions in Git. It has all the basic features. Simple formatting can be embedded by using reStructuredText which then could also embed the test cases directly as entries to complete the tracing between requirement, design and test. To trace also source code, the requirement ID could be given in the commit message (which I missed in the beginning and still do not do). See [Welcome to StrictDoc’s documentation! — StrictDoc 0.0.33 documentation](https://strictdoc.readthedocs.io/en/stable/)

In the following, I give some excerpts from the requirements to illustrate what they look like. If you are interested in all the requirements (be aware there are >200 items for requirements and design), have a look in the doc/00_requirements folder and into scripts/03_create_documentation.sh. This script generates the HTML files from the plain text files. See screenshots of the final results below. You might notice that information in this article overlaps a lot. The requirements try to be a complete description of the project standalone.

### Screenshots of the tracing view
![Screenshot requirements 1](/doc/images/Screenshot-requirements1.png)

![Screenshot requirements 2](/doc/images/Screenshot-requirements2.png)

### Excerpt - Purpose of [the Requirements] Document
This document describes the requirements, derives specifications and takes care of tracing the coverage to the tests.

This document shall
* describe the features on a high level, track down to specific implementations and explain verification.
* give the reader the reasons why features, architectures and specific implementations are like they are now.
* give a clear picture of the progress of the implementation and what really works.

Writing this document is practice for me
* to formulate requirements in various granularity and think about which information should be a requirement or a design.
* to describe the system across several hierarchies from requirements to detailed design.
* to decide on the linkage and how to split up further down incl. answers to problems like
	* how to get along with n:m relations?
	* can I skip hierarchies?
	* how to be sure coverage is complete?
* to decide on a verification strategy that best fits each statement.
* to try a tracing framework.
* to get a feeling about how much documentation is appropriate for a project (This might be too much but it is meant as practice :) )

Target Audience
* Myself
* Other developers that want to use the learnings and insights of this project to build similar.

### Excerpt - A sample requirement
UID: REQ-AUG-001

STATUS: Approved

VERIFICATION: Design

CHILD REFS: HLD-AUG, LLD-AUG-001, LLD-AUG-002, LLD-AUG-003, LLD-AUG-004, LLD-AUG-005, LLD-AUG-006, LLD-AUG-007

STATEMENT: The cloud data augmentation module shall provide nearby sightseen information to display it.

RATIONALE: Use the LTE connection to enrich local sensors with cloud data to increase user value.

COMMENT: For the beginning start with ice cream shops.

### Excerpt - A sample design description
UID: LLD-AUG-003

STATUS: Approved

VERIFICATION: AutomaticTest

PARENT REFS: REQ-AUG-001

STATEMENT: The cloud data augmentation module shall calculate the distance to the nearest open ice cream shop by an offline algorithm approximating the straight line distance in kilometres when the GNSS position becomes available.

RATIONALE: The distance is shown on the display

COMMENT: Take the shape of the earth into account.

## Testing
Testing is in a PoC state. I was thinking about a practical approach to how I could test the system against the requirements and design. I decided for the approach to use the serial console to send commands and read the results inside the logs. The event-based approach allows injecting any commands. By that, I can flexibly trigger any feature. For sure there is additional implementation effort, but you should expect this when you want to do tests. The logs need to print out the information required for verification.

As a framework, I went with the robot framework because it allows the formulation of the test cases in natural language. This can then also serve as documentation.

Only a few test cases are implemented. I do not have the time to increase the test coverage ... As said before, PoC state.

A few example test cases.
```
Query Memory Tiles Usage
    [Tags]  LLD-STATUS-001  LLD-STATUS-002  LLD-STATUS-003
    Send shell command    publish_globalStatusTestMemory
    ${DATA} =    Read Until    terminator=FINISHED TEST
    Should Match Regexp    ${DATA}    Memory tile usage: used:\\d+K / free:\\d+K \\(Largest free:\\d+K\\)
    Should Match Regexp    ${DATA}    SD usage: used:\\d+(M|G) / free:\\d+(M|G)
    Should Match Regexp    ${DATA}    RAM usage: used:\\d+B / free:\\d+B \\(Largest free:\\d+B\\)
```

```
Send GNSS Information with MQTT on Update
    [Tags]  LLD-STATUS-008  TODO
    Connect    ${MQTT_ADDRESS}    port=${MQTT_PORT}  client_id=${MQTT_CLIENT}    clean_session=False  ca=${MQTT_CA}  cert=${MQTT_CERT}  private=${MQTT_PRIVATE}
    Subscribe    topic=${MQTT_TOPIC_STATUS}
    Read Until    terminator=topic = jens/feeds/spresense.status    timeout=80
    ${MESSAGE} =    Get Message
    Should Match Regexp    ${MESSAGE}    \\{"batteryPercentage":\\d+,"bleConnected":\\d+,"phoneNearby":\\d+,"lteConnected":\\d+,"ipAddress":,"lat":\\d+.\\d+,"lng":\\d+.\\d+,"speed":0.0,"direction":\\d+.\\d+,"geoFenceStatus":\\d+,"displayOn":\\d+,"firmwareVersion":\\d+,"sdCardFreeMemory":"\\d+G","heapFreeMemory":"\\d+"\\}
    Disconnect
```

## Switch from Arduino SDK to Spresense SDK
Initially, I gave myself the requirement to use the Arduino SDK. However, I switched to the Spresense SDK for the following reasons
* Faster cycles when changing the SDK configuration.
* Support for hardware debuggers.
* Arduino libraries did not work as expected or as I wanted
	* Some libraries needed modifications to compile for the Spresense board
	* libraries need to work with all boards. Therefore there are no optimizations. The Arduino way copies streams byte by byte. Therefore the performance is poor e.g. for the libraries drawing to TFTs, for libraries reading files, and for libraries transferring data ... (Arduino provides APIs to do this correctly though)
	* libraries are missing functionality like setting the interrupt on the IMU
	* libraries are wasting memory by (in my case) unused buffers like the Audio library
	* libraries are using signals that interfere with other parts of the program like the GNSS library
* So in the end I needed to dig in all libraries and for some to modify them.
* Optimize .text size to save memory by disabling unused features and by using only required parts.

The Arduino SDK is really great as a starting point and for small PoCs to get something working quickly. This works when there is an example sketch available and you can copy & paste them together. Better assume that features without example sketch do not work.

## SDK Configuration
A comparison to the default "defconfig" is stored inside the GitHub repository.
In the following, I will make a comparison between the Arduino SDK configuration and my configuration.
* Raise the buffer for the SPI buffers for the display
```
#define CONFIG_CXD56_DMAC_SPI4_TX_MAXSIZE 192000
#define CONFIG_CXD56_DMAC_SPI4_RX_MAXSIZE 192000
```
* Switch on SMP. It is not clear if all these changes are required. Selecting CXD56_USE_SYSBUS instead of CXD56_TESTSET_WITH_HWSEM works fine too.
```
#define CONFIG_CXD56_TESTSET 1
#define CONFIG_CXD56_TESTSET_WITH_HWSEM 1
#define CONFIG_SPINLOCK 1
#define CONFIG_IRQCOUNT 1
#define CONFIG_SMP 1
#define CONFIG_SMP_NCPUS 6
#define CONFIG_SCHED_RESUMESCHEDULER 1
```
* Increase the maximum size of the message queue. As I changed the MQTT to dynamic memory allocation, this is not necessary anymore
```
#define CONFIG_MQ_MAXMSGSIZE 512
```
* Activate externals. I left configurations related to externals to the default values.
```
#define CONFIG_EXTERNALS_AWSIOT 1
#define CONFIG_EXTERNALS_MQTT 1
```
* Remove the configuration of modules not in use to save program memory.
```
#define CONFIG_CXD56_HOSTIF 1
#define CONFIG_ASMP 1
#define CONFIG_MM_TILE 1
#define CONFIG_ASMP_MEMSIZE 0xc0000
#define CONFIG_AUDIOUTILS_MANAGER 1
#define CONFIG_AUDIOUTILS_PLAYER 1
#define CONFIG_AUDIOUTILS_PLAYLIST 1
#define CONFIG_AUDIOUTILS_PLAYER_CODEC_PCM 1
#define CONFIG_AUDIOUTILS_PLAYER_CODEC_MP3 1
#define CONFIG_AUDIOUTILS_PLAYER_CODEC_AAC 1
#define CONFIG_AUDIOUTILS_PLAYER_CODEC_OPUS 1
#define CONFIG_AUDIOUTILS_RECORDER 1
#define CONFIG_AUDIOUTILS_SOUND_RECOGNIZER 1
#define CONFIG_AUDIOUTILS_SYNTHESIZER 1
#define CONFIG_AUDIOUTILS_SRC 1
#define CONFIG_AUDIOUTILS_OUTPUTMIXER 1
#define CONFIG_AUDIOUTILS_CAPTURE_CH_NUM 2
#define CONFIG_AUDIOUTILS_RENDERER_CH_NUM 2
#define CONFIG_AUDIOUTILS_DECODER 1
#define CONFIG_AUDIOUTILS_ENCODER 1
#define CONFIG_AUDIOUTILS_FILTER 1
#define CONFIG_AUDIOUTILS_RECOGNITION 1
#define CONFIG_AUDIOUTILS_RENDERER 1
#define CONFIG_AUDIOUTILS_CUSTOMPROC 1
#define CONFIG_AUDIOUTILS_DSP_DRIVER 1
#define CONFIG_DIGITAL_FILTER 1
#define CONFIG_DIGITAL_FILTER_FIR 1
#define CONFIG_DIGITAL_FILTER_DECIMATOR 1
#define CONFIG_DIFITAL_FILTER_EDGE_DETECT 1
#define CONFIG_DNN_RT 1
#define CONFIG_DNN_RT_MP 1
#define CONFIG_FWUPUTILS 1
#define CONFIG_FWUPUTILS_CLIENTS 1
#define CONFIG_SENSING_MANAGER 1
#define CONFIG_SENSING_STEPCOUNTER 1
#define CONFIG_EXTERNALS_CMSIS_NN 1
#define CONFIG_EXTERNALS_NNABLART 1
```
CONFIG_AUDIOUTILS_SRC has a dependency on ASMP. If only the ThruProcComponent is used, this is actually not true. I removed references to UserCustomComponent and SRCComponent from modules/audio/objects/front_end/front_end_obj.cpp and added the following to the modules/audio/components/customproc/Make.defs to make my program compile.
```
ifeq ($(CONFIG_AUDIOUTILS_MFE),y)

CXXSRCS += thruproc_component.cpp
VPATH   += components/customproc
DEPPATH += --dep-path components/customproc

endif
```

## Event-Driven Architecture
I split my application into modules that communicate with each other via events. That means each module can listen to certain events and react to them. They can also send events and attach data to them. The main loop only consists of an Event Manager that processes the events queue by distributing the events to the modules listening for it. Each module defines a specific event class that is derived from a common event class and adds module-specific data to it.
The following diagram shows the software modules and the used Arduino libraries. A description of each module follows in the next chapter.

![Bicycle Computer schematic software](/doc/diagrams/bike-cam-schematic-software.drawio.png)
Software Component Diagram

```
class CycleSensorEvent : public Event {
 public:
  enum CycleSensorCommand {
    CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED,
    CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED,
    CYCLE_SENSOR_EVT_HEART_RATE_READING,
    CYCLE_SENSOR_EVT_SAW_MOBILE,
    CYCLE_SENSOR_EVT_LOST_MOBILE,
    CYCLE_SENSOR_EVT_ERROR
  };

  CycleSensorEvent(CycleSensorCommand cmd, int heartRate = 0) {
    mType = Event::kEventCycleSensor;
    mCommand = cmd;
    mHeartRate = heartRate;
  }

  int getHeartRate() { return mHeartRate; }

 private:
  int mHeartRate;
}
```
For example the CycleSensorEvent.
It defines commands like "heart rate sensor connected" or "new heart rate reading" and stores a value for the heart rate reading which you can get back with getHeartRate. Whenever a new heart rate reading arrives I create a new event and enqueue it to the message queue.
```
CycleSensor.publish(new CycleSensorEvent(
          CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING,
          (uint16_t)hrmValue));
```

Each module has a publish function that enqueues this message. It is public because several modules have callbacks that must be a free function. That is why each module defines a singleton-like global instance.
boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

Each module defines a begin function here it registers for the events it wants to handle and defines a function where this event is handled. Here we need to convert between the base event type and the specific event type.
```
bool UiTftClass::begin() {
  if (DEBUG_UITFT) Log.traceln("UiTftClass::begin");
  bool ret = eventManager.addListener(Event::kEventAppStart, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);
  ret = ret && eventManager.addListener(Event::kEventGlobalStatus, this);
  ret = ret && eventManager.addListener(Event::kEventCycleSensor, this);
...
  return ret;
}
```

```
void eventHandler(Event *event);
  virtual void operator()(Event *event) override { eventHandler(event); }
void UiTftClass::eventHandler(Event *event) {
    Log.traceln("eventHandler");
  switch (event->getType()) {
    case Event::kEventAppStart:
      initTft();
      break;
...
    case Event::kEventCycleSensor:
      handleCycleSensor(static_cast<CycleSensorEvent *>(event));
      break;
...

    default:
      break;
  }
}
```

In our CycleSensorEvent example, the UiTft module handles the event and reacts to the connected, disconnected and new reading command by showing the connected item and updating the value on the display.
```
void UiTftClass::handleCycleSensor(CycleSensorEvent *ev) {
  if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING) {
    updateHeartImage(true);
    updateHrm(ev->getHeartRate());
  } else if (ev->getCommand() ==
             CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED) {
    updateBluetoothImage(true);
  } else if (ev->getCommand() ==
             CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED) {
    updateBluetoothImage(false);
  }
}
```

Each module is working the same way. It initializes itself after receiving the app start event. That means by commenting out the begin function you can activate or deactivate complete modules it prevents them from listening to the app start event. You can also easily add new modules.


## Description of each Module
**App Manager**
* The lifecycle is controlled by the App Manager. Each module registers with the App Manager’s events and inits or deinits itself on the app state defined in each module’s header file. There are mainly 3 stages
	* Cold sleep
	* Always running modules providing base functionality like BLE, LTE or GNSS to do anti-theft detection. This runs on 32 MHz.
	* UI-related modules like camera and display which interact with the user.
	* Modules demanding high resources like Audio. (currently mapped to the level below)
* After a 5 minutes timeout of no movement, the system goes into deep sleep waiting for an interrupt from the accelerometer.
* At startup the boot cause is printed, debugging functionality like logging and the serial shell is set up, SD utils are set up, system time is set to the build time, then all modules register with the App Manager and the App Manager is raising the App state based on the modules' feedback.
![Application States](/doc/diagrams/appStates.png)

**Camera Sensor**
* starts streaming the camera with 5 fps and a resolution of 240 x 180 to the display on initialization. Due to the memory size and timing constraints events are bypassed and the display module is directly called.
* saves the stream to the video folder inside the SD card with a time stamp in the filename which is disabled but can be activated by the event CAMERA_EVT_RECORD_STREAM_ENABLE
* takes an image on button release (BUTTON_EVT_RELEASED) or MQTT topic MQTT_TOPIC_GET_IMAGE and saves the image to the SD card and sends it to the MQTT topic MQTT_TOPIC_IMAGE encoded as base64 inside the Mqtt Module. The MQTT message is a JSON string defining width, height, divisor and quality like
`{ "width": 1280, "height": 720, "divisor": 15, "quality": 75 }`
* To prevent out-of-memory the image size is set to 1280 x 960 with a JPEG quality of 75 and a JPEG buffer size divisor of 15. It tried to set size and quality dynamically based on the available memory, however, deactivated it again to save program memory.
* can be started and ended via events
The code follows the examples of the Camera library. Instead of saving still pictures, it would be also possible to record a movie as described here [ [GitHub - YoshinoTaro/Spresense-Simple-AVI-codec: Simple AVI codec for Sony Spresense](https://github.com/YoshinoTaro/Spresense-Simple-AVI-codec) ]

**LTE Module**
* configures the modem for LTE-M with apn, user and password and tries to attach to the network on app start.
* configures low power mode via eDRX and PSM according to [ [arduino - spresense LTE拡張ボードでLTEとADを使うとLTE.shutdown()でエラーになる - スタック・オーバーフロー](https://ja.stackoverflow.com/questions/74425/spresense-lte%E6%8B%A1%E5%BC%B5%E3%83%9C%E3%83%BC%E3%83%89%E3%81%A7lte%E3%81%A8ad%E3%82%92%E4%BD%BF%E3%81%86%E3%81%A8lte-shutdown%E3%81%A7%E3%82%A8%E3%83%A9%E3%83%BC%E3%81%AB%E3%81%AA%E3%82%8B) ]. The function reading the values back after being attached indicates that they are not set. Not all SIM contracts support these settings. As I see this setting as low priority, I left it as is. After doing more research I conclude that it does not make sense to set both power-saving modes and that they are to be used with devices sleeping most of the time. In my case, they might not make sense. I deactivated the code. A meaningful setup is still to be found. 
* sends the obtained IP address as an event (I removed this because of the issue that the device is not remotely accessible due to the SIM contract limitations)
* sets the received time for the RTC
* A separate thread checks every minute if we are attached to the network and reconnects if not.
The actual connections are made by the cloud and MQTT modules.

**MQTT Module**
* can receive cloud commands to get an image. It shall trigger taking a photo and sending it back as binary jpg data
* can request an audio recording as opus wrapped in Ogg container format.
* can request the (global) status of the device.
* sends the position as Json by the event GNSS_EVT_POSITION
* can transmit any topic and message via generic events MQTT_EVT_SEND and MQTT_EVT_RECV
I replaced the ArduinoMqtt/LTETLSClient-based implementation with AWS IoT for the following reasons
* The way the LTETLSClient sets the certificates is very slow (reported to the Spresense team, which might get fixed in future releases). Using the Arduino samples for HTTP and MQTT leads to random failures.
* The paho MQTT library leads to failures on the AWS IoT side. The only information I get is user error. I could not solve it and others on the internet also not. It could have causes like re-use of the client name, unclean disconnects, and security rules, ... so I switched to AWS IoT SDK which is included as externals inside the Spresense SDK.
* The handling of MQTT is done inside a thread that is connected by a message queue. It periodically calls aws_iot_mqtt_yield() to receive MQTT messages from the remote.
* Before sending a message, the AWS IoT client needs to be in idle mode. I created a function to wait for idle, otherwise sending a message will fail e.g. while the client is processing the yield. This can block for quite some seconds.
* The AWS IoT SDK automatically takes care to reconnect on connection loss and takes care of setting the certificates. Only a path is required.

The module handles the Mqtt event MQTT_EVT_SEND, where any module can send a message to a topic. On receiving an MQTT topic Mqtt event MQTT_EVT_RECV is sent to listeners. 
To send images a direct call function directCall_sendImage(CamImage& img) is provided. 

**Cloud Module**
* starts after LTE is connected and ends with LTE disconnected
* searches for the nearest (open) ice cream shop in a distance of 5 km, calculates the distance to the current position and the bearing to the current heading. The bearing and distance are updated with GPS updates. The uiTft module is updated via events.
* When the position of the last query is more than 5 km away do an ice cream request again.
* Query the weather after startup and at each full hour and determine if there is rain or sun in the next hour. Send an event to the uiTft module to update the display
* and gets the public IP from  [whatismyip.akamai.com](http://whatismyip.akamai.com/).

I chose Yahoo! Japan APIs because their iOS weather app is very reliable. The drawback is that it only works in Japan. Another point is that it works with simple GET requests and an API key and returns JSON strings. Recently many services made OAuth or similar methods that use time-bound tokens mandatory which makes the handling quite complex. For HTTPS, it is required to retrieve the root certificate and store it on the SD card. See [ [Spresense SDK Examples & Tutorials](https://developer.sony.com/develop/spresense/docs/sdk_tutorials_en.html#_how_to_download_a_root_certificate_for_https_server_access) ] for how to download a root certificate for HTTPS server access.

The API key is stored inside the secrets.h file.

I have string patterns that can be used with sprintf() to create the GET requests. An issue however was that the location API did not return any results for requests in English like "ice cream". I had trouble sending the request correctly encoded for URLs and in utf-8 Unicode. After a lot of trial and error (no idea where it broke) I ended up with "%e3%82%a2%e3%82%a4%e3%82%b9%e3%82%af%e3%83%aa%e3%83%bc%e3%83%a0" which translates to "アイスクリーム" which means "ice cream".

ArduinoJson is used to retrieve desired information from the response.

I use the geofence feature to know when to update the shop information exiting after a 5 km of distance. Communication with the GNSS module is done via events.

Functions to calculate the distance and bearing are taken from StackOverflow [ [c++ - Calculating the distance between 2 latitudes and longitudes that are saved in a text file? - Stack Overflow](https://stackoverflow.com/questions/10198985/calculating-the-distance-between-2-latitudes-and-longitudes-that-are-saved-in-a) ]. This function approximates the distance by assuming the earth is a sphere using the Haversine formula. Yahoo! Japan has also an API for calculating the distance, but I need this too frequently to do remote calls.

The connection is handled by the webclient, which is available in the netutils.

**GNSS Module**
* starts after LTE is attached and initializes to use GPS, GALILEO and Michibiki (QZ_L1CA and QZ_L1S).
* uses the time received from LTE and the coordinates of the home to try a hot start.
* stores GNSS satellite information upon fix and when deinit is called
* sets up a geofence for the home coordinates which is set in secrets.h
* On fix, an event is sent with the position. In case of a hot start, the last known position is sent once.
* Auxiliary functions print various information about the satellite's status and position.
* A geofence can be added, deleted or updated via the event API. Geofence events are translated into GNSS module events.

Both GNSS and Geofence follow mainly the examples in the SDKs. The Geofence example is from the Spresense SDK.
Trying to delete a not-yet-set geofence returns an error during initialization. Therefore I skip the return on deletion error and ignore it.

Geofence and GNSS have their threads that wait for an event with POLL_TIMEOUT_FOREVER.

I switched to the Spresense SDK for the GNSS module. Unfortunately, when SMP is on, the GNSS driver tends to go to a deadlock and freezes the system. The reason is that GNSS is handled on a dedicated MCU. Only CPU core 0 is capable to communicate with the GNSS CPU. The same applies to the communication to the system CPU. While communication with the system CPU goes well because the driver takes care of switching the task to core 0 and it is synchronous. The GNSS CPU sends interrupts for incoming data. An unfortunate timing in the communication with other CPUs might cause a deadlock.

Pinning all your threads to dedicated cores is important. The assignment as given in config.h is the result of a working configuration.

So in conclusion, when using SMP and GNSS, take care of the following. (See [here](https://forum.developer.sony.com/topic/865/deadlock-issue-in-gnss-when-smp-is-on/5))
* Setting the GNSS time with CXD56_GNSS_IOCTL_SET_TIME will freeze the system -> leave it out
* Pinning the gnss_receiver thread and the thread that reads the GNSS to the same CPU will freeze the system -> pin gnss_receiver to a dedicated CPU.
* Using signals will freeze the system. -> Use polling with fds.
* Avoid any thread on CPU core 0 to free it up for any task switching to the CPU core 0 immediately.

**Global Status**
* sends a status information JSON to the MQTT topic MQTT_TOPIC_STATUS and to the SD card including battery percentage level, BLE connected status, LTE connected status, IP address, GNSS information, geofence status for home, if the display is on, firmware version and SD Card usage.
* (deactivated by CONFIG_REMOVE_NICE_TO_HAVE) Memory tile usage is retrieved via MP library GetMemoryInfo, dynamic memory usage is retrieved via "/proc/meminfo" and disk space usage via "/proc/fs/usage" is printed to Serial. In the future, it shall be parsed and added to the global info.
* Battery level is retrieved via LowPower library getVoltage() and converted to an approximated percentage using a mapping from [ [What is the relationship between voltage and capacity of 18650 li-ion battery? - Benzo Energy / China best polymer Lithium-ion battery manufacturer, lithium-ion battery, lipo battery pack, LiFePO4 battery pack, 18650 batteries, Rc battery pack](http://www.benzoenergy.com/blog/post/what-is-the-relationship-between-voltage-and-capacity-of-18650-li-ion-battery.html) ]
* Retrieve heap memory usage and sd card memory usage via the procfs and parse the results.

I noticed that accessing the SD card when there are hundreds of files in a folder takes a huge amount of time blocking the whole program flow. About 21.5s for 560 files. I found that the procfs gives information on the available memory on the SD card. 

**Button Module**
* attaches an interrupt on the button pin and sends events for on-pressed and on-released.

**Cycle Sensor Module**
* searches for a BLE device that advertises a heart rate UUID and connects to it when found.
* registers notification for heart rate reading characteristic of a heart rate service and sends an event to the display with the reading and whether the sensor is attached or not
* advertises an arbitrary UUID with an arbitrary service and characteristic for test purposes. Devices like smartphones can connect and write it.
* checks if my smartphone is nearby and sends a notification if not. It uses the IRK of the smartphone to resolve the private address of the smartphone. Without it, the smartphone cannot be identified.
* To retrieve the IRK (besides activating development logs of all BLE communication on the iPhone), an encrypted attribute is provided. Any communication is unencrypted until reading this attribute, which switches to an encrypted connection. During the encryption process, the LTK (long-term key) and IRK (identity resolving key) are exchanged. Both are saved into files on the sd card. Please connect from the phone and do the pairing process once. Any pairing attempt will be shown on the display, but the pairing code will be accepted unconditionally as we have no input devices available.
* Unfortunately reconnecting with an already paired phone did not succeed for some reason. Even after debugging intensively the ArduinoBLE and transmission logs, I could not see why the phone cannot resolve our address and recognize the device correctly. It might be due to setting our resolvable random address incorrectly. ArduinoBLE supports encryption, but it seems to be hardly used and incomplete.
* I tried to transmit the logging stream via BLE. With the current implementation, it is too slow. In principle, it should work via BLE, but maybe some setting is missing. Probably increasing the MTU size? Or writing in larger batches buffering more data? As I defined it as nice to have, I did not fix this yet.

As I did not find any add-on boards capable of a BLE stack (i.e. not just UART over BLE), I am using the ArudinoBLE library which I modified to work with my own HCI protocol over UART-based BLE controller flashed onto a Seeedstudio XIAO BLE which I created with the sample available for Zephyr OS [ [Bluetooth: HCI UART — Zephyr Project Documentation](https://docs.zephyrproject.org/2.7.0/samples/bluetooth/hci_uart/README.html) ]. So far the summary. Details are available here [ [GitHub - jens6151/spresense-BLE-with-XIAO-BLE: Using the Seeedstudio XIAO BLE as an extension board for the SONY spresense board](https://github.com/jens6151/spresense-BLE-with-XIAO-BLE) ]. A precompiled firmware for XIAO BLE is available together with the instructions on how it was created.

I tried to check if my smartphone is nearby by searching for devices. Since my smartphone is using a changing random address for security reasons, I could not set an address to search for. Additionally, the phone’s name is not part of the advertisement. I checked all bytes of the HCI messages, but the name was not present. Therefore I cannot detect the phone and the anti-theft logic with the smartphone could not be implemented. I thought about the following next steps. (1) connect by the phone to the advertised UUID to get the current BLE address and use that or (2) pair the devices initiated by a companion app on the phone.

I had long-time issues with crashes inside the ArduinoBLE until I could identify the root cause as a strlen(NULL) call.

**Imu Module**

Configures the LSM303 accelerometer to send an interrupt on INT1 on a threshold value of 250mg on the Y and Z axis. This should detect any movement. Keep in mind that gravity is not removed when applying the thresholds. In my setup, the X-axis points down. I set the interrupt latching, which means it will be active until reading the value. I do not attach any interrupts on the pins because adding interrupts to the system makes it somewhat unstable. I read the value every 5 seconds. In case of 5 minutes not detecting any movement, I send a STILL event that will bring the system to low power mode.

As the Adafruit library does not support setting interrupts, I needed to make the read8 and write8 functions public and set the registers myself according to the manual.

As I did not see any use for the orientation in AHRS, I disabled the code that fuses the IMU sensor values with a Kalman filter to absolute orientation values.

As of now this module only detects movement used for anti-theft detection and going to low power mode.

I made some considerations to detect accidents. Nothing is implemented yet. It is not that simple given that you do not want false positives (for just breaking) but we need a high recall for the accident event. Using any DNN-based approach is not possible as I do not have enough memory left for that. 

The IMU is mounted so that always x is pointing down, which means it will see gravity as acceleration. Other directions can get acceleration when driving. Usually, there should be no acceleration (or not much) to the side. Only while turning. So seeing gravity in the side direction would indicate a fall. Front/back would be driving or breaking. A fast declaration could be just breaking hard. For a fall multiple directions likely give an interrupt, while for deceleration it is usually the driving direction Y. A better approach would be to check if gravity is removed from the down direction, which means adding a low threshold. I can be combined with a strong interrupt in other directions to make a distinction between falling and laying the bike down. Additionally, we can record the last x values if it was a still or not.

**UiTft Module**
* updates the battery gauge every minute by the global status information
* updated the weather forecast every hour to a sun or rain icon by the cloud weather query
* shows an ice cream symbol when an ice cream shop nearby is found
* indicates the direction to the ice cream shop based on the current heading
* shows the time
* shows an icon when the microphone is recording, a GNSS fix is available, the LTE is connected or the BLE is connected.
* shows debug information like the IP address, the ice cream shop name, distance and bearing, the number of satellites, the geofence status, the firmware version, the free disk space of the SD card, the current position
* shows the heart symbol when a heart rate sensor is connected, a hyphen if not closed or the measurement
* shows the current speed and the heading
* shows a live stream of the camera
* can turn the TFT backlight off via the event interface
* shows a popup with text for Bluetooth pairing

All information is received via events, except the camera makes direct calls due to memory size and time constraints.

The following is a screenshot of the design created with Figma (not the latest state regarding debug information)
![bicycle Computer Display](/doc/images/bicycleComputerDisplay.webp)

The initial design of the display content created in Figma

The TFT is controlled by the libraries TFT_eSPI and lvgl.

TFT_eSPI is the driver that initializes the TFT and communicates via SPI to draw onto the display. This library works for any SPI-based display and needs to be configured. This is my User_Setup.h
```
#define USER_SETUP_INFO "User_Setup"

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

#define TFT_MISO      -1
#define TFT_MOSI      99
#define TFT_SCLK      99
#define TFT_CS        32
#define TFT_RST       5
#define TFT_DC        9

#define TFT_SPI_PORT 5


#define SPI_FREQUENCY  30000000

// to avoid linker error missing function ltoa
#define ltoa itoa

#define DISABLE_ALL_LIBRARY_WARNINGS
```
The refresh rate is very slow using SPI3 on the LTE board without any optimizations. Therefore I (1) use the SPI5 on the Spresense board (2) with DMA enabled and (3) push all pixels at once. (See also [ [Slow refresh rate streaming the camera preview to a display | Sony's Developer World forum](https://forum.developer.sony.com/topic/658/slow-refresh-rate-streaming-the-camera-preview-to-a-display/6) ] where I describe more details)

To use SPI5 with DMA it is required to configure the Spresense SDK with LCD_ON_MAIN_BOARD and rebuild the Arduino Spresense SDK. It worked on the ST7789 too without level shifters. I only use MOSI and CLK on the main board. Other connections are to the LTE extension board. So a 1.8V and 3.3V mix. (I assume connecting MISO which is unused would destroy the board as the display runs on 3.3V...)

Max speed is 48.75Mbps according to the documentation. I set it to 30 Mhz, otherwise, I get artefacts on the screen. If you have a good connection, maybe 40 Mhz will work (see definition in the board.h ILI9340_SPI_MAXFREQUENCY 40000000).

I activate DMA for SPI5 to increase the speed to draw on the display (refresh rate). This function needs to be called before initializing the display.
```
// Processors/TFT_eSPI_Generic.c
#include "/Users/jens/work/mcu_prj/prj/spresense/references/spresense-sdk/spresense/nuttx/arch/arm/src/cxd56xx/cxd56_spi.h"
#include <arch/board/board.h>

/***************************************************************************************
** Function name:           initDMA
** Description:             Initialise the DMA engine - returns true if init OK
***************************************************************************************/
bool TFT_eSPI::initDMA(bool ctrl_cs)
{
  if (DMA_Enabled) return false;

#if defined(CONFIG_CXD56_DMAC)
  ctrl_cs = ctrl_cs;  // stop unused parameter warning

#if DISPLAY_SPI != 5
#error "DISPLAY_SPI selection is wrong!!"
#endif
#if CONFIG_LCD_ON_MAIN_BOARD != 1
#error "CONFIG_LCD_ON_MAIN_BOARD selection is wrong!!"
#endif

//  Serial.printf("DISPLAY_SPI IS %d\n", DISPLAY_SPI);
//  Serial.printf("DISPLAY_DMA_TXCH IS %d\n", DISPLAY_DMA_TXCH);
//  Serial.printf("DISPLAY_DMA_TX_MAXSIZE IS %d\n", DISPLAY_DMA_TX_MAXSIZE);
//  Serial.printf("DISPLAY_DMA_TXCH_CFG IS %d\n", DISPLAY_DMA_TXCH_CFG);
//  Serial.printf("CXD56_DMAC_WIDTH8 IS %d\n", CXD56_DMAC_WIDTH8);
//  Serial.printf("CXD56_SPI_DMAC_CHTYPE_TX IS %d\n", CXD56_SPI_DMAC_CHTYPE_TX);
//
//  Serial.printf("DISPLAY_DMA_RXCH IS %d\n", DISPLAY_DMA_RXCH);
//  Serial.printf("DISPLAY_DMA_RX_MAXSIZE IS %d\n", DISPLAY_DMA_RX_MAXSIZE);
//  Serial.printf("DISPLAY_DMA_RXCH_CFG IS %d\n", DISPLAY_DMA_RXCH_CFG);
//  Serial.printf("CXD56_DMAC_WIDTH8 IS %d\n", CXD56_DMAC_WIDTH8);
//  Serial.printf("CXD56_SPI_DMAC_CHTYPE_RX IS %d\n", CXD56_SPI_DMAC_CHTYPE_RX);

  DMA_HANDLE hdl;
  dma_config_t conf;

  hdl = cxd56_dmachannel(DISPLAY_DMA_TXCH, DISPLAY_DMA_TX_MAXSIZE);
  if (hdl) {
    conf.channel_cfg = DISPLAY_DMA_TXCH_CFG;
    conf.dest_width = CXD56_DMAC_WIDTH8;
    conf.src_width = CXD56_DMAC_WIDTH8;
    cxd56_spi_dmaconfig(DISPLAY_SPI, CXD56_SPI_DMAC_CHTYPE_TX, hdl, &conf);
  }

  hdl = cxd56_dmachannel(DISPLAY_DMA_RXCH, DISPLAY_DMA_RX_MAXSIZE);
  if (hdl) {
    conf.channel_cfg = DISPLAY_DMA_RXCH_CFG;
    conf.dest_width = CXD56_DMAC_WIDTH8;
    conf.src_width = CXD56_DMAC_WIDTH8;
    cxd56_spi_dmaconfig(DISPLAY_SPI, CXD56_SPI_DMAC_CHTYPE_RX, hdl, &conf);
  }

  DMA_Enabled = true;
#endif
  return true;
}
```
I use spi.transfer() to send all pixels at once to highly improve the speed instead of sending pixel per pixel in a loop. As I use lvgl, I usually get a rectangle on the screen to draw, which enables pushing all pixels at once.
```
// Processors/TFT_eSPI_Generic.c
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len){
  spi.transfer(data_in, len);
}
```
Since we send all pixels, it is required to convert the endianness as the display expects a different than Spresense. [ [Fastest way to swap alternate bytes on ARM Cortex M4 using gcc - Stack Overflow](https://stackoverflow.com/questions/41675438/fastest-way-to-swap-alternate-bytes-on-arm-cortex-m4-using-gcc) ] suggests using ARM instructions to swap 4 bytes at once instead of 2 bytes at a time.
```
// ui_tft.cpp
inline uint32_t Rev16(uint32_t a) {
  asm("rev16 %1,%0" : "=r"(a) : "r"(a));
  return a;
}

void swapColors(uint16_t *colors, uint32_t len) {
  len = len / 2;
  uint32_t *data = (/uint32_t/ *)colors;
  for (uint32_t i = 0; i < len; /i/++) {
    data[i] = Rev16(data[/i/]);
  }
}
```
lvgl is a UI framework that renders the screen content to buffers that need to be sent to the display. It is rather complex and I just summarize the steps and special considerations.
* After calling lv_init(), a buffer needs to be prepared by lv_disp_draw_buf_init(), the display driver initialized by lv_disp_drv_init() and the screen dimensions, a drawing function (my_disp_flush) and the prepared buffer registered with the driver.
* After this initialization, any objects like labels, text or images can be created and configured with e.g. the position on the screen, the colourization, visibility and content. Whenever an object changes LVGL takes care to update the display.
* This is done by calling lv_timer_handler() periodically.
lvgl has a config file for its features. It is highly recommended to switch all unused features off. This can save a lot of RAM (as Spresense loads the code section into RAM). I uploaded my lv_config.h which must be in the Arduino library folder on GitHub.
Images are included as c arrays. There is an online converter which I used to convert png to CF_ALPHA_1_BIT formatted c arrays. This saves a lot of memory but reduces the image to 1 colour (done by colourization) [ [Online image converter - BMP, JPG or PNG to C array or binary | LVGL](https://lvgl.io/tools/imageconverter) ]. You have to modify the struct at the end of the resulting c file because it is not c++ compiler compliant.
With the font converter script [ [GitHub - lvgl/lv_font_conv: Converts TTF/WOFF fonts to compact bitmap format](https://github.com/lvgl/lv_font_conv) ] I converted Google Noto Sans CJK to the lvgl format to make Japanese characters (incl Jouyou Kanji) available. As I do not know what the shop is called, I need to provide all Glyphs.
`lv_font_conv  --no-compress --no-prefilter --bpp 1 --size 16 --font NotoSansJP-Medium.otf -r 0x20-0x7f -r 0xb0 -r 0x3000-0x303f -r 0x3040-0x309f -r 0x30a0-0x30ff -r 0xff00-0xffef --symbols 一九七二人入八力十下三千上口土夕大女子小山川五天中六円手文日月木水火犬王正出本右四左玉生田白目石立百年休先名字早気竹糸耳虫村男町花見貝赤足車学林空金雨青草音校森刀万丸才工弓内午少元今公分切友太引心戸方止毛父牛半市北古台兄冬外広母用矢交会合同回寺地多光当毎池米羽考肉自色行西来何作体弟図声売形汽社角言谷走近里麦画東京夜直国姉妹岩店明歩知長門昼前南点室後春星海活思科秋茶計風食首夏弱原家帰時紙書記通馬高強教理細組船週野雪魚鳥黄黒場晴答絵買朝道番間雲園数新楽話遠電鳴歌算語読聞線親頭曜顔丁予化区反央平申世由氷主仕他代写号去打皮皿礼両曲向州全次安守式死列羊有血住助医君坂局役投対決究豆身返表事育使命味幸始実定岸所放昔板泳注波油受物具委和者取服苦重乗係品客県屋炭度待急指持拾昭相柱洋畑界発研神秒級美負送追面島勉倍真員宮庫庭旅根酒消流病息荷起速配院悪商動宿帳族深球祭第笛終習転進都部問章寒暑植温湖港湯登短童等筆着期勝葉落軽運遊開階陽集悲飲歯業感想暗漢福詩路農鉄意様緑練銀駅鼻横箱談調橋整薬館題士不夫欠氏民史必失包末未以付令加司功札辺印争仲伝共兆各好成灯老衣求束兵位低児冷別努労告囲完改希折材利臣良芸初果刷卒念例典周協参固官底府径松毒泣治法牧的季英芽単省変信便軍勇型建昨栄浅胃祝紀約要飛候借倉孫案害帯席徒挙梅残殺浴特笑粉料差脈航訓連郡巣健側停副唱堂康得救械清望産菜票貨敗陸博喜順街散景最量満焼然無給結覚象貯費達隊飯働塩戦極照愛節続置腸辞試歴察旗漁種管説関静億器賞標熱養課輪選機積録観類験願鏡競議久仏支比可旧永句圧弁布刊犯示再仮件任因団在舌似余判均志条災応序快技状防武承価舎券制効妻居往性招易枝河版肥述非保厚故政査独祖則逆退迷限師個修俵益能容恩格桜留破素耕財造率貧基婦寄常張術情採授接断液混現略眼務移経規許設責険備営報富属復提検減測税程絶統証評賀貸貿過勢幹準損禁罪義群墓夢解豊資鉱預飼像境増徳慣態構演精総綿製複適酸銭銅際雑領導敵暴潔確編賛質興衛燃築輸績講謝織職額識護亡寸己干仁尺片冊収処幼庁穴危后灰吸存宇宅机至否我系卵忘孝困批私乱垂乳供並刻呼宗宙宝届延忠拡担拝枚沿若看城奏姿宣専巻律映染段洗派皇泉砂紅背肺革蚕値俳党展座従株将班秘純納胸朗討射針降除陛骨域密捨推探済異盛視窓翌脳著訪訳欲郷郵閉頂就善尊割創勤裁揮敬晩棒痛筋策衆装補詞貴裏傷暖源聖盟絹署腹蒸幕誠賃疑層模穀磁暮誤誌認閣障劇権潮熟蔵諸誕論遺奮憲操樹激糖縦鋼厳優縮覧簡臨難臓警乙了又与及丈刃凡勺互弔井升丹乏匁屯介冗凶刈匹厄双孔幻斗斤且丙甲凸丘斥仙凹召巨占囚奴尼巧払汁玄甘矛込弐朱吏劣充妄企仰伐伏刑旬旨匠叫吐吉如妃尽帆忙扱朽朴汚汗江壮缶肌舟芋芝巡迅亜更寿励含佐伺伸但伯伴呉克却吟吹呈壱坑坊妊妨妙肖尿尾岐攻忌床廷忍戒戻抗抄択把抜扶抑杉沖沢沈没妥狂秀肝即芳辛迎邦岳奉享盲依佳侍侮併免刺劾卓叔坪奇奔姓宜尚屈岬弦征彼怪怖肩房押拐拒拠拘拙拓抽抵拍披抱抹昆昇枢析杯枠欧肯殴況沼泥泊泌沸泡炎炊炉邪祈祉突肢肪到茎苗茂迭迫邸阻附斉甚帥衷幽為盾卑哀亭帝侯俊侵促俗盆冠削勅貞卸厘怠叙咲垣契姻孤封峡峠弧悔恒恨怒威括挟拷挑施是冒架枯柄柳皆洪浄津洞牲狭狩珍某疫柔砕窃糾耐胎胆胞臭荒荘虐訂赴軌逃郊郎香剛衰畝恋倹倒倣俸倫翁兼准凍剣剖脅匿栽索桑唆哲埋娯娠姫娘宴宰宵峰貢唐徐悦恐恭恵悟悩扇振捜挿捕敏核桟栓桃殊殉浦浸泰浜浮涙浪烈畜珠畔疾症疲眠砲祥称租秩粋紛紡紋耗恥脂朕胴致般既華蚊被託軒辱唇逝逐逓途透酌陥陣隻飢鬼剤竜粛尉彫偽偶偵偏剰勘乾喝啓唯執培堀婚婆寂崎崇崩庶庸彩患惨惜悼悠掛掘掲控据措掃排描斜旋曹殻貫涯渇渓渋淑渉淡添涼猫猛猟瓶累盗眺窒符粗粘粒紺紹紳脚脱豚舶菓菊菌虚蛍蛇袋訟販赦軟逸逮郭酔釈釣陰陳陶陪隆陵麻斎喪奥蛮偉傘傍普喚喫圏堪堅堕塚堤塔塀媒婿掌項幅帽幾廃廊弾尋御循慌惰愉惑雇扉握援換搭揚揺敢暁晶替棺棋棚棟款欺殖渦滋湿渡湾煮猶琴畳塁疎痘痢硬硝硫筒粧絞紫絡脹腕葬募裕裂詠詐詔診訴越超距軸遇遂遅遍酢鈍閑隅随焦雄雰殿棄傾傑債催僧慈勧載嗣嘆塊塑塗奨嫁嫌寛寝廉微慨愚愁慎携搾摂搬暇楼歳滑溝滞滝漠滅溶煙煩雅猿献痴睡督碁禍禅稚継腰艇蓄虞虜褐裸触該詰誇詳誉賊賄跡践跳較違遣酬酪鉛鉢鈴隔雷零靴頑頒飾飽鼓豪僕僚暦塾奪嫡寡寧腐彰徴憎慢摘概雌漆漸漬滴漂漫漏獄碑稲端箇維綱緒網罰膜慕誓誘踊遮遭酵酷銃銑銘閥隠需駆駄髪魂錬緯韻影鋭謁閲縁憶穏稼餓壊懐嚇獲穫潟轄憾歓環監緩艦還鑑輝騎儀戯擬犠窮矯響驚凝緊襟謹繰勲薫慶憩鶏鯨撃懸謙賢顕顧稿衡購墾懇鎖錯撮擦暫諮賜璽爵趣儒襲醜獣瞬潤遵償礁衝鐘壌嬢譲醸錠嘱審薪震錘髄澄瀬請籍潜繊薦遷鮮繕礎槽燥藻霜騒贈濯濁諾鍛壇鋳駐懲聴鎮墜締徹撤謄踏騰闘篤曇縄濃覇輩賠薄爆縛繁藩範盤罷避賓頻敷膚譜賦舞覆噴墳憤幣弊壁癖舗穂簿縫褒膨謀墨撲翻摩磨魔繭魅霧黙躍癒諭憂融慰窯謡翼羅頼欄濫履離慮寮療糧隣隷霊麗齢擁露 --format lvgl -o ../src/main/fonts/lv_font_notosans_j_16.c`
* Unfortunately, I mounted the camera upside down. That means I need to rotate the image. I use the hardware accelerated rotation for this. I extracted the code from the camera class and adapted it to do the rotation. The drawback is that I need a separate buffer fitting the raw image. Rotation in software would be very slow. The alternative is to bypass the lvgl drawing and send the image directly to the TFT with the command to rotate it on the TFT hardware. In this case, nothing can be drawn on top of the camera image by lvgl. This is the fastest by. 3rd option would be to leave it upside down and fix it later when creating the enclosure.

**Audio Module**
* starts recording on MQTT and button for n seconds where n is given by MQTT or is a default of 10 seconds, encodes into opus and saves the audio to file to SD card (switched off) and sends it via MQTT.

The code is separated into the following parts that I modified to encapsulate functionality in a reusable manner.
* audio_module.cpp high-level control and connection to other modules
* audio_pcm_capture.cpp takes care of capturing data from the microphone based on the audio_pcm_capture_objif example.
* audio_OpusAudioEncoder.h takes care of the interface to libopus
* audio_ogg.cpp takes care of wrapping the results into the Ogg container format.

The issue with the stream server is that my LTE-M SIM contract only allows connections from the device and the device is not remotely accessible without additional options that would cost more than 20 times the monthly cost. Therefore I used MQTT to send the audio stream.

To reduce bandwidth I use the OPUS codec. Feedback from Sony was that OPUS is not recommended due to the high demand for computing performance for real-time encoding. The result is that it works in real-time with the lowest settings of complexity 0 using VOIP with a frame size of 20ms on 48KHz mono audio. But it is hard on the limit. I needed to compile libopus without float point API because this works too slowly even though the Spresense has a float point unit.

I initially learned how to integrate libopus with the Audio libraries by Phil Schatzmann [ [GitHub - pschatzmann/arduino-libopus: The Opus Codec for Arduino](https://github.com/pschatzmann/arduino-libopus) ].

**Other Functionality that is not based on events**
* I use the ArduinoLog library for logging. The benefit is that I can set log levels, a common prefix and suffix like a timestamp and that I can use printf like style with an automatic newline or no newline. However, this library is not safe to use on multiple cores in parallel as the MPLog function. So I can only switch on logs on one core at a time. I did not succeed because ArduinoLog uses MPLog as a backend. This seems to be more complex and likely needs an adapter class derived from Printable
* I use the ArduinoShell library to enable custom shell commands via Serial which I use to send all kinds of events with any data. This is used for testing. With the event-based architecture, I can simulate any events and can therefore test the specific behaviour of modules in isolation.
* In the main.ino I initialize the RTC library that is fed by the time received from LTE and is used for the logging timestamps and in the filename of any file I save on SD like images, audio, location and status information as JSON.
* The Low-power library is initialized to access the voltage level of the battery.

## Measuring Energy Consumption
Keep in mind that I made no energy consumption optimizations and have several peripherals attached. This makes it difficult to conclude this measurement.

I tried to measure the energy by connecting a Nordic Semiconductor Power Profiler Kit II in source power mode to the battery input of the Spresense board. It seems like the LTE-M module uses high energy spikes which the PPK II could not handle. As a result, the board resets while attaching to the LTE-M network. To lower the current, I decided to raise the input voltage to 4.2V which is within the specification of the Spresense board. It was then stable in single CPU mode, but not for the final firmware with SMP. What worked is using the USB input connecting only power at 5V.

1:26 Minutes of operation incl. sending audio data via LTE took 189.96mA on average with a max of 0.60A. While only streaming the camera and reading GPS it was 184.50mA on average, while searching for the phone 248.49mA, while transmitting audio data 249.46mA. Cutting off the display power reduced by 21mA.
![Energy Consumption Measurement](/doc/images/EnergyConsumptionMeasurement.png)

## Misc information
some further information in the GitHub repository
* My original requirements are in doc/Requirements.txt. Just for reference what my thoughts were at the start of the project.
* The diagrams are available as a drawio file inside the doc folder.
* A list of shell commands is in doc/commands.txt (might not be up to date, but mostly)
* patch files for library modifications (GNSS, MemoryUitl, SDK config) are inside the patches folder. Also configuration file for lvgl and TFT_eSPI modifications.
* To analyze the memory consumption, it is good to know how much memory each compilation unit uses for code and statically allocated memory. I used the following command and a little editor multi-line edit to extract this information. The first line includes the length and the following line which file was compiled. For example, EventManager.cpp uses 0x000052b6 bytes (which is 21174 bytes, hmmm quite much). doc/memoryUsage.txt is an example of the final result. Usually, unnecessary large buffers waste memory.
```
dwarfdump  /Users/jens/work/mcu_prj/prj/spresense/prj/ArduinoOutput/main.ino.elf  --debug-info 2> /dev/null | grep -e "Compile Unit" -e "DW_AT_name.*Users"
```

```
0x00000000: Compile Unit: length = 0x000052b6, format = DWARF32, version = 0x0003, abbr_offset = 0x0000, addr_size = 0x04 (next unit at 0x000052ba)
              DW_AT_name	("/Users/jens/work/mcu_prj/prj/spresense/prj/ArduinoOutput/sketch/EventManager.cpp")
0x000052ba: Compile Unit: length = 0x0000e909, format = DWARF32, version = 0x0003, abbr_offset = 0x0c58, addr_size = 0x04 (next unit at 0x00013bc7)
              DW_AT_name	("/Users/jens/work/mcu_prj/prj/spresense/prj/ArduinoOutput/sketch/GlobalStatus.cpp")
....
```


## Future steps
Just a rough bullet point list of what is still planned to do
* Design a final housing and hardware design.
* Stabilize the application and do some long-term testing. More test cases.
* Maybe a larger display (with buttons or touch) and a redesign of the UI
* Instead of using SMP, putting the audio processing to a DSP (requires reverse engineering as the encoding DSP code is closed source and there is no skeleton.


## Licenses & Used Libraries and Sources
**Used Services**
*  [Yahoo! Japan API](https://developer.yahoo.co.jp/sitemap/) 
*  [AWS IoT](https://aws.amazon.com/iot/) 

**Source code based on or uses**

*  [sonydevworld](https://github.com/sonydevworld) / [spresense-arduino-compatible](https://github.com/sonydevworld/spresense-arduino-compatible)  LGPL2.1 Copyright 2018 Sony Semiconductor Solutions Corporation
*  [sonydevworld](https://github.com/sonydevworld) / [spresense](https://github.com/sonydevworld/spresense)  various licenses depending on which part(mostly) Copyright 2018 Sony Semiconductor Solutions Corporation
*  [igormiktor](https://github.com/igormiktor) / [arduino-EventManager](https://github.com/igormiktor/arduino-EventManager)  LGPL2.1Copyright (c) 2016 Igor Mikolic-TorreiraPortions are Copyright (c) 2010 OTTOTECNICA Italy
*  [notofonts](https://github.com/notofonts) / [noto-fonts](https://github.com/notofonts/noto-fonts)  [OFL-1.1 license](https://github.com/notofonts/noto-fonts/blob/main/LICENSE) Downloaded from Google  [googlefonts](https://github.com/googlefonts) / [noto-cjk](https://github.com/googlefonts/noto-cjk) 
*  [google](https://github.com/google) / [material-design-icons](https://github.com/google/material-design-icons)  [Apache-2.0 license](https://github.com/google/material-design-icons/blob/master/LICENSE) By Google

**Arduino Libraries**

*  [arduino-libraries](https://github.com/arduino-libraries) / [ArduinoBLE](https://github.com/arduino-libraries/ArduinoBLE) 
*  [bblanchon](https://github.com/bblanchon) / [ArduinoJson](https://github.com/bblanchon/ArduinoJson) Copyright © 2014-2022, Benoit BLANCHON
*  [thijse](https://github.com/thijse) / [Arduino-Log](https://github.com/thijse/Arduino-Log)  MIT LicenseCopyright (c) 2017, 2018, 2021 Thijs Elenbaas, MrRobot62, rahuldeo2047, NOX73, dhylands, Josha, blemasle, mfalkvidd
*  [Bodmer](https://github.com/Bodmer) / [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)  MIT License, BSD License, FreeBSD License)Check out the full license for the evolution of this library. The original starting point for this library was the Adafruit_ILI9341 library in January 2015...
*  [lvgl](https://github.com/lvgl) / [lvgl](https://github.com/lvgl/lvgl)  [MIT license](https://github.com/lvgl/lvgl/blob/master/LICENCE.txt) Many contributors. See  [https://lvgl.io/about](https://lvgl.io/about) 
*  [philj404](https://github.com/philj404) / [SimpleSerialShell](https://github.com/philj404/SimpleSerialShell)  [MIT license](https://github.com/philj404/SimpleSerialShell/blob/main/LICENSE) Copyright (c) 2020 philj404
*  [adafruit](https://github.com/adafruit) / [Adafruit_AHRS](https://github.com/adafruit/Adafruit_AHRS) Adafruit Industries.Copyright (c) 2014, Freescale Semiconductor, Inc.
*  [adafruit](https://github.com/adafruit) / [Adafruit_Sensor](https://github.com/adafruit/Adafruit_Sensor) Adafruit Industries.Copyright (C) 2008 The Android Open Source Project
*  [adafruit](https://github.com/adafruit) / [Adafruit_LSM303DLHC](https://github.com/adafruit/Adafruit_LSM303DLHC)  BSD licenseWritten by Kevin Townsend for Adafruit Industries.
*  [adafruit](https://github.com/adafruit) / [Adafruit_BMP085_Unified](https://github.com/adafruit/Adafruit_BMP085_Unified)  BSD licenseWritten by Kevin Townsend for Adafruit Industries.
*  [adafruit](https://github.com/adafruit) / [Adafruit_L3GD20_U](https://github.com/adafruit/Adafruit_L3GD20_U)  BSD licenseWritten by Kevin Townsend for Adafruit Industries.

**3rd party libraries**

* [xiph.org](https://xiph.org/ogg/) / [libogg](https://github.com/gcp/libogg.git) BSD-3-Clause license
* [xiph.org](https://xiph.org/ogg/) / [liboggz](https://github.com/kfish/liboggz.git) BSD-3-Clause license
* [opus-codec.org](https://opus-codec.org) / [opus](https://gitlab.xiph.org/xiph/opus.git) BSD-3-Clause license [See details here](https://opus-codec.org/license/)
* [Amazon Web Services](https://github.com/aws) / [aws-iot-device-sdk-embedded-C](https://github.com/aws) MIT license
* [Eclipse Foundation](https://github.com/eclipse) / [paho.mqtt.embedded-c](https://github.com/eclipse/paho.mqtt.embedded-c) Dual licensed under the EPL and EDL

**Documentation tooling**

* [Robot Framework ry](https://robotframework.org) / [Robot Framework](https://github.com/robotframework/robotframework) Apache License 2.0
* [StrictDoc](https://strictdoc.readthedocs.io/en/stable/) /  [strictdoc-project/strictdoc](https://github.com/strictdoc-project/strictdoc) Apache License 2.0
* [Plantuml](https://plantuml.com) /  [Download](https://plantuml.com/download) Many license versions -> MIT License Version

**Stack overflow**

*  [MrTJ](https://stackoverflow.com/users/1242446/mrtj) :  [Calculating the distance between 2 latitudes and longitudes that are saved in a text file?](https://stackoverflow.com/questions/10198985/calculating-the-distance-between-2-latitudes-and-longitudes-that-are-saved-in-a) 
*  [Paco Valdez](https://gis.stackexchange.com/users/4842/paco-valdez) :  [Calculate bearing between two decimal GPS coordinates](https://gis.stackexchange.com/questions/29239/calculate-bearing-between-two-decimal-gps-coordinates) 
*  [cmm](https://ja.stackoverflow.com/users/32034/cmm) :  [spresense LTE拡張ボードでLTEとADを使うとLTE.shutdown()でエラーになる](https://ja.stackoverflow.com/questions/74425/spresense-lte%e6%8b%a1%e5%bc%b5%e3%83%9c%e3%83%bc%e3%83%89%e3%81%a7lte%e3%81%a8ad%e3%82%92%e4%bd%bf%e3%81%86%e3%81%a8lte-shutdown%e3%81%a7%e3%82%a8%e3%83%a9%e3%83%bc%e3%81%ab%e3%81%aa%e3%82%8b) 
*  [Colin](https://stackoverflow.com/users/2097041/colin) :  [Fastest way to swap alternate bytes on ARM Cortex M4 using gcc](https://stackoverflow.com/questions/41675438/fastest-way-to-swap-alternate-bytes-on-arm-cortex-m4-using-gcc) 

**Other websites**

* [benzo energy](http://www.benzoenergy.com/blog/author/author-benzo.html) :  [What is the relationship between voltage and capacity of 18650 li-ion battery?](http://www.benzoenergy.com/blog/post/what-is-the-relationship-between-voltage-and-capacity-of-18650-li-ion-battery.html) 

**Support at Sony Developer World forum**

*  [CamilaSouza](https://forum.developer.sony.com/user/camilasouza) 
*  [yokonav](https://forum.developer.sony.com/user/yokonav)

Thanks to [Bryan Staley](https://www.hackster.io/stanton) and [Brayden Staley](https://www.hackster.io/cyxl) for answering about SMP usage.
