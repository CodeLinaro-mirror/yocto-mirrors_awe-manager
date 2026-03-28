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


#include "awe_ctrl.h"
#include "awosal_socket.h"
#include "awosal_string.h"
#include "awe_ctrl_logging.h"
#include "awe_config.h"


/* ****************************************************************************
 * TYPE DEFINITIONS (NON PUBLIC) - POSSIBLY BACKEND SPECIFIC
 * ***************************************************************************/

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
#ifdef AWOSAL_WINDOWS
#include <stdio.h>
#endif

#define CFG_CTRL_SOCKET_IP              "mgr.ctrl.socket.ip"
#define CFG_CTRL_SOCKET_PORT            "mgr.ctrl.socket.port"
#define CFG_CTRL_SOCKET_TIMEOUT         "mgr.ctrl.socket.timeoutms"

#define DEFAULT_CTRL_SOCKET_IP          "127.0.0.1"
#define DEFAULT_CTRL_SOCKET_PORT        "15002"
#define DEFAULT_CTRL_SOCKET_TIMEOUT     "2000"
#endif

#define CFG_CTRL_BUFFER_SIZE            "mgr.ctrl.buffersize"
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
#define DEFAULT_TUNEMSG_SIZE_IN_WORDS   "264"
#else
#define DEFAULT_TUNEMSG_SIZE_IN_WORDS   "4096"
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "awe_packet_client.h"
#include "awe_alsa.h"

// "packet server" specific types shall NOT be visible via Header file

typedef struct _com_channel_st
{
    int used;
    int fd;
    unsigned int app_id;
} com_channel_st;

#define MAX_SUPPORT_CHANNEL_NUM (16)

enum channel_status
{
    CH_STS_UNUSED,
    CH_STS_USED,
};

/**
 * close socket and reset the ctrl channel data
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 */
static void awectr_release_channel(struct awectrl_data *ctrl_p, int channel);

/**
 * register server socket according to the channel
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 */
static int awectrl_register_channel(struct awectrl_data *ctrl_p, int channel);
#endif

struct awectrl_data
{
    // backend specific handles and data
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    int s_hdl;
    int timeoutMs;
#endif
#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    com_channel_st com_channel_hash_table[MAX_SUPPORT_CHANNEL_NUM];
#endif

    // "channel" data; either dyn. allocated, or the buffer ptrs in awecmd_st are "set to shmem"
    // todo: avoid hard coded buf sizes, and have them as param to awe_CTRL init
    // todo: have awe_CTRL init have cfg param which sets ptrs into "shmem"
    struct awecmd_st channel_data[MAX_AWECTRL_COMM_CHANNELS];

    awe_config* config;
    // todo: mutex for protected channel access
};

/* ****************************************************************************
 * HELPER FUNCTIONS - common to all backends;
 * have those backend agnostic to avoid modifications in header files
 * ***************************************************************************/

static bool g_bHaveTraces = false;
static FILE* g_traceFileHdl = NULL;
static char g_tracefile[256] = "" ;

void awectrl_set_traces(bool haveTraces)
{
    g_bHaveTraces = haveTraces;
    AWE_CTRL_LOGD("Set awe_CTRL flag for trace dump to %d", g_bHaveTraces);
}
bool awectrl_get_traces()
{
    return g_bHaveTraces;
}

void awectrl_set_trace_file(FILE *fp)
{
    g_traceFileHdl = fp;
}
FILE* awectrl_get_trace_file()
{
    return g_traceFileHdl;
}

struct dumpctx
{
    int chn;
    char const *prefix;
};

static void awectrl_buf_dump(char *line, int index, void *ctx)
{
    struct dumpctx *d = (struct dumpctx*) ctx;
    AWEMGR_LOG("[chn:%d] %s: %4d : %s", d->chn, d->prefix, index, line);
}

/* ****************************************************************************
 * LOCAL FUNCTIONS
 * ***************************************************************************/

