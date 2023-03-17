#include "audio_module.h"

#include <RTC.h>
#include <sched.h>
#include <stdio.h> /* for sprintf */

#include "audio/audio_common_defs.h"
#include "audio_pcm_capture.h"
#include "audio_ogg.h"
#include "config.h"
#include "events/EventManager.h"
#include "events/audio_module_event.h"
#include "memoryManager/config/mem_layout.h"
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "my_log.h"
#include "sd_utils.h"

#include <ArduinoJson.h>

// I do not use MP3 anymore.
// https://developer.sony.com/develop/spresense/docs/arduino_developer_guide_en.html#_mp_memory
// I need 3 blocks with MEMORY_UTIL_TINY and MP3

static const uint32_t target_samplingrate = AS_SAMPLINGRATE_48000;
static const uint32_t target_channel_number = AS_CHANNEL_MONO;
static const uint32_t target_bit_length = AS_BITLENGTH_16;
static const uint32_t default_mic_gain = 210;

static const uint32_t opus_complexity = 0;

static int pollAudioThread = -1;
static bool initialized = false;

static int posInSamples = 0;

static File targetFile;

#include "audio_OpusAudioEncoder.h"
static OpusAudioEncoder *opusEncoder;

void initOpusEncoder(void) {
  OpusEncoderSettings cfg(target_channel_number, target_samplingrate);
  cfg.complexity = opus_complexity;
  cfg.frame_sizes_ms = OPUS_FRAMESIZE_20_MS;
  opusEncoder = new OpusAudioEncoder(cfg);
  Log.traceln("Expected encoder size %d", opusEncoder->getExpectedPackageSize());
}

#define OPUS_OUT_BUF_SIZE 128
uint8_t bufOpusOut[OPUS_OUT_BUF_SIZE];

// further step is to add ogg packages

#define OPUS_CHUNK_SIZE (1500)  // size of one chunk to send via MQTT
#define SIMPLE_FIFO_FRAME_NUM (3)
#define SIMPLE_FIFO_BUF_SIZE (OPUS_CHUNK_SIZE * SIMPLE_FIFO_FRAME_NUM)

struct fifo_info_s {
  CMN_SimpleFifoHandle handle;
  uint32_t fifo_area[SIMPLE_FIFO_BUF_SIZE / sizeof(uint32_t)];
  uint8_t write_buf[OPUS_CHUNK_SIZE];
};

fifo_info_s *s_opus_fifo;
MemMgrLite::MemHandle opusfifo_mh;


void send_recorded_data(bool is_end_process) {
  size_t opus_data_size = CMN_SimpleFifoGetOccupiedSize(&s_opus_fifo->handle);

  if (opus_data_size < OPUS_CHUNK_SIZE && !is_end_process) return;

  while (opus_data_size) {
    size_t send_size = (opus_data_size > OPUS_CHUNK_SIZE) ? OPUS_CHUNK_SIZE : opus_data_size;

    CMN_SimpleFifoPoll(&s_opus_fifo->handle, (void *)s_opus_fifo->write_buf, send_size);
    AudioModule.publish(new MqttEvent(MqttEvent::MQTT_EVT_SEND, MqttEvent::MQTT_TOPIC_AUDIO,
                                      s_opus_fifo->write_buf, send_size));

    opus_data_size -= send_size;
    if (!is_end_process) return;
  }
}

size_t ogg_write_cb(void *user_handle, void *buf, size_t n) {

  // Add all OPUS data to a FIFO to resize the frames. Only send chuks of 1500 Bytes.
  // When the recording process ended. Send all remaining Bytes.
  CMN_SimpleFifoOffer(&s_opus_fifo->handle, buf, n);
  send_recorded_data(false);
  return n;
}

static void app_pcm_output(uint8_t *buf, uint32_t size) {
  // OPUS START
  int sizeOut = 0;
  uint64_t start = millis();
  if (size == opusEncoder->getExpectedPackageSize()) {
    sizeOut = opusEncoder->encodeFrame(buf, size, bufOpusOut, OPUS_OUT_BUF_SIZE);
  } else {
    Log.warningln("No match to expected package size is %d should be %d", size,
                  opusEncoder->getExpectedPackageSize());
  }

  // Too slow for audio encoding, we have a semaphore to syncronize prints between CPUs
  //  Log.traceln("OPUS for %d -> %d took %l ms.", size, sizeOut, millis() - start);

  posInSamples += (opusEncoder->getExpectedPackageSize() / 2 / sizeof(int16_t) / target_channel_number);
  ogg_writeAudioDataPage(bufOpusOut, size, posInSamples);

  // skip writing to file
  return;

  int ret = targetFile.write(buf, size);
  if (ret < 0) {
    if (DEBUG_AUDIO) Log.errorln("File write error.");
  }
}

