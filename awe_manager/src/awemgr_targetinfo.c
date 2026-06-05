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

#include "awe_manager.h"
#include "types/awemgr_data.h"
#include "awe_comm.h"
#include "awe_cmd.h"
#include "awe_cmd_responses.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h"  // for eg strlcpy


/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/

static void copy_internal_info_to_extern(unsigned int instance, TargetInfo* in, ExtendedInfo* ext, awemgr_targetinfo_per_core* out)
{
    out->instance_id = instance;
    out->m_sampleRate = in->m_sampleRate;
    out->m_profileClockSpeed = in->m_profileClockSpeed;
    // unpack stuff from m_proxy_buffer_size
    out->commbuffer_size = GET_TARGET_PACKET_BUFFER_LEN(in);
    out->nr_chan_in = TARGET_INFO_NUM_INPUTS(in);
    out->nr_chan_out = TARGET_INFO_NUM_OUTPUTS(in);
    out->nr_cores = GET_TARGET_CORES(in);

    out->m_coreClockSpeed = in->m_coreClockSpeed;
    out->m_features = in->m_features;
    out->m_base_block_size = in->m_base_block_size;
    out->m_coreID = in->m_coreID;
    out->m_version = in->m_version;
    out->nr_threads = GET_TARGET_THREADS(in);
    out->block_size = GET_TARGET_BASE_BLOCK_SIZE(in);

    const char *ver = awecmd_target_info_get_version(in);
    if (ver)
        strlcpy(out->version, ver, sizeof(out->version));
    else
        out->version[0] = '\0';

    const char *ver_long = awecmd_target_info_get_version_long(in);
    if (ver_long)
        strlcpy(out->version_long, ver_long, sizeof(out->version_long));
    else
        out->version_long[0] = '\0';

    const char *name = awecmd_target_info_get_name(in);
    if (name)
        strlcpy(out->targetName, name, sizeof(out->targetName));
    else
        out->targetName[0] = '\0';

    const char* proctype = awecmd_target_info_get_proc_type(in);
    out->proc_type = proctype ? proctype : "Unknown";

    out->build_nr = ext->buildNumber;
    out->user_version = ext->userVersion;
    out->hotfix_version = awecmd_extended_info_get_hotfix_version(ext);
    out->alignment_size = awecmd_extended_info_get_alignment_size(in, ext);
    out->supports_module_reset = awecmd_extended_info_has_module_reset(ext);
    out->supports_fract16 = awecmd_extended_info_has_fract16_support(ext);
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

    struct awecmd_st *buf_p = NULL;


    for (unsigned int idx = 0; idx < info_buffer->nr_awe_instances; idx++)
    {
        unsigned int endpoint = instance_numbers[idx]/16;
        unsigned int instance = instance_numbers[idx]%16;
        AWEMGR_API_LOGI(" Endpoint %d Instance %d", endpoint, instance);
        // call awe_CMD to construct tuning message

        AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));
        awecmd_getTargetInfo(buf_p, endpoint, instance);
        // todo: "todo: ctx_p->instanceId currently not endpointId. Fix me! Variable renaming should be done to be more consistent

        unsigned int target_info_data[60] = { 0 };
        unsigned int nr_words_obtained = 0;

        enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p,
            target_info_data, sizeof(target_info_data), &nr_words_obtained);

        (void) awecomm_release_lock(mgr_p->comm_2_awe);

        if (rc != awemgr_RC_OK)
        {
            break;
        }

        char* target_info_s = awecmd_target_info_string(&((TargetInfo*)target_info_data)[0]);
        AWEMGR_API_LOGI("%s", target_info_s);

        // also get extended info
        AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));
        awecmd_getExtendedInfo(buf_p, endpoint, instance);

        unsigned int extended_info_data[13] = { 0 };

        rc = safe_transact(mgr_p->comm_2_awe, buf_p,
            extended_info_data, sizeof(extended_info_data), &nr_words_obtained);
        (void) awecomm_release_lock(mgr_p->comm_2_awe);

        if (rc != awemgr_RC_OK)
        {
            break;
        }

        copy_internal_info_to_extern(instance, (TargetInfo*)target_info_data, (ExtendedInfo*)extended_info_data, &(info_buffer->instance[idx]));
    }

    return awemgr_RC_OK;
}