static void store_cmd_to_awb(void* data)
{
    unsigned int *tuneMsg = (unsigned int *)data;
    int nr_words_with_crc = (tuneMsg[0] >> 16) & 0xffff;
    int nr_words_to_write = nr_words_with_crc - 1;
    fwrite(data, nr_words_to_write, sizeof(unsigned int), g_traceFileHdl);
}

#define FAIL_ON_PTR(x) if (!x) { AWE_CTRL_LOGE("Invalid argument: %s == NULL!", #x); return AWECTRL_RC_FAIL_PARAM; }
#define FAIL_ON_CHN(ch) if(ch < 0 || ch >= MAX_AWECTRL_COMM_CHANNELS) { \
        AWE_CTRL_LOGE("Invalid argument: channel incorrect: must be 0 < channel < %d!", MAX_AWECTRL_COMM_CHANNELS); \
        return AWECTRL_RC_FAIL_PARAM; }


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int awectrl_register_configs(awe_config* cfg_p)
{
    FAIL_ON_PTR(cfg_p);

    int rc = AWECFG_RC_OK;
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    rc = aweconfig_add(cfg_p, CFG_CTRL_SOCKET_IP, "IP Address for socket based tuning/control", DEFAULT_CTRL_SOCKET_IP);
    if(rc != AWECFG_RC_OK)
        return AWECTRL_RC_FAIL_RESOURCES;

    rc = aweconfig_add(cfg_p, CFG_CTRL_SOCKET_PORT, "Port for socket based tuning/control", DEFAULT_CTRL_SOCKET_PORT);
    if(rc != AWECFG_RC_OK)
        return AWECTRL_RC_FAIL_RESOURCES;

    rc = aweconfig_add(cfg_p, CFG_CTRL_SOCKET_TIMEOUT, "Number of milliseconds to wait for response", DEFAULT_CTRL_SOCKET_TIMEOUT);
    if(rc != AWECFG_RC_OK)
        return AWECTRL_RC_FAIL_RESOURCES;
#endif
    rc = aweconfig_add(cfg_p, CFG_CTRL_BUFFER_SIZE, "Size of Tuning packet buffer", DEFAULT_TUNEMSG_SIZE_IN_WORDS);
    if(rc != AWECFG_RC_OK)
        return AWECTRL_RC_FAIL_RESOURCES;
    return AWECTRL_RC_OK;
}

