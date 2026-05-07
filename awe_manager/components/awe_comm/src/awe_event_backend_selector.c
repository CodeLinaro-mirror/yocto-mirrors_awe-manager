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

#include "awe_event_backend_selector.h"

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
#include "awe_event_socket.h"
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
#include "awe_event_aweq.h"
#endif

/* ****************************************************************************
* PUBLIC FUNCTIONS
* ***************************************************************************/

awe_evt_backend* create_event_backend(awe_config *cfg_p, aweevt_listener cb, void* userdata)
{

#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    return create_event_backend_aweq(cfg_p, cb, userdata);
#endif

#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    return create_event_backend_socket(cfg_p, cb, userdata);
#endif
}