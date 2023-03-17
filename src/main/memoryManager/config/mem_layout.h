/* This file is generated automatically. */
/****************************************************************************
 * mem_layout.h
 *
 *   Copyright 2023 Sony Semiconductor Solutions Corporation
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

#ifndef MEM_LAYOUT_H_INCLUDED
#define MEM_LAYOUT_H_INCLUDED

/*
 * Memory devices
 */

/* AUD_SRAM: type=RAM, use=0x00020000, remainder=0x00000000 */

#define AUD_SRAM_ADDR  0x0d160000
#define AUD_SRAM_SIZE  0x00020000

/*
 * Fixed areas
 */

#define AUDIO_WORK_AREA_ALIGN   0x00000008
#define AUDIO_WORK_AREA_ADDR    0x0d160000
#define AUDIO_WORK_AREA_DRM     0x0d160000 /* _DRM is obsolete macro. to use _ADDR */
#define AUDIO_WORK_AREA_SIZE    0x0001ed00

#define MSG_QUE_AREA_ALIGN   0x00000008
#define MSG_QUE_AREA_ADDR    0x0d17ed00
#define MSG_QUE_AREA_DRM     0x0d17ed00 /* _DRM is obsolete macro. to use _ADDR */
#define MSG_QUE_AREA_SIZE    0x00001000

#define MEMMGR_WORK_AREA_ALIGN   0x00000008
#define MEMMGR_WORK_AREA_ADDR    0x0d17fd00
#define MEMMGR_WORK_AREA_DRM     0x0d17fd00 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_WORK_AREA_SIZE    0x00000200

#define MEMMGR_DATA_AREA_ALIGN   0x00000008
#define MEMMGR_DATA_AREA_ADDR    0x0d17ff00
#define MEMMGR_DATA_AREA_DRM     0x0d17ff00 /* _DRM is obsolete macro. to use _ADDR */
#define MEMMGR_DATA_AREA_SIZE    0x00000100

/*
 * Memory Manager max work area size
 */

#define S0_MEMMGR_WORK_AREA_ADDR  MEMMGR_WORK_AREA_ADDR
#define S0_MEMMGR_WORK_AREA_SIZE  0x000000b0

/*
 * Section IDs
 */

#define SECTION_NO0       0

/*
 * Number of sections
 */

#define NUM_MEM_SECTIONS  1

/*
 * Pool IDs
 */

const MemMgrLite::PoolId S0_NULL_POOL                = { 0, SECTION_NO0};  /*  0 */
const MemMgrLite::PoolId S0_INPUT_BUF_POOL           = { 1, SECTION_NO0};  /*  1 */
const MemMgrLite::PoolId S0_PRE_APU_CMD_POOL         = { 2, SECTION_NO0};  /*  2 */
const MemMgrLite::PoolId S0_AUDIO_FIFO_POOL          = { 3, SECTION_NO0};  /*  3 */
const MemMgrLite::PoolId S0_DISPLAY_BUF_POOL         = { 4, SECTION_NO0};  /*  4 */
const MemMgrLite::PoolId S0_MQTT_MSG_BUF_POOL        = { 5, SECTION_NO0};  /*  5 */
const MemMgrLite::PoolId S0_NET_BUF_POOL             = { 6, SECTION_NO0};  /*  6 */
const MemMgrLite::PoolId S0_OPUS_BUF_POOL            = { 7, SECTION_NO0};  /*  7 */
const MemMgrLite::PoolId S0_FREE_POOL                = { 8, SECTION_NO0};  /*  8 */

#define NUM_MEM_S0_LAYOUTS   1
#define NUM_MEM_S0_POOLS     9

#define NUM_MEM_LAYOUTS      1
#define NUM_MEM_POOLS        9

/*
 * Pool areas
 */

/* Section0 Layout0: */

#define MEMMGR_S0_L0_WORK_SIZE   0x000000b0

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_INPUT_BUF_POOL_ALIGN    0x00000008
#define S0_L0_INPUT_BUF_POOL_L_FENCE  0x0d160004
#define S0_L0_INPUT_BUF_POOL_ADDR     0x0d160008
#define S0_L0_INPUT_BUF_POOL_SIZE     0x00002580
#define S0_L0_INPUT_BUF_POOL_U_FENCE  0x0d162588
#define S0_L0_INPUT_BUF_POOL_NUM_SEG  0x00000005
#define S0_L0_INPUT_BUF_POOL_SEG_SIZE 0x00000780

