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

#include "awe_event_backend.h"
#include "awosal_socket.h"
#include "awosal_string.h"
#include "awe_comm_logging.h"
#include "awe_event_socket_cfg.h"

#define EVENT_MAX_PAYLOAD_SIZE 1000U
#define AWE_EVENT_MAGIC_WORD   0xdeadaffe

#define FAIL_ON_PTR(x, msg) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!: %s", #x, msg); return AWE_EVT_RC_INVALID_ARG; }

typedef struct evtsocket_data
{
    int socketHdl;
    char* buffer;
    uint32_t size;
    aweevent_header hdr;
    int current_timeout;  // keeps the actual timeout value of the socket comm
    awe_config* config;
}evtsocket_data;


int socket_evt_init(awe_evt_backend* bkend)
{
    FAIL_ON_PTR(bkend, "backend state handle is required");
    evtsocket_data* pData = (evtsocket_data*)bkend->platform_data;
    FAIL_ON_PTR(pData, "platform data is required");

    const char* host = aweconfig_get(pData->config, CFG_EVENT_SOCKET_IP, NULL);
    const char* port = aweconfig_get(pData->config, CFG_EVENT_SOCKET_PORT, NULL);

    AWE_COMM_LOGI("Opening event connection to %s(%s)", host, port);

    if((host != NULL) && (port != NULL))
    {
        pData->socketHdl = si_open_connection (host, port, 1);
        if (pData->socketHdl <= 0)
        {
            AWE_COMM_LOGW("Socket backend for event communication not available");
        }
        else
        {
            AWE_COMM_LOGI("Socket backend for event communication up and running");
        }
    }
    return AWE_EVT_RC_OK;
}

int socket_evt_exit(awe_evt_backend* bkend)
{
    if(bkend == NULL)
    {
        AWE_COMM_LOGW("Socket backend for event communication already de-initialized");
        return AWE_EVT_RC_OK;
    }
    evtsocket_data* pData = (evtsocket_data*)bkend->platform_data;
    if(pData != NULL)
    {
        if(pData->socketHdl > 0)
        {
            (void)si_close_connection(pData->socketHdl);
        }
        if(pData->buffer != NULL)
        {
            free(pData->buffer);
        }
        free(pData);
    }
    free(bkend);
    AWE_COMM_LOGI("Socket backend for event communication de-initialized");
    return AWE_EVT_RC_OK;
}

