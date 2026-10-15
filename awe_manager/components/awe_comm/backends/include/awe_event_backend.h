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


/*
This is the header file of the awe_COMM component/library.
 */

#ifndef AWE_EVENT_BACKEND_H
#define AWE_EVENT_BACKEND_H

#include <stddef.h>
#include <stdint.h>
#include "awe_config.h"
#include "awe_cmd.h"  // for awecmd_st and aweevent_header
#include "awe_event_trace.h"


#define AWE_EVT_RC_OK               0
#define AWE_EVT_RC_COMM_ERR         -1
#define AWE_EVT_RC_FAIL_RESOURCE    -2
#define AWE_EVT_RC_INVALID_ARG      -3
#define AWE_EVT_RC_TIMEOUT          -4
#define AWE_EVT_RC_PROTOCOL_ERR     -5   /* malformed event data (magic word, truncated header or payload) */


typedef struct awe_evt_backend {
    int (*init)(struct awe_evt_backend* bkend);
    int (*exit)(struct awe_evt_backend* bkend);
    int (*read_event)(struct awe_evt_backend* bkend, uint32_t timeoutMs);
    void* platform_data;
    aweevt_listener evt_notify;
    void* userdata;

    // tracing information; allows to dump the transmitted or received data to log or file
    struct awe_evt_trace  *trace_cfg_p;

} awe_evt_backend;


#endif // AWE_EVENT_BACKEND_H