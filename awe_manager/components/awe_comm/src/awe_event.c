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

#include "awe_event.h"
#include "awe_comm.h"
#include "awosal_string.h"
#include "awe_comm_logging.h"
#include "awe_config.h"
#include "awe_event_backend_selector.h"
#include "awe_event_trace.h"


#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    #include "awe_event_socket_cfg.h"
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    #include "awe_event_aweq_cfg.h"
#endif


/* ****************************************************************************
 * TYPE DEFINITIONS (NON PUBLIC) - POSSIBLY BACKEND SPECIFIC
 * ***************************************************************************/

struct aweevent_data
{
    // pointer to the actual backend implementation
    awe_evt_backend* backend;

    // tracing information; allows to dump the transmitted or received data to log or file
    struct awe_evt_trace  trace_cfg;
};

#define FAIL_ON_PTR(x) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!", #x); return AWECOMM_RC_FAIL_PARAM; }

/* ****************************************************************************
 * PRIVATE
 * ***************************************************************************/



/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int aweevent_register_configs(awe_config* cfg_p)
{
    FAIL_ON_PTR(cfg_p);

    int rc = AWECFG_RC_OK;
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    rc = register_event_backend_socket_configs(cfg_p);
#endif
#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    rc = register_event_backend_aweq_configs(cfg_p);
#endif

    // !!! not needed here, as we have to have a comm backend already initialized to have the trace configs available;
    // and trace configs are common for comm and evt backend, so they are registered via awecomm_register_configs already
    // when comm backend is initialized
    // if (rc == AWECFG_RC_OK)
    // {
    //     rc = awecomm_trace_register_configs(cfg_p);
    // }

    return (rc != AWECFG_RC_OK) ? AWE_EVT_RC_INVALID_ARG : AWE_EVT_RC_OK;
}

int aweevent_init(awe_config *cfg_p, aweevent_data **evnt_pp, aweevt_listener listener, void* userdata)
{
    int rc = AWECOMM_RC_OK;
    FAIL_ON_PTR(cfg_p);
    FAIL_ON_PTR(evnt_pp);
    aweevent_data* evt_p = (aweevent_data *) calloc(1, sizeof(aweevent_data));
    if(!evt_p)
    {
        AWE_COMM_LOGE("Memory allocation problem!");
        return AWECOMM_RC_FAIL_RESOURCES;
    }
    awe_evt_backend* pBackend = create_event_backend(cfg_p, listener, userdata);
    if(!pBackend)
    {
        AWE_COMM_LOGE("Could not create hal backend!");
        free(evt_p);
        return AWECOMM_RC_FAIL_RESOURCES;
    }
    evt_p->backend = pBackend;

    aweevent_trace_init(&evt_p->trace_cfg, cfg_p);
    pBackend->trace_cfg_p = &evt_p->trace_cfg;

    rc = pBackend->init(pBackend);
    if(rc < 0)
    {
        pBackend->exit(pBackend);
        free(evt_p);
        AWE_COMM_LOGE("Event Communication not available");
        return AWECOMM_RC_FAIL_COMM;
    }
    *evnt_pp = evt_p;

    AWE_COMM_LOGI("Event Communication backend available");
    return rc;
}

int aweevent_exit(aweevent_data  **event_pp)
{
    FAIL_ON_PTR(event_pp);

    aweevent_data  *evnt_p = *event_pp;

    if(!evnt_p)
    {
        AWE_COMM_LOGD("*event_pp already NULL. No memory to be released.");
        return AWECOMM_RC_OK;
    }
    awe_evt_backend* pBackend = evnt_p->backend;
    pBackend->exit(pBackend);

    aweevent_trace_exit(&evnt_p->trace_cfg);

    free(evnt_p);
    *event_pp = NULL;
    AWE_COMM_LOGI("Event Communication backend turned down");
    return AWECOMM_RC_OK;
}

int aweevent_read(aweevent_data *evnt_p, uint32_t timeoutMs)
{
    FAIL_ON_PTR(evnt_p);
    awe_evt_backend* pBackend = evnt_p->backend;
    int rc = pBackend->read_event(pBackend, timeoutMs);

    int ev_rc = AWECOMM_RC_OK;
    switch(rc) {
        case AWE_EVT_RC_COMM_ERR: {
            AWE_COMM_LOGE("Failed to read Event");
            ev_rc = AWECOMM_RC_FAIL_COMM;
            break;
        }
        case AWE_EVT_RC_INVALID_ARG:   // an invalid argument or malformed event data
        case AWE_EVT_RC_PROTOCOL_ERR:  // is not a failed communication
            ev_rc = AWECOMM_RC_FAIL_PARAM;
            break;
        case AWE_EVT_RC_FAIL_RESOURCE:
            ev_rc = AWECOMM_RC_FAIL_RESOURCES;
            break;
        case AWE_EVT_RC_TIMEOUT:
            ev_rc = AWECOMM_RC_TIMEOUT;
            break;
        case AWE_EVT_RC_OK:  // no error "anymore"
        default:
            break;
    }
    return ev_rc;
}