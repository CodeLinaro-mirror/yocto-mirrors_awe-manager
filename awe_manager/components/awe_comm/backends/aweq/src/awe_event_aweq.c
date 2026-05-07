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
#include <sys/time.h>

#include "awe_shmem_config.h" // for getting correct AWE_MANAGER_AREA_OFFSET size
// important, as otherwise a possibly incorrect definition from /sysroot will be used

#include "awe_event_backend.h"
#include "awe_comm_aweq_cfg.h"
#include "awe_comm_logging.h"

// further includes from platform
#include "awe_audiolite_osal.h"
#include "awe_evt_shm_head.h"


#define FAIL_ON_PTR(x) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!", #x); return -3; }
#define AWE_SECOND_US                (1000 * 1000)
#define AWE_MS_US                    1000
#define SHM_WAIT_TM_US               (10 * AWE_MS_US)

/* Set value in share memory, maybe need to call awe_audiolite_buffer_cache_flush */
#define AWEMGR_SH_SET(dst, src)        {(dst) = (src); awe_audiolite_buffer_cache_flush(&(dst), sizeof(dst));}
/* Get value in share memory, maybe need to call awe_audiolite_buffer_cache_inval */
#define AWEMGR_SH_GET(val)             ({awe_audiolite_buffer_cache_inval(&val, sizeof(val));val;})

typedef struct evtshm_data
{
    void *awemgr_share;
}
evtshm_data;

static awe_wait_int_st s_wait_int[AWEMGR_PLAT_CNT];
static int arm_transmit_index = 0;

extern int awe_alsa_get_buffer_addr(struct mem_attr *mem, uint32_t size);

/* Wait interrupt form AWE core */
static int awemgr_wait_notify_sig(uint32_t timeoutUs, uint64_t *int_mask)
{
    struct timeval timeout;

    timeout.tv_sec = timeoutUs/AWE_SECOND_US;
    timeout.tv_usec = timeoutUs%AWE_SECOND_US;
    if (int_mask != NULL)
    {
        *int_mask = 0;
    }

    return awe_wait_multi_int_tm(s_wait_int,
        (s_wait_int[AWEMGR_PLAT_ARM].fd > 0) ? AWEMGR_PLAT_CNT : AWEMGR_PLAT_ARM,
        &timeout, int_mask);
}

static long time_diff_us(struct timeval *start, struct timeval *end)
{
    long diff_seconds = end->tv_sec - start->tv_sec;
    long diff_useconds = end->tv_usec - start->tv_usec;

    return diff_seconds * AWE_SECOND_US + diff_useconds;
}

