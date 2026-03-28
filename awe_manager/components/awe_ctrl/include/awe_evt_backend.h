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


/*
This is the header file of the awe_CTRL component/library.
 */

#ifndef AWE_EVT_BACKEND_H
#define AWE_EVT_BACKEND_H
#include <stddef.h>
#include <stdint.h>
#include "awe_config.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define AWE_EVT_RC_OK               0
#define AWE_EVT_RC_COMM_ERR         -1
#define AWE_EVT_RC_FAIL_RESOURCE    -2
#define AWE_EVT_RC_INVALID_ARG      -3
#define AWE_EVT_RC_TIMEOUT          -4

#ifndef AWE_EVENT_HDR_H
#define AWE_EVENT_HDR_H
typedef struct aweevent_hdr {
    uint32_t instanceId;
    uint32_t objectId;          // event module objectId;
    uint32_t classId;           // event module classid, 0 or don't care for system events (eventCategory != 0), 
    uint32_t eventType;         // event type as defined in event module
    uint32_t eventCategory;     // 0 = user/event 1 = system
    uint32_t dataSize;          // size of payload data in Bytes
    uint64_t timeStamp;         // systick
} aweevent_hdr;
#endif

typedef int (*aweevt_listener)(const aweevent_hdr* hdr, const char* payload, const void* userdata);

typedef struct awe_evt_backend {
    int (*init)(struct awe_evt_backend* bkend);
    int (*exit)(struct awe_evt_backend* bkend);
    int (*read_event)(struct awe_evt_backend* bkend, uint32_t timeoutMs);
    void* platform_data;
    aweevt_listener evt_notify;
    void* userdata;
} awe_evt_backend;

awe_evt_backend* create_evt_backend(awe_config *cfg_p, aweevt_listener cb, void* userdata);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // AWE_EVT_BACKEND_H