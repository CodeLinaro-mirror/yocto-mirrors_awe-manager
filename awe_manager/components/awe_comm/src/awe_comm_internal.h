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

#ifndef INCLUSION_GUARD_AWE_COMM_INTERNAL_H
#define INCLUSION_GUARD_AWE_COMM_INTERNAL_H

#include "awe_comm_backend.h"
#include "awe_cmd.h"
#include "awe_config.h"
#include "awosal_thread.h"
#include "awosal_mutex.h"
#include "awe_comm_trace.h"

#define MAX_AWECOMM_CHANNELS  2


struct awecomm_data
{
    // pointer to the actual backend implementation
    awe_comm_backend* backend;

    // tracing information; allows to dump the transmitted or received data to log or file
    struct awecomm_trace  trace_cfg;

    // not used yet, but once async responses are available, this will be the event callback
    aweevt_listener rx_event_cb;
    void* rx_event_cb_ctx;

    // "channel" data; either dyn. allocated, or the buffer ptrs in awecmd_st are "set to shmem"
    struct awecmd_st channel_data[MAX_AWECOMM_CHANNELS];

    awe_config* config;

    awosal_mutex* channel_protection_mutex; //<** mutex to protect access to the channel data; this is used by all backends

    int timeout_ms; // timeout for transaction operations
};

#endif // INCLUSION_GUARD_AWE_COMM_INTERNAL_H
