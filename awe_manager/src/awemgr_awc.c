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
#include "awe_cmd.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h"  // for eg strlcpy


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/
int awemgr_get_loaded_awc_count(struct awemgr_data *mgr_p)
{
    if(!mgr_p)
    {
        AWEMGR_API_LOGE("Invalid argument: mgr_p handle == NULL!");
        return -1;
    }
    int nr_awcs = 0;
    for (int s = 0; s < MAX_AWE_ENDPOINTS; s++) {
        if (mgr_p->endpoints[s].instanceId == s && mgr_p->endpoints[s].awc)
        {
            nr_awcs++;
        }
    }
    return nr_awcs;
}

int awemgr_get_max_awcs()
{
    return MAX_AWE_ENDPOINTS;
}

struct awemgr_ctx* awemgr_get_awc_context(struct awemgr_data *mgr_p, int endpointId)
{
    if (!mgr_p)
    {
        AWEMGR_API_LOGE("Invalid argument: mgr_p == NULL! AWE-Manager initialized? Returning NULL!");
        return NULL;
    }
    if (endpointId < 0 || endpointId >= MAX_AWE_ENDPOINTS) {
        AWEMGR_API_LOGE("Invalid argument: endpoint variable value outside of supported values (0 to %d).", MAX_AWE_ENDPOINTS-1);
        return NULL;
    }
    if (mgr_p->endpoints[endpointId].awc ==NULL)
    {
        AWEMGR_API_LOGW("AWC is not loaded on the endpoint: %d", endpointId);
        return NULL;
    }
    return &mgr_p->endpoints[endpointId];
}


enum awemgr_rc awemgr_load_awc(struct awemgr_data *mgr_p, const char *awcFileName, int endpointId)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    if (endpointId < 0 || endpointId >= MAX_AWE_ENDPOINTS) {
        AWEMGR_API_LOGE("Invalid argument: endpoint %d is outside of supported values (0 to %d).", endpointId, MAX_AWE_ENDPOINTS-1);
        return awemgr_RC_ERR;
    }
    struct awemgr_ctx *ctx_p = NULL;

    if (mgr_p->endpoints[endpointId].awc !=NULL)
    {
        AWEMGR_API_LOGE("AWC is already loaded on the endpoint: %d", endpointId);
        return awemgr_RC_ERR;
    }

    ctx_p = &mgr_p->endpoints[endpointId];

    // initialize the AWC handler first ...
    ctx_p->awc = awc_init(awcFileName);
    if (! ctx_p->awc)
    {
        AWEMGR_API_LOGE("AWC data not available!");
        return awemgr_RC_ERR;
    }

    // check schema version of AWC
    awc_info_t *awc_info = awc_get_info(ctx_p->awc);
    if (awc_info)
    {
        if (awc_info->schema > AWEMGR_MAX_AWC_SCHEMA)
        {
            awc_uninit(&ctx_p->awc);
            AWEMGR_API_LOGE("AWC file has incorrect schema version: Supported: <=%d; AWC: %d",
                            AWEMGR_MAX_AWC_SCHEMA, awc_info->schema);
            return awemgr_RC_ERR;
        }
        // we might still have a newer AWC schema; it is a bit unclear what would happen,
        // or if this might happen at all: a new AWC schema might have added a new item/column
        // which is potentially backward compatible. So, for now we only print a fat warning
        if (awc_info->schema > AWEMGR_MAX_AWC_SCHEMA)
        {
            AWEMGR_API_LOGW("!!! AWC file schema mismatch !!! You are in unchartered territory !!! Required: %d; AWC: %d",
                            AWEMGR_MAX_AWC_SCHEMA, awc_info->schema);
            return awemgr_RC_ERR;
        }
    }
    else
    {
        AWEMGR_API_LOGE("AWC file info struct could not be retrieved. Check file!");
        return awemgr_RC_ERR;
    }

    ctx_p->tunnel_address = 0; // default is no tunnel address set
    const awc_dict_element *tunnelAddressInfo_p = awc_get_top_userdata_by_key(ctx_p->awc, "tunnelAddress");
    if (tunnelAddressInfo_p != NULL)
    {
        ctx_p->tunnel_address = tunnelAddressInfo_p->value.i32;
    }

    // remember directory where the AWC index file is stored;
    strlcpy(ctx_p->filename_path, awcFileName, MAX_AWEMGR_FILENAME_LEN);
    ctx_p->filename_path[MAX_AWEMGR_FILENAME_LEN - 1] = '\0';
    char* last_delim = strrchr(ctx_p->filename_path, '/');
    if (! last_delim)
        last_delim = strrchr(ctx_p->filename_path, '\\');

    if (! last_delim)
    {
        AWEMGR_API_LOGE("Problems finding directory of AWC file name %s", ctx_p->filename_path);
        awc_uninit(&ctx_p->awc);
        return AWECMD_RC_ERR;
    }
    *last_delim = '\0';

    ctx_p->parent = mgr_p;
    ctx_p->pDesign = NULL;
    ctx_p->instanceId = endpointId;
    ctx_p->skip_unload_on_exit = false;  // default is to remove design

    mgr_p->nr_designs += 1;
    AWEMGR_API_LOGI("AWC context created and control info loaded for instanceId %d", ctx_p->instanceId);

    return awemgr_RC_OK;
}


enum awemgr_rc awemgr_unload_awc(struct awemgr_data *mgr_p, int endpointId)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    if (endpointId < 0 || endpointId >= MAX_AWE_ENDPOINTS) {
        AWEMGR_API_LOGE("Invalid argument: endpoint variable value outside of supported values (0 to %d).", MAX_AWE_ENDPOINTS-1);
        return awemgr_RC_ERR;
    }

    struct awemgr_ctx *ctx_p = NULL;
    if (mgr_p->endpoints[endpointId].awc)
    {
        ctx_p = &mgr_p->endpoints[endpointId];
    }
    else
    {
        AWEMGR_API_LOGD("AWC is not loaded on the endpoint: %d", endpointId);
        return awemgr_RC_OK;
    }

    // check if we should tear down a running AWE Core instance too;
    if (! ctx_p->skip_unload_on_exit) {
        awemgr_unload_design(ctx_p, NULL); // no need to check for return code; cannot return error
    }

    if (ctx_p->awc)
    {
        awc_uninit(&ctx_p->awc);
        AWEMGR_API_LOGI("AWC unloaded: %s", ctx_p->filename_path);

        mgr_p->nr_designs--;
        ctx_p->parent = NULL;
    }

    return awemgr_RC_OK;
}
