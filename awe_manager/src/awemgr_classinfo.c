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
#include "awosal_string.h"  // for eg strlcpy

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

enum awemgr_rc  awemgr_get_modulelist_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_modulelist_info* info_buffer_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info_buffer_p);

    struct awecmd_st *buf_p;
    awectrl_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p);

    awecmd_getClassCount(buf_p, endpointId, coreId);

    unsigned int data_buffer[8];
    unsigned int nr_words_in_data_buffer;
    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, &mgr_p->awe_error, data_buffer, 8, &nr_words_in_data_buffer);
    if (rc != awemgr_RC_OK)
        return rc;

    info_buffer_p->nr_classes = data_buffer[0];
    AWEMGR_API_LOGD("Found %d classes on endpoint %d", info_buffer_p->nr_classes, endpointId);

    if (info_buffer_p->nr_classes > MAX_NR_CLASSINFOS)
    {
        AWEMGR_API_LOGE("Too many classes (%d) found for storing in awemgr_modulelist_info. Update implementation!", info_buffer_p->nr_classes);
        return awemgr_RC_ERR;
    }

    for (unsigned int idx = 0; idx < info_buffer_p->nr_classes; idx++)
    {
        awecmd_getClassInfo(buf_p, endpointId, coreId, idx);

        enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, &mgr_p->awe_error, data_buffer, 8, &nr_words_in_data_buffer);

        if (rc != awemgr_RC_OK)
            return rc;

        info_buffer_p->classes[idx].classId = data_buffer[0];
        info_buffer_p->classes[idx].nrParameters = data_buffer[1];
    }
    return awemgr_RC_OK;
}