int awectrl_init(awe_config* cfg_p, awectrl_cb_t cb, struct awectrl_data **ctrl_pp)
{
    int rc = AWECTRL_RC_OK;

    FAIL_ON_PTR(cfg_p);
    FAIL_ON_PTR(ctrl_pp);

    struct awectrl_data* ctrl_p = (struct awectrl_data *) calloc(1, sizeof(struct awectrl_data));
    if(ctrl_p == NULL)
    {
        AWE_CTRL_LOGE("Memory allocation problem!");
        return AWECTRL_RC_FAIL_RESOURCES;
    }

    uint32_t tuningBufferSize = 264;
    aweconfig_get_as_uint(cfg_p, CFG_CTRL_BUFFER_SIZE, &tuningBufferSize);

    AWE_CTRL_LOGI("Tuning Buffer size = %u words", tuningBufferSize);
    for (int i=0; i<MAX_AWECTRL_COMM_CHANNELS; i++)
    {
        int c = awecmd_init(&(ctrl_p->channel_data[i]), tuningBufferSize, false, tuningBufferSize);
        if (c != AWECMD_RC_OK)
        {
            AWE_CTRL_LOGE("Memory allocation problem for command buffer! idx=%d", i);
            return AWECTRL_RC_FAIL_RESOURCES;
        }
    }

    ctrl_p->config = cfg_p;

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    const char* host = aweconfig_get(cfg_p, CFG_CTRL_SOCKET_IP, NULL);
    const char* port = aweconfig_get(cfg_p, CFG_CTRL_SOCKET_PORT, NULL);

    if((host == NULL) || (port == NULL))
    {
        AWE_CTRL_LOGE("Unable to get socket host and port information");
        return AWECTRL_RC_FAIL_RESOURCES;
    }

    ctrl_p->timeoutMs = -1;
    aweconfig_get_as_int(cfg_p, CFG_CTRL_SOCKET_TIMEOUT, &ctrl_p->timeoutMs);

    AWE_CTRL_LOGI("Opening awe_CTRL connection to %s(%s)", host, port);
    ctrl_p->s_hdl = si_open_connection(host , port, 1);
    if (ctrl_p->s_hdl <= 0)
    {
        AWE_CTRL_LOGW("Communication to AWECore not available");
    }
    else
    {
        if(ctrl_p->timeoutMs > 0)
        {
            int rc = si_set_timeout(ctrl_p->s_hdl, ctrl_p->timeoutMs);
            if (rc != 0) {
                AWE_CTRL_LOGE("Could not set timeout to socket handle");
                return AWECTRL_RC_FAIL_COMM;
            }
            AWE_CTRL_LOGD("Timeout on socket handle: %d ms", ctrl_p->timeoutMs);
        }
    }
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM

    // todo: set ctrl_p->event_buffer to shmem location

    int client_fd = -1;
    unsigned int app_id = 0;
    int i, ret = AWECTRL_RC_OK;

    ret = awe_alsa_init();
    if (ret)
    {
        AWE_CTRL_LOGE("Failed to init awe_alsa");
        return -1;
    }

    for (i = 0; i < MAX_SUPPORT_CHANNEL_NUM; i++)
    {
        ret = awe_packet_register_client(&client_fd, &app_id);
        if (ret == PKT_SUCCESS)
        {
            ctrl_p->com_channel_hash_table[i].fd = client_fd;
            ctrl_p->com_channel_hash_table[i].app_id = app_id;
            ctrl_p->com_channel_hash_table[i].used = CH_STS_USED;

            AWE_CTRL_LOGI("Register client successfully, channel %d, app_id %x", i, app_id);
        }
        else
        {
            AWE_CTRL_LOGE("Register client failure");
        }
    }
#endif

    *ctrl_pp = ctrl_p;
    return rc;
}

int awectrl_exit(struct awectrl_data **ctrl_pp)
{
    FAIL_ON_PTR(ctrl_pp);

    struct awectrl_data *ctrl_p = *ctrl_pp;

    if(!ctrl_p)
    {
        AWE_CTRL_LOGD("*ctrl_pp already NULL. No memory to be released.");
        return AWECTRL_RC_OK;
    }

    for (int i=0; i<MAX_AWECTRL_COMM_CHANNELS; i++)
    {
        awecmd_exit(&(ctrl_p->channel_data[i]));
    }

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    if(ctrl_p->s_hdl > 0)
    {
        si_close_connection(ctrl_p->s_hdl);
    }
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    int i;
    for (i = 0; i < MAX_SUPPORT_CHANNEL_NUM; i++)
    {
        if (ctrl_p->com_channel_hash_table[i].used == CH_STS_USED)
        {
            awe_packet_deregister_client(ctrl_p->com_channel_hash_table[i].fd);

            /* clear each chanel data */
            ctrl_p->com_channel_hash_table[i].fd = -1;
            ctrl_p->com_channel_hash_table[i].app_id = 0;
            ctrl_p->com_channel_hash_table[i].used = CH_STS_UNUSED;

            AWE_CTRL_LOGI("deinit channel %d ", i);

            usleep(1000);
        }
    }
#endif

    free(ctrl_p);
    *ctrl_pp = NULL;
    return AWECTRL_RC_OK;
}

