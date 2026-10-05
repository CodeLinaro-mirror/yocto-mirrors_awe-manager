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

#ifndef INCLUSION_GUARD_AWE_COMM_AWEQ_CFG_H
#define INCLUSION_GUARD_AWE_COMM_AWEQ_CFG_H

#include "awe_config.h"

/* Default tuning/communication packet buffer size in 32-bit words for the AWEQ backend.
   This value comes from the BSP and shared memory configuration in that project.
   Serves both as the registered default for CFG_COMM_BUFFER_SIZE and as the
   runtime fallback (see awecomm_init) when the key cannot be read. The value
   depends on the selected AWECore connection backend. */
#define CFG_COMM_BUFFER_SIZE_DEFAULT    4096


#ifdef __cplusplus
extern "C" {
#endif

// define CFG items here later

int register_comm_backend_aweq_configs(awe_config* cfg_p);

#ifdef __cplusplus
}
#endif

#endif // INCLUSION_GUARD_AWE_COMM_AWEQ_CFG_H