#define S0_L0_PRE_APU_CMD_POOL_ALIGN    0x00000008
#define S0_L0_PRE_APU_CMD_POOL_L_FENCE  0x0d16258c
#define S0_L0_PRE_APU_CMD_POOL_ADDR     0x0d162590
#define S0_L0_PRE_APU_CMD_POOL_SIZE     0x00000114
#define S0_L0_PRE_APU_CMD_POOL_U_FENCE  0x0d1626a4
#define S0_L0_PRE_APU_CMD_POOL_NUM_SEG  0x00000003
#define S0_L0_PRE_APU_CMD_POOL_SEG_SIZE 0x0000005c

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_AUDIO_FIFO_POOL_ALIGN    0x00000008
#define S0_L0_AUDIO_FIFO_POOL_L_FENCE  0x0d1626ac
#define S0_L0_AUDIO_FIFO_POOL_ADDR     0x0d1626b0
#define S0_L0_AUDIO_FIFO_POOL_SIZE     0x0000c314
#define S0_L0_AUDIO_FIFO_POOL_U_FENCE  0x0d16e9c4
#define S0_L0_AUDIO_FIFO_POOL_NUM_SEG  0x00000001
#define S0_L0_AUDIO_FIFO_POOL_SEG_SIZE 0x0000c314

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_DISPLAY_BUF_POOL_ALIGN    0x00000008
#define S0_L0_DISPLAY_BUF_POOL_L_FENCE  0x0d16e9cc
#define S0_L0_DISPLAY_BUF_POOL_ADDR     0x0d16e9d0
#define S0_L0_DISPLAY_BUF_POOL_SIZE     0x00001c20
#define S0_L0_DISPLAY_BUF_POOL_U_FENCE  0x0d1705f0
#define S0_L0_DISPLAY_BUF_POOL_NUM_SEG  0x00000001
#define S0_L0_DISPLAY_BUF_POOL_SEG_SIZE 0x00001c20

#define S0_L0_MQTT_MSG_BUF_POOL_ALIGN    0x00000008
#define S0_L0_MQTT_MSG_BUF_POOL_L_FENCE  0x0d1705f4
#define S0_L0_MQTT_MSG_BUF_POOL_ADDR     0x0d1705f8
#define S0_L0_MQTT_MSG_BUF_POOL_SIZE     0x00001e00
#define S0_L0_MQTT_MSG_BUF_POOL_U_FENCE  0x0d1723f8
#define S0_L0_MQTT_MSG_BUF_POOL_NUM_SEG  0x00000004
#define S0_L0_MQTT_MSG_BUF_POOL_SEG_SIZE 0x00000780

#define S0_L0_NET_BUF_POOL_ALIGN    0x00000008
#define S0_L0_NET_BUF_POOL_L_FENCE  0x0d1723fc
#define S0_L0_NET_BUF_POOL_ADDR     0x0d172400
#define S0_L0_NET_BUF_POOL_SIZE     0x00001000
#define S0_L0_NET_BUF_POOL_U_FENCE  0x0d173400
#define S0_L0_NET_BUF_POOL_NUM_SEG  0x00000001
#define S0_L0_NET_BUF_POOL_SEG_SIZE 0x00001000

#define S0_L0_OPUS_BUF_POOL_ALIGN    0x00000008
#define S0_L0_OPUS_BUF_POOL_L_FENCE  0x0d173404
#define S0_L0_OPUS_BUF_POOL_ADDR     0x0d173408
#define S0_L0_OPUS_BUF_POOL_SIZE     0x00001784
#define S0_L0_OPUS_BUF_POOL_U_FENCE  0x0d174b8c
#define S0_L0_OPUS_BUF_POOL_NUM_SEG  0x00000001
#define S0_L0_OPUS_BUF_POOL_SEG_SIZE 0x00001784

/* Skip 0x0004 bytes for alignment. */

#define S0_L0_FREE_POOL_ALIGN    0x00000008
#define S0_L0_FREE_POOL_L_FENCE  0x0d174b94
#define S0_L0_FREE_POOL_ADDR     0x0d174b98
#define S0_L0_FREE_POOL_SIZE     0x0000959c
#define S0_L0_FREE_POOL_U_FENCE  0x0d17e134
#define S0_L0_FREE_POOL_NUM_SEG  0x00000001
#define S0_L0_FREE_POOL_SEG_SIZE 0x0000959c

/* Remainder AUDIO_WORK_AREA=0x00000bc8 */

#endif /* MEM_LAYOUT_H_INCLUDED */
