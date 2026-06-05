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


#include "awe_comm.h"

#include "awosal_string.h"
#include "awosal_time.h"
#include "awe_comm_internal.h"

#include "awe_comm_common_cfg.h"
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    #include "awe_comm_socket_cfg.h"
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    #include "awe_comm_aweq_cfg.h"
#endif

#include "awe_comm_backend_selector.h"
#include "awe_comm_logging.h"
#include "awe_config.h"


/* ****************************************************************************
 * TYPE DEFINITIONS (NON PUBLIC) - POSSIBLY BACKEND SPECIFIC
 * ***************************************************************************/

/* ****************************************************************************
 * LOCAL FUNCTIONS
 * ***************************************************************************/

#define FAIL_ON_PTR(x) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!", #x); return AWECOMM_RC_FAIL_PARAM; }
#define FAIL_ON_CHN(ch) if(ch < 0 || ch >= MAX_AWECOMM_CHANNELS) { \
        AWE_COMM_LOGE("Invalid argument: channel incorrect: must be 0 < channel < %d!", MAX_AWECOMM_CHANNELS); \
        return AWECOMM_RC_FAIL_PARAM; }

 // helper function to update the timeout value
static void timeout_value_changed(const char* key, const char* value, const char* description, void* context)
{
    struct awecomm_data* ctrl_p = (struct awecomm_data*) context;

    aweconfig_get_as_int(ctrl_p->config, CFG_COMM_TIMEOUT, &ctrl_p->timeout_ms);
    if (ctrl_p->timeout_ms < 0) {
        AWE_COMM_LOGI("Timeout value set to negative value, will wait indefinitely for responses");
    }
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int awecomm_register_configs(awe_config* cfg_p)
{
    FAIL_ON_PTR(cfg_p);

    int rc = AWECFG_RC_OK;
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    rc = register_comm_backend_socket_configs(cfg_p);
#endif
#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    rc = register_comm_backend_aweq_configs(cfg_p);
#endif

    aweconfig_init_tuple common_configs [] = {
        {CFG_COMM_TIMEOUT, DEFAULT_COMM_TIMEOUT, "Number of milliseconds to wait for response (negative value = wait indefinitely)", NULL, NULL},
        {CFG_COMM_BUFFER_SIZE, DEFAULT_TUNEMSG_SIZE_IN_WORDS, "Size of Tuning packet buffer", NULL, NULL},
    };

    rc = aweconfig_add_multiple(cfg_p, common_configs, sizeof(common_configs)/sizeof(common_configs[0]));

    if (rc == AWECFG_RC_OK)
    {
        rc = awecomm_trace_register_configs(cfg_p);
    }
    return (rc != AWECFG_RC_OK) ? AWECOMM_RC_FAIL_PARAM : AWECOMM_RC_OK;
}


int awecomm_init(awe_config* cfg_p, aweevt_listener cb, void *userdata_p, struct awecomm_data **ctrl_pp)
{
    int rc = AWECOMM_RC_OK;

    FAIL_ON_PTR(cfg_p);
    FAIL_ON_PTR(ctrl_pp);

    // allocation of the component structure itself
    struct awecomm_data* ctrl_p = (struct awecomm_data *) calloc(1, sizeof(struct awecomm_data));
    if(ctrl_p == NULL)
    {
        AWE_COMM_LOGE("Memory allocation problem!");
        return AWECOMM_RC_FAIL_RESOURCES;
    }

    // create a protection mutex for the channel data; this is used by all backends, so create it here; if backend specific mutexes are needed, they can be created in the backend_data and handled by the backend
    ctrl_p->channel_protection_mutex = awosal_create_mutex();
    if (ctrl_p->channel_protection_mutex == NULL)
    {
        AWE_COMM_LOGE("Failed to create channel protection mutex");
        free(ctrl_p);
        return AWECOMM_RC_FAIL_RESOURCES;
    }

    if(awecomm_trace_init(&ctrl_p->trace_cfg, cfg_p) != 0)
    {
        AWE_COMM_LOGE("Failed to initialize trace configuration");
        free(ctrl_p);
        return AWECOMM_RC_FAIL_RESOURCES;
    }

    uint32_t tuningBufferSize = 264;
    aweconfig_get_as_uint(cfg_p, CFG_COMM_BUFFER_SIZE, &tuningBufferSize);

    AWE_COMM_LOGI("Tuning Buffer size = %u words", tuningBufferSize);
    for (int i=0; i<MAX_AWECOMM_CHANNELS; i++)
    {
        int cmd_rc = awecmd_init(&(ctrl_p->channel_data[i]), tuningBufferSize, false, tuningBufferSize);
        if (cmd_rc != AWECMD_RC_OK)
        {
            AWE_COMM_LOGE("Memory allocation problem for command buffer! idx=%d", i);
            free(ctrl_p);
            // note: theoretically we might have a memory leak here for already initialized channels,
            // but as this is a fatal error, we can ignore it and just free the main struct
            return AWECOMM_RC_FAIL_RESOURCES;
        }
    }

    ctrl_p->config = cfg_p;

    ctrl_p->rx_event_cb = cb;
    ctrl_p->rx_event_cb_ctx = userdata_p;

    aweconfig_get_as_int(cfg_p, CFG_COMM_TIMEOUT, &ctrl_p->timeout_ms);

    awe_comm_backend* pBackend = create_backend(cfg_p, cb, userdata_p);
    if(!pBackend)
    {
        AWE_COMM_LOGE("Could not create communication backend!");
        free(ctrl_p);
        return AWECOMM_RC_FAIL_RESOURCES;
    }
    ctrl_p->backend = pBackend;

    rc = pBackend->init(pBackend);
    if (rc < 0)
    {
        pBackend->exit(pBackend);
        free(ctrl_p);
        AWE_COMM_LOGE("Communication to AWECore not available");
        return AWECOMM_RC_FAIL_COMM;
    }

    (void) aweconfig_add_listener(cfg_p, CFG_COMM_TIMEOUT, timeout_value_changed, ctrl_p);

    *ctrl_pp = ctrl_p;
    return rc;
}

int awecomm_exit(struct awecomm_data **ctrl_pp)
{
    FAIL_ON_PTR(ctrl_pp);

    struct awecomm_data *ctrl_p = *ctrl_pp;

    if(!ctrl_p)
    {
        AWE_COMM_LOGD("*ctrl_pp already NULL. No memory to be released.");
        return AWECOMM_RC_OK;
    }

    (void) aweconfig_remove_listener(ctrl_p->config, CFG_COMM_TIMEOUT);

    awe_comm_backend* pBackend = ctrl_p->backend;
    pBackend->exit(pBackend);

    for (int i=0; i<MAX_AWECOMM_CHANNELS; i++)
    {
        awecmd_exit(&(ctrl_p->channel_data[i]));
    }

    awecomm_trace_exit(&ctrl_p->trace_cfg);

    free(ctrl_p);
    *ctrl_pp = NULL;
    return AWECOMM_RC_OK;
}

int awecomm_transact_on_cmdbuf(struct awecomm_data *ctrl_p, struct awecmd_st *buf_p)
{
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_PTR(buf_p);

    return awecomm_transact_explicit(ctrl_p, buf_p->buffer_p, buf_p->words_written,
        buf_p->response_buffer_p, buf_p->response_buffer_size, 0);  // channel
}

int awecomm_transact(struct awecomm_data *ctrl_p, int channel)
{
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_CHN(channel);

    struct awecmd_st *buf_p = &(ctrl_p->channel_data[channel]);

    return awecomm_transact_on_cmdbuf(ctrl_p, buf_p);
}

int awecomm_transact_explicit(struct awecomm_data *ctrl_p, void* data, int data_sz, void* response, int response_sz, int chn)
{
    int ret = 0;
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_PTR(data);
    FAIL_ON_PTR(response);

    awe_comm_backend* pBackend = ctrl_p->backend;

    // write data to "AWE Core"
    int rc = pBackend->write(pBackend, data, data_sz, ctrl_p->timeout_ms);
    if (rc != AWECOMM_RC_OK)
    {
        AWE_COMM_LOGE("failed to send data on channel %d, rc %d", chn, rc);
        return AWECOMM_RC_FAIL_COMM;
    }

    // dump data content to logging if tracing enabled
    awecomm_trace_dump(&ctrl_p->trace_cfg, chn, "TX", NULL, data, data_sz);

    // ************************************************************************
    // wait for response from channel buffer

    int nr_words_read = 0;
    ret = pBackend->read(pBackend, response, response_sz, &nr_words_read, ctrl_p->timeout_ms);

    if (ret != AWECOMM_RC_OK)
    {
        AWE_COMM_LOGE("Failed to read response from AWE Core, ret=%d", ret);
        return ret;
    }

    // dump data content to logger if enabled
    awecomm_trace_dump(&ctrl_p->trace_cfg, 0, "RX", NULL, response, nr_words_read);

    return AWECOMM_RC_OK;
}

int awecomm_notify(struct awecomm_data *ctrl_p, int channel)
{
    FAIL_ON_PTR(ctrl_p);

    // for AWEMGR_AWECORE_CONNECTION_SOCKET: nothing to do!

    #if 0 // def AWEMGR_AWECORE_CONNECTION_CSHMEM
    if (ctrl_p->com_channel_hash_table[channel].used == CH_STS_USED)
    {
        awe_packet_trigger_ipcc(ctrl_p->com_channel_hash_table[channel].fd);
    }
    #endif

    return AWECOMM_RC_OK;
}

int awecomm_acquire_lock(struct awecomm_data *ctrl_p)
{
    FAIL_ON_PTR(ctrl_p);

    int rc = ctrl_p->channel_protection_mutex->lock(ctrl_p->channel_protection_mutex, ctrl_p->timeout_ms);
    switch (rc)
    {
        case 0:
            // success
            break;
        case AWEOSAL_RC_TIMEOUT:
            AWE_COMM_LOGE("Timeout after %d ms while acquiring channel protection mutex", ctrl_p->timeout_ms);
            // return still failed resource allocation, the timeout is for when the other side "misbehaves"
            return AWECOMM_RC_FAIL_RESOURCES;
        default:
            AWE_COMM_LOGE("Error while acquiring channel protection mutex, rc=%d", rc);
            return AWECOMM_RC_FAIL_RESOURCES;
    }

    return AWECOMM_RC_OK;
}

int awecomm_release_lock(struct awecomm_data *ctrl_p)
{
    FAIL_ON_PTR(ctrl_p);

    int rc = ctrl_p->channel_protection_mutex->unlock(ctrl_p->channel_protection_mutex);
    if (rc != 0)
    {
        AWE_COMM_LOGE("Could not unlock channel protection mutex");
        return AWECOMM_RC_FAIL_RESOURCES;
    }

    return AWECOMM_RC_OK;
}

int awecomm_get_cmdbuf(struct awecomm_data *ctrl_p, int tunnel_address, struct awecmd_st **cmdbuf_pp)
{
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_PTR(cmdbuf_pp);

    // we can't put the lock inside the channel_data buffers,
    // as we end up with ONE SINGLE buffer (in shared memory) anyhow,
    // this would mean tunnel-cmds and normal cmds would possibly collide
    int rc = awecomm_acquire_lock(ctrl_p);
    if (rc != AWECOMM_RC_OK)
    {
        return rc;
    }

    int channel_idx = (tunnel_address != 0) ? 1 : 0;
    *cmdbuf_pp = &(ctrl_p->channel_data[channel_idx]);

    if (tunnel_address) {
        awecmd_reset(*cmdbuf_pp, tunnel_address);
    }

    return AWECOMM_RC_OK;
}

void awecomm_set_observer(struct awecomm_data *ctrl_p, awecomm_buf_observer_cb cb, void *ctx)
{
    if (!ctrl_p)
        return;

    awecomm_trace_set_observer(&ctrl_p->trace_cfg, cb, ctx);
}