int awectrl_transact(struct awectrl_data *ctrl_p, int channel)
{
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_CHN(channel);

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    if(ctrl_p->s_hdl <= 0)
    {
        // try to establish the socket connection
        const char* host = aweconfig_get(ctrl_p->config, CFG_CTRL_SOCKET_IP, NULL);
        const char* port = aweconfig_get(ctrl_p->config, CFG_CTRL_SOCKET_PORT, NULL);
        ctrl_p->s_hdl = si_open_connection(host , port, 1);
        if(ctrl_p->s_hdl <= 0)
        {
            return AWECTRL_RC_FAIL_COMM;
        }
        else
        {
            if(ctrl_p->timeoutMs > 0)
            {
                int rc = si_set_timeout(ctrl_p->s_hdl, ctrl_p->timeoutMs);
                if (rc != 0) {
                    AWE_CTRL_LOGE("Could not set timeout to socket handle");
                    return AWECTRL_RC_FAIL_COMM;
                }
                AWE_CTRL_LOGD("Timeout on socket handle: %d ms", ctrl_p->timeoutMs);
            }
        }
    }
#endif

    struct awecmd_st *buf_p = &(ctrl_p->channel_data[channel]);

    return awectrl_transact_explicit(ctrl_p, buf_p->buffer_p, buf_p->words_written,
        buf_p->response_buffer_p, buf_p->response_buffer_size, channel);
}

int awectrl_transact_explicit(struct awectrl_data *ctrl_p, void* data, int data_sz, void* response, int response_sz, int chn)
{
    unsigned int nr_bytes_received = 0;
    int ret = 0;
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_PTR(data);
    FAIL_ON_PTR(response);

    // ************************************************************************
    // write data into channel buffer

// TODO: MUTEX PROTECTION HERE NOW !!!

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM


    if (ctrl_p->com_channel_hash_table[chn].fd <= 0
        || ctrl_p->com_channel_hash_table[chn].used == CH_STS_UNUSED)
    {
        AWE_CTRL_LOGI("channel %d invalid, reconnect to server", chn);
        ret = awectrl_register_channel(ctrl_p, chn);
        if (ret != AWECTRL_RC_OK)
        {
            return AWECTRL_RC_FAIL_COMM;
        }
    }

    ret = awe_packet_client_write(ctrl_p->com_channel_hash_table[chn].fd, data, data_sz*sizeof(unsigned int));
    if (ret != PKT_SUCCESS)
    {
        AWE_CTRL_LOGE("failed to send data on channel %d, ret %d", chn, ret);
        awectr_release_channel(ctrl_p, chn);
        return AWECTRL_RC_FAIL_COMM;
    }
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    if(ctrl_p->s_hdl <= 0)
    {
        AWE_CTRL_LOGE("No communication to AWE HostTask! Returning AWECTRL_RC_FAIL_COMM");
        return AWECTRL_RC_FAIL_COMM;
    }

    // transmit to AWE Server
    ret = si_write(ctrl_p->s_hdl, data, data_sz * sizeof(unsigned int));
    if (ret < 0) // Note: For write 0 is not an error
    {
        ctrl_p->s_hdl = 0;
        AWE_CTRL_LOGE("Could not send to AWE HostTask! rc = %d", ret);
        AWE_CTRL_LOGE("Tuning Socket Error, reseting connection");
        return AWECTRL_RC_FAIL_COMM;
    }
#endif

    if (g_bHaveTraces)
    {
        struct dumpctx c = { .chn = chn, .prefix = "TX"};
        awemgr_log_buffer(data, data_sz * sizeof(unsigned int), AWEMGR_LOG_VARTYPE_INT, awectrl_buf_dump, &c);
    }

    if (g_traceFileHdl != NULL)
    {
        store_cmd_to_awb(data);
    }

    // ************************************************************************
    // wait for response from channel buffer

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    ret = awe_packet_client_read(ctrl_p->com_channel_hash_table[chn].fd, response, response_sz*sizeof(unsigned int), &nr_bytes_received);
    if (ret != PKT_SUCCESS)
    {
        AWE_CTRL_LOGE("failed to receive data from packet on channel %d, ret %d\n", chn, ret);
        awectr_release_channel(ctrl_p, chn);
        return AWECTRL_RC_FAIL_COMM;
    }
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    // read response
    ret = si_readbuf(ctrl_p->s_hdl, response, response_sz * sizeof(unsigned int), response_sz * sizeof(unsigned int));
    if(ret <= 0) // For read 0 is also error
    {
        int err_number;
        bool is_timed_out = si_timedout(&err_number);
        if (is_timed_out) {
            AWE_CTRL_LOGD("Timeout on socket handle. No data received.");
            return AWECTRL_RC_TIMEOUT;
        }
        ctrl_p->s_hdl = 0;
        AWE_CTRL_LOGE("Tuning Socket Error, reseting connection");
        return AWECTRL_RC_FAIL_COMM;
    }
    nr_bytes_received = ret; // set number of bytes for debugging method
#endif

// TODO: END MUTEX PROTECTION HERE NOW !!!

    if (g_bHaveTraces)
    {
        struct dumpctx c = { .chn = chn, .prefix = "RX"};
        awemgr_log_buffer(response, nr_bytes_received, AWEMGR_LOG_VARTYPE_INT, awectrl_buf_dump, &c);
    }
    return AWECTRL_RC_OK;
}

