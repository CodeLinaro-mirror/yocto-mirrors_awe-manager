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
#include "awosal_socket.h"
#include "awosal_string.h"
#include "awosal_time.h"
#include "awe_comm_logging.h"
#include "awe_config.h"
#include "awe_comm.h"
#include "awe_comm_socket_cfg.h"
#include "awe_comm_common_cfg.h"

#include <errno.h>


#define FAIL_ON_PTR(x, msg) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!: %s", #x, msg); return AWECOMM_RC_FAIL_PARAM; }

typedef struct commsocket_data
{
    int s_hdl;
    int timeoutMs;  /* poll interval for read operations */

    /* Reconnection state — tracks when the last attempt was made so that
     * reconnect tries are rate-limited to mgr.comm.socket.reconnect_interval_ms. */
    aweosal_clock_time last_reconnect_time;

    awe_config* config;

} commsocket_data;

/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/
static int set_timeout(commsocket_data* pData, int timeout_ms)
{
    // configure the socket with a timeout if required
    if (pData->timeoutMs != timeout_ms) {
        int rc = si_set_timeout(pData->s_hdl, timeout_ms);
        if (rc != 0) {
            AWE_COMM_LOGE("Could not set timeout to socket handle (%d ms)", timeout_ms);
            return AWECOMM_RC_FAIL_COMM;
        }
        pData->timeoutMs = timeout_ms;
        AWE_COMM_LOGD("Timeout on socket handle: %d ms", timeout_ms);
    }
    return AWECOMM_RC_OK;
}

/* Open (or reopen) the TCP socket to AWECore.
 * On success: pData->s_hdl > 0 and timeout is applied.
 * On failure: pData->s_hdl <= 0, pData->last_reconnect_time is updated. */
static void socket_connect(commsocket_data* pData)
{
    const char* host = aweconfig_get(pData->config, CFG_COMM_SOCKET_IP, NULL);
    const char* port = aweconfig_get(pData->config, CFG_COMM_SOCKET_PORT, NULL);

    pData->last_reconnect_time = aweosal_measure_start();

    if (!host || !port)
    {
        AWE_COMM_LOGE("Unable to get socket host/port from config");
        return;
    }

    pData->s_hdl = si_open_connection(host, port, 1);

    if (pData->s_hdl <= 0)
    {
        AWE_COMM_LOGD("Connection attempt to AWECore at %s:%s failed", host, port);
        return;
    }

    if (pData->timeoutMs > 0)
    {
        if (si_set_timeout(pData->s_hdl, pData->timeoutMs) != 0)
        {
            AWE_COMM_LOGE("Could not set timeout on socket");
            si_close_connection(pData->s_hdl);
            pData->s_hdl = 0;
            return;
        }
        AWE_COMM_LOGD("Timeout set on socket: %d ms", pData->timeoutMs);
    }

    AWE_COMM_LOGI("Connected to AWECore at %s:%s", host, port);
}

static int socket_init(awe_comm_backend* bkend)
{
    FAIL_ON_PTR(bkend, "backend state handle is required");
    commsocket_data* pData = (commsocket_data*)bkend->platform_data;
    FAIL_ON_PTR(pData, "platform data is required");

    if (aweconfig_get(pData->config, CFG_COMM_SOCKET_IP, NULL) == NULL ||
        aweconfig_get(pData->config, CFG_COMM_SOCKET_PORT, NULL) == NULL)
    {
        AWE_COMM_LOGE("Unable to get socket host and port information");
        return AWECOMM_RC_FAIL_RESOURCES;
    }

    pData->timeoutMs = -1;
    aweconfig_get_as_int(pData->config, CFG_COMM_TIMEOUT, &pData->timeoutMs);

    const char* host = aweconfig_get(pData->config, CFG_COMM_SOCKET_IP, NULL);
    const char* port = aweconfig_get(pData->config, CFG_COMM_SOCKET_PORT, NULL);
    AWE_COMM_LOGI("Opening awe_COMM connection to %s(%s)", host, port);

    socket_connect(pData);

    if (pData->s_hdl <= 0)
        AWE_COMM_LOGW("Communication to AWECore not available — will retry on first use");

    return AWECOMM_RC_OK;
}

static int socket_close(awe_comm_backend* bkend)
{
    if(bkend == NULL)
    {
        AWE_COMM_LOGW("Socket backend already de-initialized");
        return AWECOMM_RC_OK;
    }
    commsocket_data* pData = (commsocket_data*)bkend->platform_data;
    if(pData != NULL)
    {
        if(pData->s_hdl > 0)
        {
            (void)si_close_connection(pData->s_hdl);
            pData->s_hdl = 0;
        }
        free(pData);
    }
    free(bkend);
    AWE_COMM_LOGI("Socket backend de-initialized");
    return AWECOMM_RC_OK;
}

