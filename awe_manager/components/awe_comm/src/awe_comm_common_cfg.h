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

#ifndef INCLUSION_GUARD_AWE_COMM_COMMON_CFG_H
#define INCLUSION_GUARD_AWE_COMM_COMMON_CFG_H

#include "awe_config.h"

#define CFG_COMM_BUFFER_SIZE            "mgr.comm.buffersize"
#ifdef AWEMGR_AWECORE_CONNECTION_SOCKET
    #define DEFAULT_TUNEMSG_SIZE_IN_WORDS   "264"
#endif
#ifdef AWEMGR_AWECORE_CONNECTION_CSHMEM
    #define DEFAULT_TUNEMSG_SIZE_IN_WORDS   "4096"
#endif

#define CFG_COMM_TIMEOUT         "mgr.comm.timeoutms"
#define DEFAULT_COMM_TIMEOUT     "2000"

#endif // INCLUSION_GUARD_AWE_COMM_COMMON_CFG_H