int awectrl_notify(struct awectrl_data *ctrl_p, int channel)
{
    FAIL_ON_PTR(ctrl_p);

    // for AWEMGR_AWECORE_CONNECTION_SOCKET: nothing to do!

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    if (ctrl_p->com_channel_hash_table[channel].used == CH_STS_USED)
    {
        awe_packet_trigger_ipcc(ctrl_p->com_channel_hash_table[channel].fd);
    }
#endif

    return AWECTRL_RC_OK;
}

int awectrl_get_cmdbuf(struct awectrl_data *ctrl_p, int tunnel_address, struct awecmd_st **cmdbuf_pp)
{
    FAIL_ON_PTR(ctrl_p);
    FAIL_ON_PTR(cmdbuf_pp);

    int channel_idx = (tunnel_address != 0) ? 1 : 0;
    *cmdbuf_pp = &(ctrl_p->channel_data[channel_idx]);

    if (tunnel_address) {
        awecmd_reset(*cmdbuf_pp, tunnel_address);
    }

    return AWECTRL_RC_OK;
}

int awectrl_load_design_start(struct awectrl_data *ctrl_p, int channel)
{
#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    int ret = awe_alsa_load_awb_start(ctrl_p->com_channel_hash_table[channel].fd);
    return ret ? AWECTRL_RC_FAIL_COMM:AWECTRL_RC_OK;
#else
    return AWECTRL_RC_OK;
#endif
}

int awectrl_load_design_end(struct awectrl_data *ctrl_p, int channel)
{
#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    int ret = awe_alsa_load_awb_end(ctrl_p->com_channel_hash_table[channel].fd);
    return ret ? AWECTRL_RC_FAIL_COMM:AWECTRL_RC_OK;
#else
    return AWECTRL_RC_OK;
#endif
}

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
static void awectr_release_channel(struct awectrl_data *ctrl_p, int channel)
{
    awe_packet_deregister_client(ctrl_p->com_channel_hash_table[channel].fd);
    ctrl_p->com_channel_hash_table[channel].fd = -1;
    ctrl_p->com_channel_hash_table[channel].app_id = 0;
    ctrl_p->com_channel_hash_table[channel].used = CH_STS_USED;
}

static int awectrl_register_channel(struct awectrl_data *ctrl_p, int channel)
{
    int ret = PKT_SUCCESS;
    int client_fd = -1;
    unsigned int app_id = 0;

    FAIL_ON_PTR(ctrl_p);

    /* current socket fd is invalid and register new socket */
    AWE_CTRL_LOGI("reset channel %d", channel);

    ret = awe_packet_register_client(&client_fd, &app_id);
    if (ret == PKT_SUCCESS)
    {
        ctrl_p->com_channel_hash_table[channel].fd = client_fd;
        ctrl_p->com_channel_hash_table[channel].app_id = app_id;
        ctrl_p->com_channel_hash_table[channel].used = CH_STS_USED;

        AWE_CTRL_LOGI("retry to reregister client successfully, channel %d, app_id %x", channel, app_id);
    }
    else
    {
        AWE_CTRL_LOGI("failed to reregister client, channel %d", channel);
        return AWECTRL_RC_FAIL_COMM;
    }

    return AWECTRL_RC_OK;
}
#endif


