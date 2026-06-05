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

#ifndef INCLUSION_GUARD_AWE_EVNT_H
#define INCLUSION_GUARD_AWE_EVNT_H

#include <stdint.h>
#include "awe_event_backend.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct aweevent_data;
typedef struct aweevent_data aweevent_data;

/**
 * Fills the awe_config structure with the required configurations for this component
 *
 * @param cfg_p[in]: Pointer to the awe_config object
 * @return an error code
 */
int aweevent_register_configs(awe_config* cfg_p);

/**
 * Initializes the underlying communication channel with BSP and buffers for events handling
 *
 * @param cfg_p[in]: Pointer to the awe_config object
 * @param evnt_pp[out]: on success, evnt_pp is assigned the address of underlying aweevnt_data_t structure pointer.
 * @param listener[in]: event backend notifies the listener by calling this callback.
 * @param userdata[in]: the data is used by the event listener and passed in the listener callback.
 * @return an error code
 */
int aweevent_init(awe_config *cfg_p, aweevent_data **evnt_pp, aweevt_listener listener, void* userdata);

/**
 * Releases the internal data resources
 *
 * @param event_pp[in/out]: Pointer to variable holding the aweevnt_data handle; will be NULL after the call
 *
 * @return an error code
 */
int aweevent_exit(aweevent_data  **event_pp);

/**
 * Blocking function to read the event data from the BSP
 *
 * @param evnt_p[in]: Pointer to an aweevnt_data object handle.
 * @param timeoutMs[in] - 0 = Blocking mode or > 0 API exits if no event could be received in timeout milliseconds.
 * @return an error code
 */
int aweevent_read(aweevent_data *evnt_p, uint32_t timeoutMs);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWE_EVNT_H