int aweqshm_evt_init(awe_evt_backend* bkend)
{
    int i;
    int platform;
    FAIL_ON_PTR(bkend);
    evtshm_data* pData = (evtshm_data*)bkend->platform_data;
    FAIL_ON_PTR(pData);
    awemgr_share_mem_t *awe_manager_sh;

    // Initialize the shared buffer address
    struct mem_attr mem = {0};
    int err = awe_alsa_get_buffer_addr(&mem, 0);
    if (err != 0)
    {
        AWE_COMM_LOGE("awe_alsa_get_buffer_addr fail err:%d", err);
        return AWE_EVT_RC_FAIL_RESOURCE;
    }
    pData->awemgr_share = (mem.vaddr + AWE_MANAGER_AREA_OFFSET);

    AWE_COMM_LOGD("Shared Memory mapped addr:%p len:%d - event data offset:%d", mem.vaddr, mem.len, AWE_MANAGER_AREA_OFFSET);
    int cnt_providers = 0;
    for (platform = AWEMGR_PLAT_ADSP; platform < AWEMGR_PLAT_CNT; platform++)
    {
        awe_manager_sh = (awemgr_share_mem_t *)(pData->awemgr_share + (platform * AWE_MANAGER_BUF_SIZE));
        if (AWE_MGR_MAGIC != awe_manager_sh->magic)
        {
            AWE_COMM_LOGW("Magic word not found for core:%d id:0x%x - should be 0x%x", platform, awe_manager_sh->magic, AWE_MGR_MAGIC);
            continue;
        }
        AWE_COMM_LOGD("Event producer found. core:%d magic:0x%x", platform, awe_manager_sh->magic);
        AWEMGR_SH_SET(awe_manager_sh->state, AWEMGR_EV_ACTIVE);
        if (platform == AWEMGR_PLAT_ARM)
        {
            arm_transmit_index = AWEMGR_SH_GET(awe_manager_sh->transmit_index);
        }
        cnt_providers++;
    }

    if (cnt_providers == 0)
    {
        AWE_COMM_LOGE("No event producers found");
        return AWE_EVT_RC_FAIL_RESOURCE;
    }
    awe_audiolite_ipcc_init(VFIO_UMD_AUDIO_HLOS2);

    //Init the wait interrupt struct, The arm platform not support now
    memset(&s_wait_int[0], 0, sizeof(s_wait_int));
    for (i = AWEMGR_PLAT_ADSP; i < AWEMGR_PLAT_ARM; i++)
    {
        s_wait_int[i].apps_num = SOURCE_CLIENT;
        s_wait_int[i].sig_num = IPCC_SIG_AWE_MGR;
        s_wait_int[i].proto = SOURCE_CLIENT;
    }
    s_wait_int[AWEMGR_PLAT_ADSP].client_id = DESTINATION_CLIENT;
    s_wait_int[AWEMGR_PLAT_GPDSP0].client_id = IPCC_C_GPDSP0;
    s_wait_int[AWEMGR_PLAT_GPDSP1].client_id = IPCC_C_GPDSP1;

    s_wait_int[AWEMGR_PLAT_ARM].fd = awe_alsa_get_eventfd(AWE_EVENTFD_MGR);

    AWE_COMM_LOGI("Attached to AWE-Q event memory");
    return AWE_EVT_RC_OK;
}

int aweqshm_evt_exit(awe_evt_backend* bkend)
{
    awemgr_share_mem_t *awe_manager_sh;
    int platform;

    if(bkend == NULL)
    {
        AWE_COMM_LOGW("Backend already Null");
        return AWE_EVT_RC_OK;
    }

    evtshm_data* pData = (evtshm_data*)bkend->platform_data;
    if(pData != NULL)
    {
        for (platform = AWEMGR_PLAT_ADSP; platform < AWEMGR_PLAT_CNT; platform++)
        {
            awe_manager_sh = (awemgr_share_mem_t *)(pData->awemgr_share + (platform * AWE_MANAGER_BUF_SIZE));
            if (AWE_MGR_MAGIC != awe_manager_sh->magic)
            {
                continue;
            }
            AWEMGR_SH_SET(awe_manager_sh->state, AWEMGR_EV_INACTIVE);
        }

        // Add Shm related cleanup
        free(pData);
    }
    free(bkend);
    return AWE_EVT_RC_OK;
}

