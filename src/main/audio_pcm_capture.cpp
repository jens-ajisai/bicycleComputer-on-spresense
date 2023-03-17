/****************************************************************************
 * audio_pcm_capture/pcm_capture_main.cxx
 *
 *   Copyright 2019 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <stdio.h>
#include <strings.h>
#include <asmp/mpshm.h>
#include "memutils/simple_fifo/CMN_SimpleFifo.h"
#include "memutils/memory_manager/MemHandle.h"
#include "memutils/message/Message.h"
#include "audio/audio_frontend_api.h"
#include "audio/audio_capture_api.h"
#include "audio/audio_message_types.h"
#include "audio/utilities/frame_samples.h"

#include "memoryManager/config/msgq_id.h"
#include "memoryManager/config/mem_layout.h"

#include "audio_pcm_capture.h"

#include "my_log.h"

/****************************************************************************
 * Codec parameters
 ****************************************************************************/

#define USE_MIC_CHANNEL_NUM (1)

/* Set the PCM buffer size. */
#define READ_SIMPLE_FIFO_SIZE (1920) // 1920 is the expected size of the encoder
#define SIMPLE_FIFO_FRAME_NUM (24)

using namespace MemMgrLite;

#define SIMPLE_FIFO_BUF_SIZE (READ_SIMPLE_FIFO_SIZE * SIMPLE_FIFO_FRAME_NUM)
#define FIFO_STRUCT_SIZE 20

struct fifo_info_s {
  CMN_SimpleFifoHandle handle;
  uint32_t fifo_area[SIMPLE_FIFO_BUF_SIZE / sizeof(uint32_t)];
  uint8_t write_buf[READ_SIMPLE_FIFO_SIZE];
};

fifo_info_s* s_fifo;

MemMgrLite::MemHandle audiofifo_mh;

static volatile bool stopRecorder = false;


/****************************************************************************
 * Private Functions
 ****************************************************************************/

static bool app_init_simple_fifo(void) {
  if (CMN_SimpleFifoInitialize(&s_fifo->handle, s_fifo->fifo_area, SIMPLE_FIFO_BUF_SIZE, NULL) != 0) {
    if (DEBUG_AUDIO) Log.errorln("Error: Fail to initialize simple FIFO for audio.");
    return false;
  }
  CMN_SimpleFifoClear(&s_fifo->handle);

  if (DEBUG_AUDIO) Log.traceln("Size of s_fifo is %d", sizeof(s_fifo));

  return true;
}

static void app_pop_simple_fifo(bool is_end_process, ProcessOutputCb out_cb) {
  size_t occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&s_fifo->handle);
  uint32_t output_size = 0;

  while (occupied_simple_fifo_size > 0) {
    if(occupied_simple_fifo_size < 1920) return;
    output_size = (occupied_simple_fifo_size > READ_SIMPLE_FIFO_SIZE) ? READ_SIMPLE_FIFO_SIZE
                                                                      : occupied_simple_fifo_size;
    if (CMN_SimpleFifoPoll(&s_fifo->handle, (void *)s_fifo->write_buf, output_size) == 0) {
      if (DEBUG_AUDIO) Log.errorln("ERROR: Fail to get data from simple FIFO.");
      break;
    }

    // Too slow for audio encoding, we have a semaphore to syncronize prints between CPUs
    //    if (DEBUG_AUDIO) Log.traceln("FIFO usage %d", occupied_simple_fifo_size);
    
    // Process data by callback
    out_cb((uint8_t *)s_fifo->write_buf, output_size);

    if (is_end_process) {
      occupied_simple_fifo_size = CMN_SimpleFifoGetOccupiedSize(&s_fifo->handle);
    } else {
      occupied_simple_fifo_size -= output_size;
    }
  }
}

static bool app_receive_object_reply(uint32_t id = 0) {
  AudioObjReply reply_info;
  AS_ReceiveObjectReply(MSGQ_AUD_MGR, &reply_info);

  if (reply_info.type != AS_OBJ_REPLY_TYPE_REQ) {
    if (DEBUG_AUDIO) Log.errorln("app_receive_object_reply() error! type 0x%x", reply_info.type);
    return false;
  }

  if (id && reply_info.id != id) {
    if (DEBUG_AUDIO) Log.errorln("app_receive_object_reply() error! id 0x%lx(request id 0x%lx)", reply_info.id, id);
    return false;
  }

  if (reply_info.result != OK) {
    if (DEBUG_AUDIO) Log.errorln("app_receive_object_reply() error! result 0x%lx", reply_info.result);
    return false;
  }

  return true;
}

