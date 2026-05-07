/* MIT License
**
** Copyright (c) 2026 DSP Concepts, Inc.
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

#include "awe_comm_backend.h"
#include "awosal_string.h"
#include "awe_comm_logging.h"
#include "awe_config.h"
#include "awe_comm.h"

#include "awe_comm_aweq_cfg.h"
#include "awe_comm_common_cfg.h"


// further platform includes
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "awe_packet_client.h"
#include "awe_alsa.h"



#define FAIL_ON_PTR(x, msg) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!: %s", #x, msg); return AWECOMM_RC_FAIL_PARAM; }

typedef struct _com_channel_st
{
    int used;
    int fd;
    unsigned int app_id;
} com_channel_st;

#define MAX_SUPPORT_CHANNEL_NUM (1)

enum channel_status
{
    CH_STS_UNUSED,
    CH_STS_USED,
};

// platform specific data for this backend
typedef struct aweqshm_data
{
    com_channel_st com_channel_hash_table[MAX_SUPPORT_CHANNEL_NUM];
    awe_config* config;

} aweqshm_data;


/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/
static void release_channel(aweqshm_data *ctx_p, int channel)
{
    awe_packet_deregister_client(ctx_p->com_channel_hash_table[channel].fd);
    ctx_p->com_channel_hash_table[channel].fd = -1;
    ctx_p->com_channel_hash_table[channel].app_id = 0;
    ctx_p->com_channel_hash_table[channel].used = CH_STS_UNUSED;
    AWE_COMM_LOGD("deinit channel %d ", channel);
}

static int register_channel(aweqshm_data *ctx_p, int channel)
{
    int ret = PKT_SUCCESS;
    int client_fd = -1;
    unsigned int app_id = 0;

    FAIL_ON_PTR(ctx_p, "no platform data ptr provided");

    /* current socket fd is invalid and register new socket */
    AWE_COMM_LOGI("reset channel %d", channel);

    ret = awe_packet_register_client(&client_fd, &app_id);
    if (ret == PKT_SUCCESS)
    {
        ctx_p->com_channel_hash_table[channel].fd = client_fd;
        ctx_p->com_channel_hash_table[channel].app_id = app_id;
        ctx_p->com_channel_hash_table[channel].used = CH_STS_USED;

        AWE_COMM_LOGI("retry to reregister client successfully, channel %d, app_id %x", channel, app_id);
    }
    else
    {
        AWE_COMM_LOGI("failed to reregister client, channel %d", channel);
        return AWECOMM_RC_FAIL_COMM;
    }

    return AWECOMM_RC_OK;
}

/* **************************************************************************** */

static int aweqshm_init(awe_comm_backend* bkend)
{
    FAIL_ON_PTR(bkend, "backend state handle is required");
    aweqshm_data* pData = (aweqshm_data*)bkend->platform_data;
    FAIL_ON_PTR(pData, "platform data is required");

    int client_fd = -1;
    unsigned int app_id = 0;
    int i, ret = AWECOMM_RC_OK;

    ret = awe_alsa_init();
    if (ret)
    {
        AWE_COMM_LOGE("Failed to init EaseAudio's awe_alsa library, ret=%d", ret);
        return AWECOMM_RC_FAIL_RESOURCES;
    }

    for (i = 0; i < MAX_SUPPORT_CHANNEL_NUM; i++)
    {
        ret = awe_packet_register_client(&client_fd, &app_id);
        if (ret == PKT_SUCCESS)
        {
            pData->com_channel_hash_table[i].fd = client_fd;
            pData->com_channel_hash_table[i].app_id = app_id;
            pData->com_channel_hash_table[i].used = CH_STS_USED;

            AWE_COMM_LOGD("Register client successfully, channel %d, app_id %x", i, app_id);
        }
        else
        {
            AWE_COMM_LOGE("Register packet client failure, ret=%d", ret);
        }
    }
    AWE_COMM_LOGI("AWE-Q SHMEM control backend initialized");
    return AWECOMM_RC_OK;
}

