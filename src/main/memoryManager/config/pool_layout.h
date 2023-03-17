/* This file is generated automatically. */
/****************************************************************************
 * pool_layout.h
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

#ifndef POOL_LAYOUT_H_INCLUDED
#define POOL_LAYOUT_H_INCLUDED

#include "memutils/memory_manager/MemMgrTypes.h"

namespace MemMgrLite {

MemPool*  static_pools_block[NUM_MEM_SECTIONS][NUM_MEM_POOLS];
MemPool** static_pools[NUM_MEM_SECTIONS] = {
  static_pools_block[0],
};
uint8_t layout_no[NUM_MEM_SECTIONS] = {
  BadLayoutNo,
};
uint8_t pool_num[NUM_MEM_SECTIONS] = {
  NUM_MEM_S0_POOLS,
};
extern const PoolSectionAttr MemoryPoolLayouts[NUM_MEM_SECTIONS][NUM_MEM_LAYOUTS][9] = {
  {  /* Section:0 */
    {/* Layout:0 */
     /* pool_ID                          type         seg  fence  addr        size         */
      { S0_INPUT_BUF_POOL              , BasicType  ,   5,  true, 0x0d160008, 0x00002580 },  /* AUDIO_WORK_AREA */
      { S0_PRE_APU_CMD_POOL            , BasicType  ,   3,  true, 0x0d162590, 0x00000114 },  /* AUDIO_WORK_AREA */
      { S0_AUDIO_FIFO_POOL             , BasicType  ,   1,  true, 0x0d1626b0, 0x0000c314 },  /* AUDIO_WORK_AREA */
      { S0_DISPLAY_BUF_POOL            , BasicType  ,   1,  true, 0x0d16e9d0, 0x00001c20 },  /* AUDIO_WORK_AREA */
      { S0_MQTT_MSG_BUF_POOL           , BasicType  ,   4,  true, 0x0d1705f8, 0x00001e00 },  /* AUDIO_WORK_AREA */
      { S0_NET_BUF_POOL                , BasicType  ,   1,  true, 0x0d172400, 0x00001000 },  /* AUDIO_WORK_AREA */
      { S0_OPUS_BUF_POOL               , BasicType  ,   1,  true, 0x0d173408, 0x00001784 },  /* AUDIO_WORK_AREA */
      { S0_FREE_POOL                   , BasicType  ,   1,  true, 0x0d174b98, 0x0000959c },  /* AUDIO_WORK_AREA */
      { S0_NULL_POOL, 0, 0, false, 0, 0 },
    },
  },
}; /* end of MemoryPoolLayouts */

}  /* end of namespace MemMgrLite */

#endif /* POOL_LAYOUT_H_INCLUDED */
