/* MIT License
**
** Copyright (c) 2024 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/
#ifndef __AWE_SHMEM_CONFIG_H__
#define __AWE_SHMEM_CONFIG_H__

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <StandardDefs.h>

#define LISTEN_PORT                         (15002)
#define AWE_TUNING_LOADAWBFILE_FLAG         (0x5A5A5A5A)
#define AWE_SHMEM_FLAG_CLEAR                (0x00000000)


/*
    length: 10          4096
            [flags_area][packetbuffer_area]
*/
#define AWE_MAX_COMMAND_BUFFER_LEN          (4096)
#define AWE_MSG_BUFFER_BYTE_LEN             (AWE_MAX_COMMAND_BUFFER_LEN * sizeof(UINT32)) /* 4096 * 4*/
#define AWE_TUNING_FLAGS_NUM                (16)

#define AWE_MANAGER_BUF_SIZE                  (32 * 1024)
#define AWE_MANAGER_BUF_CNT                   (4)

#define SHMEM_SIZE                           (0x2000000)  /* 32MB */
#define AWE_SHMEM_SIZE                       (0x1600000)  /* 22MB */

/*DSPC PAYLOAD AREA SIZE*/
#define AWE_TUNING_FLAGS_AREA_SIZE          (AWE_TUNING_FLAGS_NUM * sizeof(UINT32)) /* 16 words */
#define AWE_SYNC_STATE_FLAG_SIZE            (4 * sizeof(uint32_t))
#define AWE_TUNING_PACKETBUFFER_AREA_SIZE   (AWE_MSG_BUFFER_BYTE_LEN)   /* 4096 words */
#define AWE_TUNING_PAYLOAD_AREA_SIZE        (AWE_TUNING_FLAGS_AREA_SIZE+AWE_TUNING_PACKETBUFFER_AREA_SIZE) /* 4096 + 16 words */
#define AWE_ALSA_IO_AREA_SIZE               (0x100000)  /* 1MB */
#define AWE_SHARED_HEAP_AREA_SIZE           (0x400000)  /* 4MB */
#define AWE_DSP_DEBUG_AREA_SIZE             (0X300000)  /* 3MB */
#define AWE_CONFIG_LOAD_AREA_SIZE           (0xA00000)  /* 10MB */
#define AWE_MANAGER_TOTAL_BUF_SIZE          (AWE_MANAGER_BUF_SIZE * AWE_MANAGER_BUF_CNT) /* 128KB */
#define AWE_TDM_IO_AREA_SIZE                (0x010000)  /* 64KB */
#define AWE_CHIME_REQ_MSG_SIZE              (0x000400)  /* 1KB */
#define AWE_CHIME_RSP_MSG_SIZE              (0x000400)  /* 1KB */
#define AWE_AVSYNC_TEST_DATA_SIZE           (0x001000)  /* 4KB */
#define DSPC_PAYLOAD_AREA_SIZE              (AWE_TUNING_FLAGS_AREA_SIZE+\
                                             AWE_TUNING_PACKETBUFFER_AREA_SIZE+\
                                             AWE_ALSA_IO_AREA_SIZE+\
                                             AWE_SHARED_HEAP_AREA_SIZE+\
                                             AWE_DSP_DEBUG_AREA_SIZE+\
                                             AWE_CONFIG_LOAD_AREA_SIZE+\
                                             AWE_MANAGER_TOTAL_BUF_SIZE+\
                                             AWE_TDM_IO_AREA_SIZE+\
                                             AWE_CHIME_REQ_MSG_SIZE+\
                                             AWE_CHIME_RSP_MSG_SIZE+\
                                             AWE_AVSYNC_TEST_DATA_SIZE) /* Total size */


