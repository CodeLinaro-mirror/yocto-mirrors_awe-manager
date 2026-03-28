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
 * PRIVATE FUNCTIONS
 * ***************************************************************************/

static void copy_internal_info_to_extern(TargetInfo* in, awemgr_targetinfo_per_core* ext)
{
    ext->m_sampleRate = in->m_sampleRate;
    ext->m_profileClockSpeed = in->m_profileClockSpeed;
    ext->m_proxy_buffer_size = in->m_proxy_buffer_size;
    ext->m_coreClockSpeed = in->m_coreClockSpeed;
    ext->m_features = in->m_features;
    ext->m_base_block_size = in->m_base_block_size;
    ext->m_coreID = in->m_coreID;
    ext->m_version = in->m_version;
    ext->nr_threads = awecmd_target_info_get_threads(in);
    ext->block_size = awecmd_target_info_get_blocksize(in);

    const char *ver = awecmd_target_info_get_version(in);
    if (ver)
        strlcpy(ext->version, ver, sizeof(ext->version));
    else
        ext->version[0] = '\0';

    const char *ver_long = awecmd_target_info_get_version_long(in);
    if (ver_long)
        strlcpy(ext->version_long, ver_long, sizeof(ext->version_long));
    else
        ext->version_long[0] = '\0';

    const char *name = awecmd_target_info_get_name(in);
    if (name)
        strlcpy(ext->targetName, name, sizeof(ext->targetName));
    else
        ext->targetName[0] = '\0';
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

enum awemgr_rc  awemgr_get_target_info(struct awemgr_data* mgr_p, awemgr_targetinfo* info_buffer)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info_buffer);

    unsigned int* instance_numbers = NULL;
    enum awemgr_rc rc = get_awe_instance_ids(mgr_p, &info_buffer->nr_awe_instances, &instance_numbers);
    if (rc != awemgr_RC_OK)
        return rc;

    AWEMGR_API_LOGD("Found %d instances", info_buffer->nr_awe_instances);

    // retrieve correct communication buffer here too
    struct awecmd_st *buf_p;
    awectrl_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p);

    for (unsigned int idx = 0; idx < info_buffer->nr_awe_instances; idx++)
    {
        int endpoint = instance_numbers[idx]/16;
        int instance = instance_numbers[idx]%16;
        AWEMGR_API_LOGI(" Endpoint %d Instance %d", endpoint, instance);
        // call awe_CMD to construct tuning message

        awecmd_getTargetInfo(buf_p, endpoint, instance);
        // todo: "todo: ctx_p->instanceId currently not endpointId. Fix me! Variable renaming should be done to be more consistent

        rc = awectrl_transact(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            if(rc == AWECTRL_RC_TIMEOUT)
            {
                return awemgr_RC_COMM_TIMEOUT;
            }
            return awemgr_RC_ERR;
        }

        unsigned int target_info_data[60];
        unsigned int nr_words_obtained;

        rc = awecmd_response_getData(buf_p, target_info_data, sizeof(target_info_data), &nr_words_obtained);

        COPY_AWE_ERROR(mgr_p, buf_p);
        if(rc == AWECMD_RC_AWE_ERR)
        {
            AWEMGR_API_LOGE("AWECore returned error for getting target info");
            return awemgr_RC_AWECORE_ERROR;
        }
        else if(rc == AWECMD_RC_ERR)
        {
            return awemgr_RC_ERR;
        }

        char* target_info_s = awecmd_target_info_string(&((TargetInfo*)target_info_data)[0]);
        AWEMGR_API_LOGI("%s", target_info_s);

        copy_internal_info_to_extern((TargetInfo*)target_info_data, &(info_buffer->instance[idx]));
    }

    return awemgr_RC_OK;
}