static bool app_create_audio_sub_system() {
  bool result = false;

  /* Create Frontend. */

  AsCreateMicFrontendParams_t frontend_create_param;
  frontend_create_param.msgq_id.micfrontend = MSGQ_AUD_FRONTEND;
  frontend_create_param.msgq_id.mng = MSGQ_AUD_MGR;
  frontend_create_param.msgq_id.dsp = MSGQ_AUD_PREDSP;
  frontend_create_param.pool_id.input = S0_INPUT_BUF_POOL;
  frontend_create_param.pool_id.output = S0_NULL_POOL;
  frontend_create_param.pool_id.dsp = S0_PRE_APU_CMD_POOL;

  AS_CreateMicFrontend(&frontend_create_param, NULL);

  /* Create Capture feature. */

  AsCreateCaptureParam_t capture_create_param;
  capture_create_param.msgq_id.dev0_req = MSGQ_AUD_CAP;
  capture_create_param.msgq_id.dev0_sync = MSGQ_AUD_CAP_SYNC;
  capture_create_param.msgq_id.dev1_req = 0xFF;
  capture_create_param.msgq_id.dev1_sync = 0xFF;

  result = AS_CreateCapture(&capture_create_param);
  if (!result) {
    if (DEBUG_AUDIO) Log.errorln("Error: As_CreateCapture() failure. system memory insufficient!");
    return false;
  }

  return true;
}

static void app_deact_audio_sub_system(void) {
  AS_DeleteMicFrontend();
  AS_DeleteCapture();
}

static bool app_power_on(void) { return (cxd56_audio_poweron() == CXD56_AUDIO_ECODE_OK); }

static bool app_power_off(void) { return (cxd56_audio_poweroff() == CXD56_AUDIO_ECODE_OK); }

static bool app_set_ready(void) {
  AsDeactivateMicFrontendParam deact_param;
  AS_DeactivateMicFrontend(&deact_param);

  if (!app_receive_object_reply(MSG_AUD_MFE_CMD_DEACT)) {
    /* To end processing */;
  }

  if (cxd56_audio_dis_input() != CXD56_AUDIO_ECODE_OK) {
    return false;
  }

  return true;
}

static bool app_init_mic_gain(int16_t gain) {
  cxd56_audio_mic_gain_t mic_gain;

  mic_gain.gain[0] = gain;
  mic_gain.gain[1] = gain;
  mic_gain.gain[2] = gain;
  mic_gain.gain[3] = gain;
  mic_gain.gain[4] = gain;
  mic_gain.gain[5] = gain;
  mic_gain.gain[6] = gain;
  mic_gain.gain[7] = gain;

  return (cxd56_audio_set_micgain(&mic_gain) == CXD56_AUDIO_ECODE_OK);
}

static bool app_set_frontend(void) {
  if (cxd56_audio_en_input() != CXD56_AUDIO_ECODE_OK) {
    return false;
  }

  AsActivateMicFrontend act_param;
  act_param.param.input_device = AsMicFrontendDeviceMic;
  act_param.cb = NULL;
  AS_ActivateMicFrontend(&act_param);

  if (!app_receive_object_reply(MSG_AUD_MFE_CMD_ACT)) {
    return false;
  }

  return true;
}

static bool app_init_micfrontend(uint32_t target_samplingrate, uint32_t target_channel_number,
                                 uint32_t target_bit_length) {
  AsInitMicFrontendParam init_param;

  init_param.channel_number = target_channel_number;
  init_param.bit_length = target_bit_length;
  init_param.samples_per_frame = getCapSampleNumPerFrame(AS_CODECTYPE_LPCM, target_samplingrate);
  init_param.preproc_type = AsMicFrontendPreProcThrough;
  init_param.data_path = AsDataPathSimpleFIFO;
  init_param.dest.simple_fifo_handler = &s_fifo->handle;

  AS_InitMicFrontend(&init_param);

  return app_receive_object_reply(MSG_AUD_MFE_CMD_INIT);
}

static bool app_start_capture(void) {
  stopRecorder = false;

  CMN_SimpleFifoClear(&s_fifo->handle);

  AsStartMicFrontendParam cmd;

  AS_StartMicFrontend(&cmd);

  return app_receive_object_reply(MSG_AUD_MFE_CMD_START);
}

static bool app_stop_capture(ProcessOutputCb out_cb) {
  stopRecorder = true;
  
  AsStopMicFrontendParam cmd;

  cmd.stop_mode = 0;

  AS_StopMicFrontend(&cmd);

  if (!app_receive_object_reply()) {
    return false;
  }

  /* Spout processing of remaining data. */

  app_pop_simple_fifo(true, out_cb);

  return true;
}

