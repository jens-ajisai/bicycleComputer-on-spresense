
#include <common/arm_arch.h>
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

#include "memoryManager/config/mem_layout.h"
#include "memoryManager/config/pool_layout.h"

#include "memoryManager/config/msgq_id.h"
#include "memoryManager/config/msgq_pool.h"

#include "memoryManager/config/fixed_fence.h"

#include "audio_pcm_capture.h"

using namespace MemMgrLite;

static mpshm_t s_shm;

#define MEM_LAYOUT_PCM_CAPTURE (0)
#define AUDIO_SECTION SECTION_NO0

bool app_init_libraries(void) {
  int ret;

  err_t err = MsgLib::initFirst(NUM_MSGQ_POOLS, MSGQ_TOP_DRM);
  if (err != ERR_OK) {
    printf("Error: MsgLib::initFirst() failure. 0x%x\n", err);
    return false;
  }

  err = MsgLib::initPerCpu();
  if (err != ERR_OK) {
    printf("Error: MsgLib::initPerCpu() failure. 0x%x\n", err);
    return false;
  }

  void *mml_data_area = (void *) MEMMGR_DATA_AREA_ADDR;
  err = Manager::initFirst(mml_data_area, MEMMGR_DATA_AREA_SIZE);
  if (err != ERR_OK) {
    printf("Error: Manager::initFirst() failure. 0x%x\n", err);
    return false;
  }

  err = Manager::initPerCpu(mml_data_area, static_pools, pool_num, layout_no);
  if (err != ERR_OK) {
    printf("Error: Manager::initPerCpu() failure. 0x%x\n", err);
    return false;
  }

  /* Create static memory pool of VoiceCall. */

  const uint8_t sec_no = AUDIO_SECTION;
  const NumLayout layout_no = MEM_LAYOUT_PCM_CAPTURE;
  void *work_va = (void *) S0_MEMMGR_WORK_AREA_ADDR;
  const PoolSectionAttr *ptr = &MemoryPoolLayouts[AUDIO_SECTION][layout_no][0];
  err = Manager::createStaticPools(sec_no, layout_no, work_va, S0_MEMMGR_WORK_AREA_SIZE, ptr);
  if (err != ERR_OK) {
    printf("Error: Manager::createStaticPools() failure. %d\n", err);
    return false;
  }

  printf("Initialized shared memory at %x size %x\n", AUD_SRAM_ADDR, AUD_SRAM_SIZE);
  return true;
}

bool app_finalize_libraries(void) {
  /* Finalize MessageLib. */

  MsgLib::finalize();

  /* Destroy static pools. */

  MemMgrLite::Manager::destroyStaticPools(AUDIO_SECTION);

  /* Finalize memory manager. */

  MemMgrLite::Manager::finalize();

  /* Destroy shared memory. */


  return true;
}