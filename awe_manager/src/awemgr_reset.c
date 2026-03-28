/* MIT License
**
** Copyright (c) 2025 DSP Concepts, Inc.
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
#include "awemgr_util.h"   // for eg AWEMGR_FAIL_ON_HANDLE_NULL

/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/
enum awemgr_rc awemgr_reset_state_all(struct awemgr_data* mgr_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);

    unsigned int  nr_awe_instances = 0;
    unsigned int* instance_numbers = NULL;
    enum awemgr_rc rc = get_awe_instance_ids(mgr_p, &nr_awe_instances, &instance_numbers);
    if (rc != awemgr_RC_OK)
        return rc;

    // retrieve correct communication buffer here too
    struct awecmd_st *buf_p;
    awectrl_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p);

    for (unsigned int idx = 0; idx < nr_awe_instances; idx++)
    {
        int endpoint = instance_numbers[idx]/16;
        int instance = instance_numbers[idx]%16;
        AWEMGR_API_LOGD(" Endpoint %d Instance %d", endpoint, instance);

        awecmd_resetState(buf_p, endpoint, instance);

        rc = awectrl_transact(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            if(rc == AWECTRL_RC_TIMEOUT)
            {
                return awemgr_RC_COMM_TIMEOUT;
            }
            return awemgr_RC_ERR;
        }
    }

    return awemgr_RC_OK;
}