static bool app_set_clkmode(uint32_t target_samplingrate) {
  cxd56_audio_clkmode_t clk_mode;

  if (target_samplingrate == AS_SAMPLINGRATE_192000) {
    clk_mode = CXD56_AUDIO_CLKMODE_HIRES;
  } else {
    clk_mode = CXD56_AUDIO_CLKMODE_NORMAL;
  }

  return (cxd56_audio_set_clkmode(clk_mode) == CXD56_AUDIO_ECODE_OK);
}

static void app_recorde_process(uint32_t rec_time, ProcessOutputCb out_cb) {
  /* Timer Start */

  time_t start_time;
  time_t cur_time;

  time(&start_time);

  do {
    app_pop_simple_fifo(false, out_cb);

  } while (!stopRecorder && (time(&cur_time) - start_time) < rec_time);
}

static void app_disp_codec_params(uint32_t target_samplingrate, uint32_t target_channel_number,
                                  uint32_t target_bit_length) {
  if (DEBUG_AUDIO) Log.traceln("Sampling rate  %lu", target_samplingrate);
  if (DEBUG_AUDIO) Log.traceln("Channel number %lu", target_channel_number);
  if (DEBUG_AUDIO) Log.traceln("Bit length     %lu", target_bit_length);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

bool start_recorder(void) { return app_start_capture(); }
bool stop_recorder(ProcessOutputCb out_cb) { return app_stop_capture(out_cb); }
void process_recorder(uint32_t rec_time, ProcessOutputCb out_cb) {
  app_recorde_process(rec_time, out_cb);
}

int initialize_recorder(uint32_t target_samplingrate, uint32_t target_channel_number,
                        uint32_t target_bit_length, const uint32_t default_mic_gain) {
  if (DEBUG_AUDIO) Log.traceln("Initialize recorder");

  if (audiofifo_mh.allocSeg(S0_AUDIO_FIFO_POOL, SIMPLE_FIFO_BUF_SIZE + READ_SIMPLE_FIFO_SIZE + FIFO_STRUCT_SIZE)) {
    Log.errorln("Failed to allocate memory for camera rotation buffer");
    return 0;
  }
  s_fifo = (fifo_info_s*) audiofifo_mh.getPa();

  app_disp_codec_params(target_samplingrate, target_channel_number, target_bit_length);

  if (!app_create_audio_sub_system()) {
    if (DEBUG_AUDIO) Log.errorln("Error: act_audiosubsystem() failure.");
    goto errout_create_audio_sub_system;
  }

  // Change AudioSubsystem to Ready state so that I/O parameters can be changed.
  if (DEBUG_AUDIO) Log.traceln("Switch power on");
    if (!app_power_on()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_power_on() failure.");
    goto errout_power_on;
  }

  if (DEBUG_AUDIO) Log.traceln("Init simple fifo");
  if (!app_init_simple_fifo()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_init_simple_fifo() failure.");
    goto errout_init_simple_fifo;
  }

  if (DEBUG_AUDIO) Log.traceln("Init mic gain");
  if (!app_init_mic_gain(default_mic_gain)) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_init_mic_gain() failure.");
    goto errout_init_simple_fifo;
  }

  if (DEBUG_AUDIO) Log.traceln("Set clock mode");
  if (!app_set_clkmode(target_samplingrate)) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_set_clkmode() failure.");
    goto errout_init_simple_fifo;
  }

  if (DEBUG_AUDIO) Log.traceln("Set frontend");
  if (!app_set_frontend()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_set_frontend() failure.");
    goto errout_init_simple_fifo;
  }

  if (DEBUG_AUDIO) Log.traceln("Init mic frontend");
  if (!app_init_micfrontend(target_samplingrate, target_channel_number, target_bit_length)) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_init_frontend() failure.");
    goto errout_init_frontend;
  }

  if (DEBUG_AUDIO) Log.traceln("Recorder initialized");
  return 0;

errout_init_frontend:

  if (DEBUG_AUDIO) Log.traceln("Failed to initialize the recorder - do cleanup");

  if (!app_set_ready()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_set_ready() failure.");
  }

errout_init_simple_fifo:

  if (!app_power_off()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_power_off() failure.");
  }

errout_power_on:

  app_deact_audio_sub_system();

errout_create_audio_sub_system:

  if (DEBUG_AUDIO) Log.traceln("Failed to initialize the recorder - EXIT");

  return -1;
}

int deinit_recorder(void) {
  if (!app_set_ready()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_set_ready() failure.");
  }

  if (!app_power_off()) {
    if (DEBUG_AUDIO) Log.errorln("Error: app_power_off() failure.");
  }

  app_deact_audio_sub_system();

  return 0;
}