int pollAudio(int argc, FAR char *argv[]) {
  int duration = atoi(argv[1]);
  AudioModule.publish(new AudioEvent(AudioEvent::AUDIO_EVT_AUDIO));
  if (DEBUG_AUDIO) Log.traceln("Start recording to for %d", duration);

  initOpusEncoder();

  if (CMN_SimpleFifoInitialize(&s_opus_fifo->handle, s_opus_fifo->fifo_area, SIMPLE_FIFO_BUF_SIZE,
                               NULL) != 0) {
    if (DEBUG_AUDIO) Log.errorln("Error: Fail to initialize simple FIFO for opus.");
    return false;
  }
  CMN_SimpleFifoClear(&s_opus_fifo->handle);

  posInSamples = 0;
  ogg_begin(ogg_write_cb);
  ogg_writeHeaderPage(target_channel_number, target_samplingrate);  

  process_recorder(duration, app_pcm_output);
  stop_recorder(app_pcm_output);

  delete opusEncoder;

  ogg_writeFooterPage();
  ogg_end();

  send_recorded_data(true);

  targetFile.close();
  AudioModule.publish(new AudioEvent(AudioEvent::AUDIO_EVT_AUDIO_FINISHED));

  pollAudioThread = -1;
  return 0;
}

extern SDClass theSD;

void AudioModuleClass::startRecording(uint16_t duration) {
  if (initialized) {
    if (pollAudioThread < 0) {
      char filename[20] = {0};
      RtcTime now = RTC.getTime();
      sprintf(filename, "audio/AUDIO%10lu.pcm", now.unixtime());

      targetFile = theSD.open(filename, FILE_WRITE);
      if (!targetFile) {
        if (DEBUG_AUDIO) Log.errorln("File open error for Audio");
        return;
      }

      if (DEBUG_AUDIO) Log.traceln("Start recording to %s", filename);
      start_recorder();

      const char *argv[2];
      char str[8];
      snprintf(str, 8, "%u", duration);

      argv[0] = str;
      argv[1] = NULL;

      pollAudioThread = task_create("pollAudio", SCHED_PRIORITY_DEFAULT,
                                    CONFIG_AUDIO_MODULE_STACK_SIZE, pollAudio, (char *const *)argv);
      assert(pollAudioThread > 0);
#ifdef CONFIG_SMP
      cpu_set_t cpuset = 1 << CPU_AFFINITY_AUDIO;
      sched_setaffinity(pollAudioThread, sizeof(cpu_set_t), &cpuset);
#endif
    }
  }
}

void AudioModuleClass::init() {
  if (!initialized) {
    initialize_recorder(target_samplingrate, target_channel_number, target_bit_length,
                        default_mic_gain);
    initialized = true;
  }
}

void AudioModuleClass::deinit() {
  if (initialized) {
    deinit_recorder();
    if (pollAudioThread) {
      task_delete(pollAudioThread);
      pollAudioThread = -1;
    }
    initialized = false;
  }
}

void AudioModuleClass::handleButton(ButtonEvent *ev) {
  if (ev->getCommand() == ButtonEvent::BUTTON_EVT_RELEASED) {
    startRecording();
  }
}

void AudioModuleClass::handleAudio(AudioEvent *ev) {
  if (ev->getCommand() == AudioEvent::AUDIO_EVT_GET_AUDIO) {
    startRecording(ev->getDuration());
  } else if (ev->getCommand() == AudioEvent::AUDIO_EVT_STOP_AUDIO) {
    stop_recorder(app_pcm_output);
  }
}

void AudioModuleClass::handleAppManager(AppManagerEvent *ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
//      deinit();
    }
  }
}

void AudioModuleClass::handleMqtt(MqttEvent* ev) {
  if (ev->getCommand() == MqttEvent::MQTT_EVT_RECV &&
      ev->getTopic() == MqttEvent::MQTT_TOPIC_GET_AUDIO) {
    // we expect a JSON like { "duration": 3 }
    // to jens/feeds/spresense.getAudio
    DynamicJsonDocument doc(128);
    deserializeJson(doc, ev->getMessage());

    publish(new AudioEvent(AudioEvent::AUDIO_EVT_GET_AUDIO, doc["duration"]));        
  }
}

void AudioModuleClass::eventHandler(Event *event) {
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent *>(event));
      break;

    case Event::kEventUiButton:
      handleButton(static_cast<ButtonEvent *>(event));
      break;

    case Event::kEventAudio:
      handleAudio(static_cast<AudioEvent *>(event));
      break;

    case Event::kEventMqtt:
      handleMqtt(static_cast<MqttEvent*>(event));
      break;

    default:
      break;
  }
}

bool AudioModuleClass::begin() {
  if (DEBUG_AUDIO) Log.traceln("AudioModuleClass::begin");
  bool ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventAudio, this);
  ret = ret && eventManager.addListener(Event::kEventMqtt, this);

  // The button is already used to send images
  //  ret = ret && eventManager.addListener(Event::kEventUiButton, this);

  if (opusfifo_mh.allocSeg(S0_OPUS_BUF_POOL, sizeof(fifo_info_s))) {
    Log.errorln("Failed to allocate memory for opus buffer");
    return 0;
  }
  s_opus_fifo = (fifo_info_s *)opusfifo_mh.getPa();

  return ret;
}

AudioModuleClass AudioModule;
