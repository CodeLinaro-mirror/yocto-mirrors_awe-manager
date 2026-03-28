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

#ifndef INCLUSION_GUARD_AWE_MANAGER_INTERNAL_TYPES_AWEMGRCTX_H
#define INCLUSION_GUARD_AWE_MANAGER_INTERNAL_TYPES_AWEMGRCTX_H

#include <stdbool.h>
#include "types/awemgr_limits.h"
#include "types/awemgr_ctx.h"

#include "awe_manager.h" // only for MAX_AWEMGR_DESIGNS and event_cb_t :(
#include "awe_event.h"   // for aweevent_data

/**
 * structure to hold an event callback function pointer and its correlated user data pointer
 */
struct evt_category_cb_st {
    event_cb_t cb;
    void *user_data_p;
};

/**
 * AWE Manager specific internal data structure, forward declared in public header file and
 * therefore invisible for applications/clients
 *
 * This is the "main" structure of an AWE-Manager instance; it is created upon call to awemgr_init() and
 * de-allocated upon awemgr_exit().
 *
 * It holds the handles for the communication with an AWE-Core instance ("BSP"),
 * for sending "down" tuning commands and for receiving events from the signal flows or from BSP.
 *
 * Each signal flow or design handled via AWE-Manager API is handled by a (sub) structure `awemgr_ctx`
 * (multi-canvas support).
 *
 */
typedef struct awemgr_data {
    // nr of AWE designs executed in the system ("cardId")
    int                nr_designs;

    // Array of Endpoint structure objects
    awemgr_ctx  endpoints[MAX_AWE_ENDPOINTS];

    // internal data buffer structure to generate and consume AWE tuning messages
    // the handler for the struct manages the communication to the DSP/Audio Processors
    // struct awecmd_st   msg_buffer;

    // handler for the communication to AWE HostTask
    // backend specific code handled in awe_CTRL component
    struct awectrl_data *comm_2_awe;

    // Event listener default callback function
    struct evt_category_cb_st evt_cb_default;

    // Array of event category specific callback functions
    struct evt_category_cb_st evt_category_cb[MAX_SUPPORTED_CATEGORIES];

    // Handler containing event communication backend, header structure and pointer to payload
    aweevent_data *event_data;

    // Pointer to the awe_config object, if passed in from client code in awemgr_init() 
    // it points to the external structure, no memory ownership is maintained then.
    awe_config *config;

    //If true, the *config pointer is handled AWE-Manager internally. Otherwise *config points to an externally allocated config structure
    bool config_internally_allocated;

    // Stores the AWECore error information (error code and Description) 
    awecore_error_t awe_error;
} awemgr_data;

#endif // INCLUSION_GUARD_AWE_MANAGER_INTERNAL_TYPES_AWEMGRCTX_H