int socket_evt_read(awe_evt_backend* bkend, uint32_t timeoutMs)
{
    int err_number;

    FAIL_ON_PTR(bkend, "backend state handle is required");
    evtsocket_data* pData = bkend->platform_data;
    FAIL_ON_PTR(pData, "platform data is required");
    if(pData->socketHdl <= 0)
    {
        // try to establish the socket connection
        const char* host = aweconfig_get(pData->config, "mgr.event.socket.ip", NULL);
        const char* port = aweconfig_get(pData->config, "mgr.event.socket.port", NULL);
        pData->socketHdl = si_open_connection(host , port, 1);
        if(pData->socketHdl <= 0)
        {
            AWE_COMM_LOGE("Could not read event, socket connection failed");
            return AWE_EVT_RC_COMM_ERR;
        }
    }

    // configure the socket with a timeout if required
    if (pData->current_timeout != timeoutMs && (timeoutMs != 0)) {
        int rc = si_set_timeout(pData->socketHdl, timeoutMs);
        if (rc != 0) {
            AWE_COMM_LOGE("Could not set timeout to socket handle");
            return AWE_EVT_RC_COMM_ERR;
        }
        AWE_COMM_LOGD("Timeout on socket handle: %d ms", timeoutMs);
    } else {
        int rc = si_set_timeout(pData->socketHdl, 0);
        if (rc) {
            AWE_COMM_LOGE("Could not set zero timeout to socket handle");
            return AWE_EVT_RC_COMM_ERR;
        }
    }

    // 1. read magicWord
    uint32_t magicWord;
    int nr_bytes_received = si_readbuf(pData->socketHdl, &magicWord, sizeof(magicWord), sizeof(magicWord));
    if (nr_bytes_received <= 0) {
        bool is_timed_out = si_timedout(&err_number);
        if (is_timed_out) {
            AWE_COMM_LOGD("Timeout on socket handle. No event received yet.");
            return AWE_EVT_RC_TIMEOUT;
        }
        AWE_COMM_LOGE("Event communication error: no eventHeaderInfo. Reseting connection");
        (void)si_close_connection(pData->socketHdl);
        pData->socketHdl = 0;
        return AWE_EVT_RC_COMM_ERR;
    }

    if (magicWord != AWE_EVENT_MAGIC_WORD)
    {
        AWE_COMM_LOGE("Event communication error: Incorrect magic word received from communication buffer: 0x%x (seen) != 0x%x (wanted). Returning AWECOMM_RC_FAIL_PARAM",
                        magicWord, AWE_EVENT_MAGIC_WORD);
        return AWE_EVT_RC_COMM_ERR;
    }

    // 2. read Event header
    nr_bytes_received = si_readbuf(pData->socketHdl, &pData->hdr, sizeof(aweevent_header), sizeof(aweevent_header));
    if (nr_bytes_received < sizeof(aweevent_header))
    {
        // si_timedout(&err_number); not needed actually; also a timeout error would be an error
        // as the magic cookie was already received
        AWE_COMM_LOGE("Event communication error: no eventHeader. Reseting connection");
        (void)si_close_connection(pData->socketHdl);
        pData->socketHdl = 0;
        return AWE_EVT_RC_COMM_ERR;
    }

    // 3. Resize Event payload Buffer(if required)
    if (pData->hdr.dataSize > pData->size)
    {
        AWE_COMM_LOGW("Payload size(0x%x Bytes) greater than local buffer size(0x%x Bytes). Resizing local buffer.", pData->hdr.dataSize, pData->size);
        uint32_t newSize = pData->hdr.dataSize;
        char* newBuff = (char*)realloc(pData->buffer, newSize);
        if (newBuff == NULL) {
            AWE_COMM_LOGE("Payload buffer could not be resized");
            return AWE_EVT_RC_FAIL_RESOURCE;
        } else {
            pData->buffer = newBuff;
            pData->size = newSize;
        }
    }

    // 4. read payload
    nr_bytes_received = si_readbuf(pData->socketHdl, pData->buffer, pData->hdr.dataSize, pData->hdr.dataSize);
    if (nr_bytes_received == 0)
    {
        // si_timedout(&err_number); not needed actually; also a timeout error would be an error
        // as the magic cookie was already received
        AWE_COMM_LOGE("Event communication error: payload read failed. Reseting connection");
        (void)si_close_connection(pData->socketHdl);
        pData->socketHdl = 0;
        return AWE_EVT_RC_COMM_ERR;
    }

    // log trace of event and of event data (if enabled)
    AWE_COMM_LOGI("Dispatching event: instId=%d, objId=%d, classId=%d, evType=%d, evCat=%d,size=%d",
        pData->hdr.instanceId, pData->hdr.objectId, pData->hdr.classId, pData->hdr.eventType, pData->hdr.eventCategory, pData->hdr.dataSize);

    aweevent_trace_dump(bkend->trace_cfg_p, 0, "EVT", NULL, pData->buffer, pData->hdr.dataSize / sizeof(unsigned int));

    // Notify the listener (Awe Manager)
    bkend->evt_notify(&pData->hdr, pData->buffer, bkend->userdata);

    return AWE_EVT_RC_OK;
}

awe_evt_backend* create_event_backend_socket(awe_config *cfg_p, aweevt_listener cb, void* userdata) {
    if (cfg_p == NULL) {
        AWE_COMM_LOGE("config object can not be NULL");
        return NULL;
    }

    if (cb == NULL) {
        AWE_COMM_LOGE("Event listener can not be NULL");
        return NULL;
    }
    awe_evt_backend* pBkend = (awe_evt_backend *) calloc(1, sizeof(awe_evt_backend));
    if (pBkend == NULL) {
        AWE_COMM_LOGE("Failed to allocate memory for Event Backend");
        return NULL;
    }

    evtsocket_data* pData = (evtsocket_data *) calloc(1, sizeof(evtsocket_data));
    if(pData == NULL)
    {
        AWE_COMM_LOGE("Failed to allocate memory for platform data!");
        socket_evt_exit(pBkend);
        return NULL;
    }
    pData->current_timeout = 0;  // start with blocking mode
    pData->size = EVENT_MAX_PAYLOAD_SIZE;
    pData->buffer = calloc(pData->size, sizeof(char));
    if(pData->buffer == NULL)
    {
        AWE_COMM_LOGE("Failed to allocate memory for event payload data!");
        socket_evt_exit(pBkend);
        return NULL;
    }
    pBkend->evt_notify = cb;
    pBkend->userdata = userdata;
    pBkend->platform_data = (void*)pData;
    pBkend->init = socket_evt_init;
    pBkend->exit = socket_evt_exit;
    pBkend->read_event = socket_evt_read;
    pData->config = cfg_p;
    return pBkend;
}
