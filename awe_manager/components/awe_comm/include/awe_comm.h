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
It provides very simple methods to abstract the communication between 2 participants.
 */

#ifndef INCLUSION_GUARD_AWE_COMM_H
#define INCLUSION_GUARD_AWE_COMM_H

#include <stdbool.h>
#include <stdio.h>

#include "awe_config.h"
#include "awe_cmd.h"
#include "awe_comm_trace.h"

#if defined(__cplusplus)
extern "C" {
#endif


#define AWECOMM_RC_OK              0
#define AWECOMM_RC_FAIL_COMM      -1
#define AWECOMM_RC_FAIL_RESOURCES -2
#define AWECOMM_RC_FAIL_PARAM     -3
#define AWECOMM_RC_TIMEOUT        -4
#define AWECOMM_RC_RETRY_COMM     -5


struct awecomm_data;


/**
 * Fills the awe_config structure with the required configurations for this component
 *
 * @param cfg_p[in]: Pointer to the awe_config object
 * @return an error code
 */
int awecomm_register_configs(awe_config* cfg_p);

/**
 * Initializes the underlying communication channel (backend), for example it sets up everything
 * inside the carved shared memory, required for communicating with the other side.
 * For a socket backend it connects to the server specified via the "mgr.comm.socket.ip", "mgr.comm.socket.port" configuration in the awe_config
 *
 * @param cfg_p[in]: Pointer to the awe_config object
 * @param cb[in]: Callback invoked when receiving data asynchronously (events) from the other side
 * @param userdata_p[in]: User data pointer passed back in the event callback
 * @param ctrl_pp[in/out]: Pointer to variable receiving the awe_CTRL handle
 *
 * @return an error code
 *
 * @todo: role should not be an int,
 * @todo: think about "cfg" parameters in case higher level software has the
 *        need to configure or switch backend at runtime, instead of #defines
 */
int awecomm_init(awe_config* cfg_p, aweevt_listener cb, void *userdata_p, struct awecomm_data **ctrl_pp);

/**
 * Releases the internal data resources
 *
 * @param ctrl_pp[in/out]: Pointer to variable holding the awe_CTRL handle; will be NULL after the call
 *
 * @return an error code
 */
int awecomm_exit(struct awecomm_data **ctrl_pp);

/**
 * Gets the pointer to a channel specific CMD buffer object.
 *
 * NOTE: This also locks the transmission for exclusive access, so that no other
 * thread can acquire it until awecomm_release_lock() is called.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param tunnel_address: passes a tunnel address that selects a specific communication channel
 *
 * @return an error code
 */
int awecomm_get_cmdbuf(struct awecomm_data *ctrl_p, int tunnel_address, struct awecmd_st **cmdbuf_pp);

/**
 * Acquires the previously acquired buffer/transmission lock.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 *
 * @return an error code
 */
int awecomm_acquire_lock(struct awecomm_data *ctrl_p);

/**
 * Releases the previously acquired buffer/transmission lock.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 *
 * @return an error code
 */
int awecomm_release_lock(struct awecomm_data *ctrl_p);

/**
 * Hands over data to be written to the communication channel and which awaits the response.
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param channel: selects a specific communication channel
 *
 * @return an error code
 */
int awecomm_transact(struct awecomm_data *ctrl_p, int channel);

/**
 * Delivers data in command buffer to the communication channel
 *
 * @param ctrl_p[in]: Pointer to an awe_CTRL object handle.
 * @param buf_p[in]: Pointer to the (filled) command buffer.
 *
 * @return an error code
 */
int awecomm_transact_on_cmdbuf(struct awecomm_data *ctrl_p, struct awecmd_st *buf_p);

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
int awecomm_transact_explicit(struct awecomm_data *ctrl_p, void* data, int data_sz, void* response, int response_sz, int chn);

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
 * @todo: clarify if needed, or if implicit notify on awecomm_write() is enough
 */
int awecomm_notify(struct awecomm_data *ctrl_p, int channel);

/**
 * Install (or remove) a raw-buffer tap on all comm transactions.
 *
 * @p cb is called once for TX and once for RX on every transaction, regardless
 * of the do_trace / trace-file settings.  Pass @p cb = NULL to remove the tap.
 */
void awecomm_set_observer(struct awecomm_data *ctrl_p, awecomm_buf_observer_cb cb, void *ctx);


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWE_COMM_H