static inline int read_event_from_shm(awe_evt_backend* bkend, uint32_t timeoutMs)
{
    int ret;
    int platform;
    evtshm_data* pData = bkend->platform_data;
    awemgr_share_mem_t *awe_manager_sh;
    event_entry_st *ev;
    long timeoutUs = (long)timeoutMs * AWE_MS_US;
    long timeoutNow;
    struct timeval tv_start;
    struct timeval tv_end;
    uint64_t int_mask;
    int arm_transmit_index_cur;

    FAIL_ON_PTR(pData);
    gettimeofday(&tv_start, NULL);

    do
    {
        /* scan adsp,gpdsp0,gpdsp1,arm to find a platform have event */
        for (platform = AWEMGR_PLAT_ADSP; platform < AWEMGR_PLAT_CNT; platform++)
        {
            awe_manager_sh = (awemgr_share_mem_t *)(pData->awemgr_share
                + (platform * AWE_MANAGER_BUF_SIZE));
            awe_audiolite_buffer_cache_inval(&awe_manager_sh->magic,
                (void *)(&awe_manager_sh->event_buf[0]) - (void *)(&awe_manager_sh->magic));

            if ((AWE_MGR_MAGIC != awe_manager_sh->magic) ||
                (AWEMGR_EV_ACTIVE != awe_manager_sh->state))
            {
                continue;
            }

            if (awe_manager_sh->transmit_index != awe_manager_sh->receive_index)
            {
                break;
            }
        }

        if (platform >= AWEMGR_PLAT_CNT)
        {
            gettimeofday(&tv_end, NULL);
            timeoutNow = time_diff_us(&tv_end, &tv_start);
            if (timeoutNow >= timeoutUs)
            {
                return AWE_EVT_RC_TIMEOUT;
            }

            awe_manager_sh = (awemgr_share_mem_t *)(pData->awemgr_share
                + (AWEMGR_PLAT_ARM * AWE_MANAGER_BUF_SIZE));
            ret = awemgr_wait_notify_sig(min(SHM_WAIT_TM_US, timeoutUs - timeoutNow),
                &int_mask);
            /* Check whether awe_command has restarted. */
            if (AWE_MGR_MAGIC == awe_manager_sh->magic)
            {
                arm_transmit_index_cur = AWEMGR_SH_GET(awe_manager_sh->transmit_index);
                if (arm_transmit_index != arm_transmit_index_cur)
                {
                    if (!(int_mask & (1 << AWEMGR_PLAT_ARM)))
                    {
                        s_wait_int[AWEMGR_PLAT_ARM].fd = awe_alsa_get_eventfd(AWE_EVENTFD_MGR);
                    }
                    arm_transmit_index = arm_transmit_index_cur;
                    continue;
                }
            }

            if (ret < 0)
            {
                return AWE_EVT_RC_COMM_ERR;
            }
        }
    }
    while (platform >= AWEMGR_PLAT_CNT);

    uint32_t receive_index = awe_manager_sh->receive_index;
    if (awe_manager_sh->transmit_index != receive_index)
    {
        ev = &awe_manager_sh->event_buf[receive_index];
        awe_audiolite_buffer_cache_inval(ev, sizeof(event_entry_st));
        aweevent_header* pHdr = (aweevent_header*) &ev->hdr;  // WARNING: this relies on the fact that the event_entry_st is designed to have the aweevent_header as the first member, so that the header can be directly accessed from the event entry pointer. This is a common pattern in C, but it is important to ensure that this design is maintained for this code to work correctly.
        char* pBuff = (char*)(&ev->payload[0]);

        // Notify the listener (Awe Manager)
        AWE_COMM_LOGD("Dispatching event: instId=%d, objId=%d, classId=%d, evType=%d, evCat=%d,size=%d",
            pHdr->instanceId, pHdr->objectId, pHdr->classId, pHdr->eventType, pHdr->eventCategory, pHdr->dataSize);
        bkend->evt_notify(pHdr, pBuff, bkend->userdata);
        AWEMGR_SH_SET(awe_manager_sh->receive_index, (receive_index + 1)%AWEMGR_EVENT_BUF_LEN);
        return AWE_EVT_RC_OK;
    }

    AWE_COMM_LOGW("No events to read!");
    return AWE_EVT_RC_OK;
}

int aweqshm_evt_read(awe_evt_backend* bkend, uint32_t timeoutMs)
{
    FAIL_ON_PTR(bkend);
    return read_event_from_shm(bkend, timeoutMs);
}

awe_evt_backend* create_event_backend_aweq(awe_config *cfg_p, aweevt_listener cb, void* userdata)
{
    if (cb == NULL)
    {
        AWE_COMM_LOGE("Event listener can not be NULL");
        return NULL;
    }
    awe_evt_backend* pBkend = calloc(1, sizeof(awe_evt_backend));
    if (pBkend == NULL)
    {
        AWE_COMM_LOGE("Failed to allocate memory for Event Backend");
        return NULL;
    }
    evtshm_data* pData = (evtshm_data *) calloc(1, sizeof(evtshm_data));
    if(pData == NULL)
    {
        AWE_COMM_LOGE("Failed to allocate memory for platform data!");
        aweqshm_evt_exit(pBkend);
        return NULL;
    }
    pBkend->platform_data = pData;
    pBkend->evt_notify = cb;
    pBkend->userdata = userdata;
    pBkend->init = aweqshm_evt_init;
    pBkend->exit = aweqshm_evt_exit;
    pBkend->read_event = aweqshm_evt_read;
    return pBkend;
}