static int socket_write(awe_comm_backend* bkend, void* data, int data_sz_words, uint32_t timeoutMs)
{
    (void)timeoutMs;

    commsocket_data* pData = (commsocket_data*)bkend->platform_data;

    if (pData->s_hdl <= 0)
    {
        /* No active connection — attempt reconnect if the configured interval
         * has elapsed since the last attempt.  interval_ms < 0 disables it. */
        int32_t interval_ms = 1000;
        aweconfig_get_as_int(pData->config, CFG_COMM_SOCKET_RECONNECT_INTERVAL_MS, &interval_ms);

        if (interval_ms >= 0)
        {
            double elapsed_ms = aweosal_measure_elapsed(pData->last_reconnect_time) * 1000.0;
            if (elapsed_ms >= (double)interval_ms)
            {
                AWE_COMM_LOGI("AWECore unreachable — attempting reconnect");
                socket_connect(pData);
            }
        }

        if (pData->s_hdl <= 0)
        {
            AWE_COMM_LOGE("No communication to AWECore");
            return AWECOMM_RC_FAIL_COMM;
        }
    }

    // possibly update the timeout value for the socket if it has changed since last time
    int ret = set_timeout(pData, timeoutMs);
    if (ret != AWECOMM_RC_OK)
    {
        return ret;
    }

    ret = si_write(pData->s_hdl, data, data_sz_words * sizeof(unsigned int));
    if (ret != data_sz_words * sizeof(unsigned int))
    {
        AWE_COMM_LOGE("Send to AWECore failed (rc=%d) — connection lost", ret);
        si_close_connection(pData->s_hdl);
        pData->s_hdl = 0;
        pData->last_reconnect_time = aweosal_measure_start();
        return AWECOMM_RC_FAIL_COMM;
    }

    return AWECOMM_RC_OK;
}

static int socket_read(awe_comm_backend* bkend, void* target_buffer, int target_buffer_sz_words, int* nr_words_read, uint32_t timeoutMs)
{
    commsocket_data* pData = (commsocket_data*)bkend->platform_data;
    // note: we can omit checking if nr_words_read is NULL, it is only use in awe_comm.c and verified to point to a variable on stack

    // possibly update the timeout value for the socket if it has changed since last time
    int ret = set_timeout(pData, timeoutMs);
    if (ret != AWECOMM_RC_OK)
    {
        *nr_words_read = 0;
        return ret;
    }

    ret = si_readbuf(pData->s_hdl, target_buffer, target_buffer_sz_words * sizeof(unsigned int), target_buffer_sz_words * sizeof(unsigned int));
    // AWE_COMM_LOGW("ret=%d from si_readbuf", ret);

    if (ret <= 0)
    {
        *nr_words_read = 0;

        if (pData->s_hdl <= 0)
        {
            AWE_COMM_LOGW("Socket handle closed during read");
            return AWECOMM_RC_FAIL_COMM;
        }

        int err_number;
        bool is_timed_out = si_timedout(&err_number);
        if (is_timed_out)
        {
            AWE_COMM_LOGD("No data after timeout on receiving socket. Trying again...");
            return AWECOMM_RC_TIMEOUT;
        }

        if (err_number == EINTR)
            return AWECOMM_RC_RETRY_COMM; /* signal interrupted the read, try again */

        /* Connection lost (ret=0 is EOF, negative is a socket error).
         * Close and clear the handle so the next socket_write() triggers
         * a reconnect attempt. */
        AWE_COMM_LOGW("AWECore connection lost (err=%d, ret=%d)", err_number, ret);
        si_close_connection(pData->s_hdl);
        pData->s_hdl = 0;
        pData->last_reconnect_time = aweosal_measure_start();
        return AWECOMM_RC_FAIL_COMM;
    }

    // convert to number of words read for the caller;
    // we assume that the buffer is always a multiple of word size as AWECore is on the other side
    *nr_words_read = ret / sizeof(unsigned int);
    if (ret % sizeof(unsigned int) != 0)
    {
        AWE_COMM_LOGE("Received data size %d is not a multiple of word size! Invalid response from AWECore", ret);
        return AWECOMM_RC_FAIL_PARAM;   // a malformed response, not a failed connection
    }

    return AWECOMM_RC_OK;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

awe_comm_backend* create_comm_backend_socket(awe_config *cfg_p, aweevt_listener cb, void* userdata)
{
    if (cfg_p == NULL) {
        AWE_COMM_LOGE("config object can not be NULL");
        return NULL;
    }

    awe_comm_backend* pBkend = (awe_comm_backend *) calloc(1, sizeof(awe_comm_backend));
    if (pBkend == NULL) {
        AWE_COMM_LOGE("Failed to allocate memory for Event Backend");
        return NULL;
    }

    commsocket_data* pData = (commsocket_data *) calloc(1, sizeof(commsocket_data));
    if(pData == NULL)
    {
        AWE_COMM_LOGE("Failed to allocate memory for platform data!");
        socket_close(pBkend);
        return NULL;
    }

    pBkend->platform_data = (void*)pData;
    pBkend->init = socket_init;
    pBkend->exit = socket_close;
    pBkend->write = socket_write;
    pBkend->read = socket_read;
    pData->config = cfg_p;
    return pBkend;
}
