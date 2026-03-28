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
It provides very simple methods to abstract the communication between 2 participants.
 */

#ifndef INCLUSION_GUARD_AWE_CTRL_H
#define INCLUSION_GUARD_AWE_CTRL_H

#include <stdbool.h>
#include <stdio.h>

#include "awe_config.h"
#include "awe_cmd.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Helper function to enable/disable tracing of data packages on the
 * awe_CTRL communication interface
 */
void awectrl_set_traces(bool haveTraces);
bool awectrl_get_traces();
void awectrl_set_trace_file(FILE *fp);
FILE* awectrl_get_trace_file();

// todo: define return/error values, enum?

#define MAX_AWECTRL_COMM_CHANNELS 5

#define AWECTRL_RC_OK              0
#define AWECTRL_RC_FAIL_COMM      -1
#define AWECTRL_RC_FAIL_RESOURCES -2
#define AWECTRL_RC_FAIL_PARAM     -3
#define AWECTRL_RC_TIMEOUT        -4

struct awectrl_data;

typedef int (*awectrl_cb_t)(void *todo_define_me);

/**
 * Fills the awe_config structure with the required configurations for this component
 *
 * @param cfg_p[in]: Pointer to the awe_config object
 * @return an error code
 */
int awectrl_register_configs(awe_config* cfg_p);

/**
 * Initializes the underlying communication channel (backend), for example it sets up everything
 * inside the carved shared memory, required for communicating with the other side.
 * For a socket backend it connects to the server specified via the "mgr.ctrl.socket.ip", "mgr.ctrl.socket.port" configuration in the awe_config
 *
 * @param cfg_p[in]: Pointer to the awe_config object
 * @param cb[in]: LATER - will be used to notify about events; called from within IRQ routine
 * @param ctrl_pp[in/out]: Pointer to variable receiving the awe_CTRL handle
 *
 * @return an error code
 *
 * @todo: role should not be an int,
 * @todo: think about "cfg" parameters in case higher level software has the
 *        need to configure or switch backend at runtime, instead of #defines
 */
int awectrl_init(awe_config* cfg_p, awectrl_cb_t cb, struct awectrl_data **ctrl_pp);

/**
 * Releases the internal data resources
 *
 * @param ctrl_pp[in/out]: Pointer to variable holding the awe_CTRL handle; will be NULL after the call
 *
 * @return an error code
 */
int awectrl_exit(struct awectrl_data **ctrl_pp);

/**
 * Gets the pointer to a channel specific CMD buffer object
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param tunnel_address: selects a specific communication channel
 *
 * @return an error code
 */
int awectrl_get_cmdbuf(struct awectrl_data *ctrl_p, int tunnel_address, struct awecmd_st **cmdbuf_pp);

/**
 * Hands over data to be written to the communication channel and which awaits the response.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 */
int awectrl_transact(struct awectrl_data *ctrl_p, int channel);

/**
 * Explicit version:
 * Hands over data to be written to the communication channel and which awaits the response.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 * @param data: pointer to data to be written
 * @param data_sz: size of the data packet
 * @param response: pointer to a data buffer where response data shall be written
 * @param response_sz: size of the response data buffer
 * @param chn[in]: channel index, only used for trace prints; recommendation: default to -1
 *
 * @return an error code
 *
 * @todo: clarify void* and size in bytes or dwords
 */
int awectrl_transact_explicit(struct awectrl_data *ctrl_p, void* data, int data_sz, void* response, int response_sz, int chn);

/**
 * Explicitly notifies the other communication partner, that
 * data has been written and is available.
 * This could be causing an interrupt, or any other IPC specific
 * method to raise "awareness" on the other side. For polling
 * solutions this method may remain empty.
 *
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 *
 * @todo: clarify if needed, or if implicit notify on awectrl_write() is enough
 */
int awectrl_notify(struct awectrl_data *ctrl_p, int channel);

/**
 * The function needs to be called before the awe manager load design.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 */
int awectrl_load_design_start(struct awectrl_data *ctrl_p, int channel);

/**
 * The function needs to be called after the awe manager load design.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 */
int awectrl_load_design_end(struct awectrl_data *ctrl_p, int channel);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWE_CTRL_H
