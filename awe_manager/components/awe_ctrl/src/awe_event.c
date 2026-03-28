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

#include "awe_event.h"
#include "awe_ctrl.h"
#include <stdlib.h>
#include "awosal_string.h"
#include "awe_ctrl_logging.h"
#include "awe_config.h"

/* ****************************************************************************
 * TYPE DEFINITIONS (NON PUBLIC) - POSSIBLY BACKEND SPECIFIC
 * ***************************************************************************/

struct aweevent_data
{
    awe_evt_backend* backend;
};

#define FAIL_ON_PTR(x) if (!x) { AWE_CTRL_LOGE("Invalid argument: %s == NULL!", #x); return AWECTRL_RC_FAIL_PARAM; }

/* ****************************************************************************
 * PRIVATE
 * ***************************************************************************/
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
#define CFG_EVT_SOCKET_IP                   "mgr.event.socket.ip"
#define CFG_EVT_SOCKET_PORT                 "mgr.event.socket.port"

#define DEFAULT_EVT_SOCKET_IP               "127.0.0.1"
#define DEFAULT_EVT_SOCKET_PORT             "15010"
#endif


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int aweevent_register_configs(awe_config* cfg_p)
{
    FAIL_ON_PTR(cfg_p);
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    int rc = aweconfig_add(cfg_p, CFG_EVT_SOCKET_IP, "IP Address for socket based event transmitter", DEFAULT_EVT_SOCKET_IP);
    if(rc != AWECFG_RC_OK)
        return AWECTRL_RC_FAIL_RESOURCES;

    rc = aweconfig_add(cfg_p, CFG_EVT_SOCKET_PORT, "Port for socket based event transmitter", DEFAULT_EVT_SOCKET_PORT);
    if(rc != AWECFG_RC_OK)
        return AWECTRL_RC_FAIL_RESOURCES;
#endif
    return AWE_EVT_RC_OK;
}

int aweevent_init(awe_config *cfg_p, aweevent_data **evnt_pp, aweevt_listener listener, void* userdata)
{
    int rc = AWECTRL_RC_OK;
    FAIL_ON_PTR(cfg_p);
    FAIL_ON_PTR(evnt_pp);
    aweevent_data* evt_p = (aweevent_data *) calloc(1, sizeof(aweevent_data));
    if(!evt_p) 
    {
        AWE_CTRL_LOGE("Memory allocation problem!");
        return AWECTRL_RC_FAIL_RESOURCES;
    }
    awe_evt_backend* pBackend = create_evt_backend(cfg_p, listener, userdata);
    if(!pBackend) 
    {
        AWE_CTRL_LOGE("Could not create hal backend!");
        free(evt_p);
        return AWECTRL_RC_FAIL_RESOURCES;
    }
    evt_p->backend = pBackend;
    
    rc = pBackend->init(pBackend);
    if(rc < 0)
    {
        pBackend->exit(pBackend);
        free(evt_p);
        AWE_CTRL_LOGE("Event Communication not available");
        return AWECTRL_RC_FAIL_COMM;
    }
    *evnt_pp = evt_p;
    
    AWE_CTRL_LOGI("Event Communication backend available");
    return rc;
}

int aweevent_exit(aweevent_data  **event_pp)
{
    FAIL_ON_PTR(event_pp);

    aweevent_data  *evnt_p = *event_pp;

    if(!evnt_p)
    {
        AWE_CTRL_LOGD("*event_pp already NULL. No memory to be released.");
        return AWECTRL_RC_OK;
    }
    awe_evt_backend* pBackend = evnt_p->backend;
    pBackend->exit(pBackend);
    free(evnt_p);
    *event_pp = NULL;
    AWE_CTRL_LOGI("Event Communication backend turned down");
    return AWECTRL_RC_OK;
}

int aweevent_read(aweevent_data *evnt_p, uint32_t timeoutMs)
{
    FAIL_ON_PTR(evnt_p);
    awe_evt_backend* pBackend = evnt_p->backend;
    int rc = pBackend->read_event(pBackend, timeoutMs);

    int ev_rc = AWECTRL_RC_OK;
    switch(rc) {
        case AWE_EVT_RC_COMM_ERR: {
            AWE_CTRL_LOGE("Failed to read Event");
            ev_rc = AWECTRL_RC_FAIL_COMM;
            break;
        }
        case AWE_EVT_RC_INVALID_ARG: 
            ev_rc = AWECTRL_RC_FAIL_COMM;
            break;
        case AWE_EVT_RC_TIMEOUT:
            ev_rc = AWECTRL_RC_TIMEOUT;
            break;
        case AWE_EVT_RC_OK:  // no error "anymore"
        default:
            break;
    }
    return ev_rc;
}