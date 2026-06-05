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


#ifndef INCLUSION_GUARD_AWE_COMM_TRACE_H
#define INCLUSION_GUARD_AWE_COMM_TRACE_H

#include <stdbool.h>
#include <stdio.h>

#include "awe_config.h"
#include "awosal_mutex.h"


#define CFG_COMM_TRACE_STATE              "mgr.comm.trace.state"
#define CFG_COMM_TRACE_STATE_VAL_DEFAULT  "off"

#define CFG_COMM_TRACE_FILE_NONE "~"

#define CFG_COMM_TRACE_FILE               "mgr.comm.trace.file"
#define CFG_COMM_TRACE_FILE_VAL_DEFAULT   CFG_COMM_TRACE_FILE_NONE


/**
 * Callback type for the shell comm-trace tap.
 *
 * Called once for TX (before the packet is sent) and once for RX (after the
 * response is received).  Both calls happen synchronously inside the same
 * manager API call, so the shell can print TX/RX blocks in order.
 *
 * @param direction  "TX" or "RX"
 * @param data       Pointer to the raw word buffer
 * @param data_sz_words  Number of 32-bit words in @p data
 * @param ctx        Opaque context supplied to awecomm_set_observer()
 */
typedef void (*awecomm_buf_observer_cb)(const char *direction,
                                   const void *data,
                                   int         data_sz_words,
                                   void       *ctx);

struct awecomm_trace
{
    awe_config* cfg_p;

    FILE*       comm_tx_fp;  // != NULL if file is open for dumping control data
    bool        do_trace; // whether to print trace output to console

    /* Optional comm tap - set via awecomm_trace_set_observer(), cleared by passing NULL. */
    awecomm_buf_observer_cb  tap_cb;
    void*                    tap_ctx;
    awosal_mutex*            observer_cb_mutex; //<** mutex to protect setting to the callback/data

};

int awecomm_trace_register_configs(awe_config* cfg_p);
int awecomm_trace_init(struct awecomm_trace *trace_cfg_p, awe_config* cfg_p);
int awecomm_trace_exit(struct awecomm_trace *trace_cfg_p);
int awecomm_trace_dump(struct awecomm_trace *trace_cfg_p, int chn, const char *direction, FILE *fp, void* data, int data_sz_words);
int awecomm_trace_finalize(struct awecomm_trace *trace_cfg_p);

/**
 * Install (or remove) a raw-buffer tap on the comm trace.
 *
 * When set, @p cb is called for every TX and RX packet in addition to any
 * normal log / file output.  Pass @p cb = NULL to remove the tap.
 */
void awecomm_trace_set_observer(struct awecomm_trace *trace_cfg_p, awecomm_buf_observer_cb cb, void *ctx);

#endif // INCLUSION_GUARD_AWE_COMM_TRACE_H