static int aweqshm_exit(awe_comm_backend* bkend)
{
    if(bkend == NULL)
    {
        AWE_COMM_LOGW("AWE-Q SHMEM control backend already down");
        return AWECOMM_RC_OK;
    }
    aweqshm_data* pData = (aweqshm_data*)bkend->platform_data;
    if(pData != NULL)
    {
	    int i;
	    for (i = 0; i < MAX_SUPPORT_CHANNEL_NUM; i++)
	    {
	        if (pData->com_channel_hash_table[i].used == CH_STS_USED)
	        {
                release_channel(pData, i);
	            usleep(500);
	        }
	    }

    }
    free(bkend);
    AWE_COMM_LOGI("AWE-Q SHMEM control de-initialized");
    return AWECOMM_RC_OK;
}

static int aweqshm_write(awe_comm_backend* bkend, void* data, int data_sz_words, uint32_t timeoutMs)
{
    FAIL_ON_PTR(bkend, "backend state handle is required");
    aweqshm_data* pData = (aweqshm_data*)bkend->platform_data;
    FAIL_ON_PTR(pData, "platform data is required");

    int chn = 0; // currently we only support one channel, which is indexed at 0
    int ret;

    if (pData->com_channel_hash_table[chn].fd <= 0
        || pData->com_channel_hash_table[chn].used == CH_STS_UNUSED)
    {
        AWE_COMM_LOGI("channel %d invalid, reconnect to server", chn);
        ret = register_channel(pData, chn);
        if (ret != AWECOMM_RC_OK)
        {
            return AWECOMM_RC_FAIL_COMM;
        }
    }

    ret = awe_packet_client_write(pData->com_channel_hash_table[chn].fd, data, data_sz_words*sizeof(unsigned int));
    if (ret != PKT_SUCCESS)
    {
        AWE_COMM_LOGE("failed to send data on channel %d, ret %d", chn, ret);
        release_channel(pData, chn);
        return AWECOMM_RC_FAIL_COMM;
    }

    return AWECOMM_RC_OK;
}

static int aweqshm_read(awe_comm_backend* bkend, void* target_buffer, int target_buffer_sz_words, int* nr_words_read, uint32_t timeoutMs)
{
    aweqshm_data* pData = (aweqshm_data*)bkend->platform_data;

    int chn = 0; // currently we only support one channel, which is indexed at 0
    int ret;
    int nr_bytes_received;

    if (pData->com_channel_hash_table[chn].fd <= 0
        || pData->com_channel_hash_table[chn].used == CH_STS_UNUSED)
    {
        AWE_COMM_LOGE("channel %d invalid", chn);
        return AWECOMM_RC_FAIL_COMM;
    }

    ret = awe_packet_client_read(pData->com_channel_hash_table[chn].fd, target_buffer, target_buffer_sz_words*sizeof(unsigned int), &nr_bytes_received);
    if (ret != PKT_SUCCESS)
    {
        *nr_words_read = 0;
        AWE_COMM_LOGE("failed to receive data from packet on channel %d, ret %d\n", chn, ret);
        release_channel(pData, chn);
        return AWECOMM_RC_FAIL_COMM;
    }

    *nr_words_read = nr_bytes_received / sizeof(unsigned int);
    return AWECOMM_RC_OK;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

awe_comm_backend* create_comm_backend_aweq(awe_config *cfg_p, aweevt_listener cb, void* userdata)
{
    if (cfg_p == NULL) {
        AWE_COMM_LOGE("config object can not be NULL");
        return NULL;
    }

    if (cb == NULL) {
        AWE_COMM_LOGE("Event listener can not be NULL");
        return NULL;
    }
    awe_comm_backend* pBkend = (awe_comm_backend *) calloc(1, sizeof(awe_comm_backend));
    if (pBkend == NULL) {
        AWE_COMM_LOGE("Failed to allocate memory for Event Backend");
        return NULL;
    }

    aweqshm_data* pData = (aweqshm_data *) calloc(1, sizeof(aweqshm_data));
    if(pData == NULL)
    {
        AWE_COMM_LOGE("Failed to allocate memory for platform data!");
        aweqshm_exit(pBkend);
        return NULL;
    }
    // pData->current_timeout = 0;  // start with blocking mode

    pBkend->userdata = userdata;
    pBkend->platform_data = (void*)pData;
    pBkend->init = aweqshm_init;
    pBkend->exit = aweqshm_exit;
    pBkend->write = aweqshm_write;
    pBkend->read = aweqshm_read;
    pData->config = cfg_p;
    return pBkend;
}
