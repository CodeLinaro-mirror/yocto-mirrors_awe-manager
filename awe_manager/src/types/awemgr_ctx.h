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

#ifndef INCLUSION_GUARD_AWE_MANAGER_TYPES_CTX_H
#define INCLUSION_GUARD_AWE_MANAGER_TYPES_CTX_H

#include "types/awemgr_limits.h"
#include "awe_types.h" // for awc_design_t: part of awe_AWC; TODO: check if that file should not be split too; contains basic types like UINT8 stuff

/**
 * Structure to hold all information relevant for a single (possibly multi-core) design or signal flow.
 *
 * It contains
 *   - the file path to the configuration file (awc_index.txt file typically)
 *   - all "tunable" or controllable items in this design (handle to AWC)
 *   - the information which "endpointID" this signal flow runs at (multi-canvas case!)
 *   - an AWE-Manager instance handle (pointer to "parent")
 *
 */
struct awemgr_data;

typedef struct awemgr_ctx {

    // handle to AWC handling component
    void* awc;             // TODO: will turn into a forward declared "handle" instead of void*

    // copy the ptr to the design info; it's a convenience ptr into "awc"
    awc_design_t *pDesign;

    // copy of the design's instance or core ID (0, 16, ...)  // TODO: should be endpointId
    int           instanceId;

    // variable to hold the prefix which allows to construct tuning messages for Subcanvases
    int           tunnel_address;

    // pointer to the "higher-level" AWE Manager handle
    struct awemgr_data *parent;

    // filename to the AWC index file
    char  filename_path[MAX_AWEMGR_FILENAME_LEN];

    // flag to indicate that this AWC context has event handling attached
    // TODO: this is currently a 1x1 relation to the "parent->event_data" structure,
    //       but it may be different in the future, when every AWC context independently
    //       enables/disables event reporting
    bool events_handling_enabled;

    // if true, this context/canvas will not unload the design it may have loaded
    // when the context is removed (unload_awc).
    // this means that when AWE Manager is exited and the AWC in removed,
    // no specific DESTROY command to AWE Core is issued and the signal flow
    // continues to process
    bool skip_unload_on_exit;

} awemgr_ctx;


#endif // INCLUSION_GUARD_AWE_MANAGER_TYPES_CTX_H