/*DSPC PAYLOAD OFFSET*/
#define DSPC_PAYLOAD_AREA_OFFSET            (0X00000000)
#define AWE_TUNING_PAYLOAD_OFFSET           (DSPC_PAYLOAD_AREA_OFFSET)
#define AWE_TUNING_FLAGS_AREA_OFFSET        (AWE_TUNING_PAYLOAD_OFFSET)
#define AWE_TUNING_START_FLAG_OFFSET        (AWE_TUNING_FLAGS_AREA_OFFSET+(1*sizeof(UINT32)))
#define AWE_TUNING_LOADAWBFILE_FLAG_OFFSET  (AWE_TUNING_FLAGS_AREA_OFFSET+(2*sizeof(UINT32)))
#define AWE_SAIL_DSP_MSG_FLAG_OFFSET        (AWE_TUNING_FLAGS_AREA_OFFSET+(3*sizeof(UINT32)))
#define AWE_INSTANCE_INIT_FLAG_OFFSET       (AWE_TUNING_FLAGS_AREA_OFFSET+(5*sizeof(uint32_t)))
#define AWE_DSP_FUNCTION_FLAG_OFFSET        (AWE_TUNING_FLAGS_AREA_OFFSET+(6*sizeof(uint32_t)))
#define AWE_DSP_REASSIGN_FLAG_OFFSET        (AWE_TUNING_FLAGS_AREA_OFFSET+(7*sizeof(uint32_t)))
#define AWE_TUNING_PACKET_FLAG_OFFSET       (AWE_TUNING_FLAGS_AREA_OFFSET+(8*sizeof(UINT32)))
// 4 words (16 bytes) reserved for sync flags
#define AWE_SYNC_STATE_FLAG_OFFSET          (AWE_TUNING_FLAGS_AREA_OFFSET+(9*sizeof(uint32_t)))
#define AWE_SYNC_STATE_FLAG_END_OFFSET      (AWE_TUNING_FLAGS_AREA_OFFSET+(12*sizeof(uint32_t)))

#define AWE_TUNING_PACKETBUFFER_AREA_OFFSET (AWE_TUNING_FLAGS_AREA_OFFSET+AWE_TUNING_FLAGS_AREA_SIZE)
#define AWE_ALSA_IO_AREA_OFFSET             (AWE_TUNING_PACKETBUFFER_AREA_OFFSET+AWE_TUNING_PACKETBUFFER_AREA_SIZE)
#define AWE_SHARED_HEAP_AREA_OFFSET         (AWE_ALSA_IO_AREA_OFFSET+AWE_ALSA_IO_AREA_SIZE)
#define AWE_DSP_DEBUG_AREA_OFFSET           (AWE_SHARED_HEAP_AREA_OFFSET+AWE_SHARED_HEAP_AREA_SIZE)
#define AWE_CONFIG_LOAD_AREA_OFFSET         (AWE_DSP_DEBUG_AREA_OFFSET+AWE_DSP_DEBUG_AREA_SIZE)
#define AWE_MANAGER_AREA_OFFSET             (AWE_CONFIG_LOAD_AREA_OFFSET+AWE_CONFIG_LOAD_AREA_SIZE)
#define AWE_TDM_IO_AREA_OFFSET              (AWE_MANAGER_AREA_OFFSET+AWE_MANAGER_TOTAL_BUF_SIZE)
#define AWE_CHIME_REQ_MSG_OFFSET            (AWE_TDM_IO_AREA_OFFSET+AWE_TDM_IO_AREA_SIZE)
#define AWE_CHIME_RSP_MSG_OFFSET            (AWE_CHIME_REQ_MSG_OFFSET+AWE_CHIME_REQ_MSG_SIZE)
#define AWE_AVSYNC_TEST_TDM_TICKS_OFFSET    (AWE_CHIME_RSP_MSG_OFFSET+AWE_CHIME_RSP_MSG_SIZE)
#define DSPC_PAYLOAD_END_OFFSET             (DSPC_PAYLOAD_AREA_OFFSET+DSPC_PAYLOAD_AREA_SIZE)

/* other shared mem partition */
#define SHMEM_CMD_AREA_SIZE                (0x100000)   /* 1MB  */
#define SHMEM_CMD_OFFSET                   (DSPC_PAYLOAD_END_OFFSET)

#define AWE_ARM_INSTANCE_ID_OFFSET          3
#define AWE_GPDSP0_INSTANCE_ID_OFFSET       2
#define AWE_GPDSP1_INSTANCE_ID_OFFSET       1
#define AWE_ADSP_INSTANCE_ID_OFFSET         0
#define AWE_INSTANCE_ID_INIT                (0xEE)
#define AWE_INSTANCE_ID_MASK                (0x0F)

#define AWE_ARM_INSTANCE_NUMBEER_SHIFT      4
#define AWE_ARM_INSTANCE_ID_SHIFT           0

#ifndef SUPPORT_CORES_4
#define SUPPORT_CORES_4                     (4)
#endif

#ifndef DISABLE_CORE
#define DISABLE_CORE                        (0xFF)
#endif

#define AWE_ALL_DSP_ACTIVE                  (0)
#define AWE_ONLY_DSP0_ACTIVE                (1)
#define AWE_DSP0_DSP1_ACTIVE                (2)
#define AWE_DSP0_DSP2_ACTIVE                (3)

#define AWE_DSP_REASSIGN_COMPLETE           (0x4F564552)

typedef enum {
    AWE_SYNC_STATE_CLEAN = 0x53434C4E, // "SCLN"
    AWE_SYNC_STATE_READY = 0x53524459, // "SRDY"
} awe_sync_state_t;

#endif