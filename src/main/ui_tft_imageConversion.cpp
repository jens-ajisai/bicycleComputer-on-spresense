/****************************************************************************
 * boards/arm/cxd56xx/common/src/cxd56_imageproc.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/semaphore.h>

#include <debug.h>

#include <arch/chip/ge2d.h>
#include <arch/chip/chip.h>
#include <arch/board/cxd56_imageproc.h>

#include "chip.h"

#include <common/arm_arch.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_IMAGEPROC_GEDEVNAME
#define GEDEVNAME CONFIG_IMAGEPROC_GEDEVNAME
#else
#define GEDEVNAME "/dev/ge"
#endif

#define CXD56_ROT_BASE      (CXD56_ADSP_BASE + 0x02101400)
#define ROT_INTR_STATUS     (CXD56_ROT_BASE  + 0x0000)
#define ROT_INTR_ENABLE     (CXD56_ROT_BASE  + 0x0004)
#define ROT_INTR_DISABLE    (CXD56_ROT_BASE  + 0x0008)
#define ROT_INTR_CLEAR      (CXD56_ROT_BASE  + 0x000C)
#define ROT_SET_DIRECTION   (CXD56_ROT_BASE  + 0x0014)
#define ROT_SET_SRC_HSIZE   (CXD56_ROT_BASE  + 0x0018)
#define ROT_SET_SRC_VSIZE   (CXD56_ROT_BASE  + 0x001C)
#define ROT_SET_SRC_ADDRESS (CXD56_ROT_BASE  + 0x0020)
#define ROT_SET_SRC_PITCH   (CXD56_ROT_BASE  + 0x0024)
#define ROT_SET_DST_ADDRESS (CXD56_ROT_BASE  + 0x0028)
#define ROT_SET_DST_PITCH   (CXD56_ROT_BASE  + 0x002C)
#define ROT_CONV_CTRL       (CXD56_ROT_BASE  + 0x0034)
#define ROT_RGB_ALIGNMENT   (CXD56_ROT_BASE  + 0x0038)
#define ROT_COMMAND         (CXD56_ROT_BASE  + 0x0010)

/****************************************************************************
 * Private Data
 ****************************************************************************/

static bool g_imageprocinitialized = false;
static sem_t g_rotwait;
static sem_t g_rotexc;

static struct file g_gfile;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int ip_semtake(sem_t * id)
{
  return nxsem_wait_uninterruptible(id);
}

static void ip_semgive(sem_t * id)
{
  nxsem_post(id);
}

static int intr_handler_rot(int irq, FAR void *context, FAR void *arg)
{
  putreg32(1, ROT_INTR_CLEAR);
  putreg32(0, ROT_INTR_ENABLE);
  putreg32(1, ROT_INTR_DISABLE);

  ip_semgive(&g_rotwait);

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void my_imageproc_initialize(void)
{
//  printf("!! enter my_imageproc_initialize\n");
  if (g_imageprocinitialized)
    {
      return;
    }

  g_imageprocinitialized = true;

  nxsem_init(&g_rotexc, 0, 1);
  nxsem_init(&g_rotwait, 0, 0);
  nxsem_set_protocol(&g_rotwait, SEM_PRIO_NONE);

  cxd56_ge2dinitialize(GEDEVNAME);

  file_open(&g_gfile, GEDEVNAME, O_RDWR);

  putreg32(1, ROT_INTR_CLEAR);
  putreg32(0, ROT_INTR_ENABLE);
  putreg32(1, ROT_INTR_DISABLE);

  irq_attach(CXD56_IRQ_ROT, intr_handler_rot, NULL);
  up_enable_irq(CXD56_IRQ_ROT);
//  printf("!! exit my_imageproc_initialize\n");
}

void my_imageproc_finalize(void)
{
  if (!g_imageprocinitialized)
    {
      return;
    }

  up_disable_irq(CXD56_IRQ_ROT);
  irq_detach(CXD56_IRQ_ROT);

  if (g_gfile.f_inode)
    {
      file_close(&g_gfile);
    }

  cxd56_ge2duninitialize(GEDEVNAME);

  nxsem_destroy(&g_rotwait);
  nxsem_destroy(&g_rotexc);

  g_imageprocinitialized = false;
}

int my_imageproc_rotate(uint8_t* ibuf,
                        uint8_t* obuf,
                        uint32_t hsize,
                        uint32_t vsize)
{
  int ret;

  if (!g_imageprocinitialized)
    {
      return -EPERM;
    }

  if ((hsize & 1) || (vsize & 1))
    {
      return -EINVAL;
    }

  ret = ip_semtake(&g_rotexc);
  if (ret)
    {
      return ret;
    }

  /* Image processing hardware want to be set horizontal/vertical size
   * to actual size - 1.
   */

  --hsize;
  --vsize;

  putreg32(1, ROT_INTR_ENABLE);
  putreg32(0, ROT_INTR_DISABLE);

  putreg32(hsize, ROT_SET_SRC_HSIZE);
  putreg32(vsize, ROT_SET_SRC_VSIZE);
  putreg32(CXD56_PHYSADDR(ibuf), ROT_SET_SRC_ADDRESS);

  putreg32(hsize, ROT_SET_SRC_PITCH);
  putreg32(CXD56_PHYSADDR(obuf), ROT_SET_DST_ADDRESS);

  putreg32(hsize, ROT_SET_DST_PITCH);

  putreg32(0, ROT_CONV_CTRL);     // 0:NOCONVERT 1:YCBCR422_RGB565 2:RGB565_YCBCR422
  putreg32(0, ROT_RGB_ALIGNMENT); // 0:RGB; 1:BGR
  putreg32(2, ROT_SET_DIRECTION); // No rotation  Right 90 degrees  Right 180 degrees   Right 270 degrees
  putreg32(1, ROT_COMMAND);       // start rotation

  ip_semtake(&g_rotwait);

  ip_semgive(&g_rotexc);

  return 0;
}
