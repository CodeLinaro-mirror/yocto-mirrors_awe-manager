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

#include "awe_manager.h"
#include "types/awemgr_data.h"
#include "awe_ctrl.h"
#include "awe_cmd.h"
#include "awe_cmd_responses.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL


/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

enum awemgr_rc  awemgr_get_cpu_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_cpuinfo* info_buffer_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info_buffer_p);

    struct awecmd_st *buf_p;
    awectrl_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p);

    awecmd_getCpuLoad(buf_p, endpointId, coreId);

    // the response is encoded into rx_buffer (0: AverageCycles, 1: TimePerProc )
    unsigned int data_buffer[2];
    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, &mgr_p->awe_error, data_buffer, 2);
    if (rc == awemgr_RC_OK)
    {
        info_buffer_p->AverageCycles = data_buffer[0];
        info_buffer_p->TimePerProcess = data_buffer[1];

        AWEMGR_API_LOGD("CPU %d/%d load", info_buffer_p->AverageCycles, info_buffer_p->TimePerProcess);

    }
    return rc;
}
