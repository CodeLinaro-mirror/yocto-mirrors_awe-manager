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


#ifndef INCLUSION_GUARD_AWE_EVT_TRACE_H
#define INCLUSION_GUARD_AWE_EVT_TRACE_H

#include <stdbool.h>
#include <stdio.h>

#include "awe_config.h"


#define CFG_COMM_EVT_TRACE_STATE              "mgr.event.trace.state"
#define CFG_COMM_EVT_TRACE_STATE_VAL_DEFAULT  "off"

#define CFG_COMM_EVT_TRACE_FILE_NONE "~"

#define CFG_COMM_EVT_TRACE_FILE               "mgr.event.trace.file"
#define CFG_COMM_EVT_TRACE_FILE_VAL_DEFAULT   CFG_COMM_EVT_TRACE_FILE_NONE


struct awe_evt_trace  // renamed to awe_evt to avoid collision with external integration code
{
    awe_config* cfg_p;

    FILE*       event_dump_fp;  // != NULL if file is open for dumping control data
    bool        do_trace; // whether to print trace output to console
};

int aweevent_trace_register_configs(awe_config* cfg_p);
int aweevent_trace_init(struct awe_evt_trace *trace_cfg_p, awe_config* cfg_p);
int aweevent_trace_exit(struct awe_evt_trace *trace_cfg_p);
int aweevent_trace_dump(struct awe_evt_trace *trace_cfg_p, int chn, const char *direction, FILE *fp, void* data, int data_sz_words);
int aweevent_trace_finalize(struct awe_evt_trace *trace_cfg_p);

#endif // INCLUSION_GUARD_AWE_EVT_TRACE_H
