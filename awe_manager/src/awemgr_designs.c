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
#include "awe_awc.h"
#include "awe_ctrl.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h"  // for eg strlcpy

#include <string.h>


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int  awemgr_get_design_count(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    return awc_design_count(ctx_p->awc);
}


enum awemgr_rc awemgr_load_design(struct awemgr_ctx *ctx_p, const char *designName)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(designName);

    int rc = awemgr_RC_ERR;
    awc_design_t *pDesign = awc_get_design(ctx_p->awc, designName);
    if (pDesign == NULL)
    {
        AWEMGR_API_LOGE("Could not find design in AWC: %s", designName);
        return awemgr_RC_ERR;
    }

    //uint8_t reserved = (ctx_p->pDesign->coreid_objectid >> 24) & 0xff;
    uint8_t coreId = (pDesign->coreid_objectid >> 16) & 0xff;
    uint32_t objectId = pDesign->coreid_objectid & 0xffff;

    AWEMGR_API_LOGI("Found design: %s; instanceId: %d, objectId: %d, file: %s", pDesign->name, coreId, objectId, pDesign->file);

    char fullpath[MAX_AWEMGR_FILENAME_LEN];
    int written = snprintf(fullpath, sizeof(fullpath), "%s/%s", ctx_p->filename_path, pDesign->file);
    if (written >= sizeof(fullpath)) {
        AWEMGR_API_LOGE("Full File path %s/%s longer than buffer size (%d bytes)", ctx_p->filename_path, pDesign->file, MAX_AWEMGR_FILENAME_LEN);
        return awemgr_RC_ERR;
    }

    AWEMGR_API_LOGI("Loading AWB file %s (instanceId: %d)", fullpath, ctx_p->instanceId);

    FILE *fp = fopen(fullpath, "rb");

    if (!fp)
    {
        AWEMGR_API_LOGE("AWC knows about design '%s'. But file could not be loaded: %s",
                        designName, fullpath);
        return awemgr_RC_ERR;
    }
    // check which transmission buffer is required, if it needs to be a prefixed or "normal" one
    struct awecmd_st *buf_p;
    int tunnel_address = pDesign->coreid_objectid;
    rc = awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnel_address, &buf_p);
    if(rc != AWECTRL_RC_OK)
    {
        goto FAIL_RET;
    }

    int cnt = awecmd_from_stream(buf_p, fp, ctx_p->instanceId, coreId, objectId);
    AWEMGR_API_LOGD(" - obtained next cmd with %d words (instanceId: %d)", cnt, ctx_p->instanceId);
    while (cnt > 0)
    {
        rc = awectrl_transact(ctx_p->parent->comm_2_awe, (tunnel_address != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            goto FAIL_RET;
        }

        rc = awecmd_response_getData(buf_p, NULL, 0, NULL);

        COPY_AWE_ERROR(ctx_p->parent, buf_p);
        if (rc != AWECMD_RC_OK)
        {
            goto FAIL_RET;
        }

        awecmd_reset(buf_p, tunnel_address);
        cnt = awecmd_from_stream(buf_p, fp, ctx_p->instanceId, coreId, objectId);
        AWEMGR_API_LOGD(" - obtained next cmd with %d words (instanceId: %d)", cnt, ctx_p->instanceId);
    }

    fclose(fp);

    AWEMGR_API_LOGI("AWB data applied for %s (instanceId: %d)", pDesign->name, ctx_p->instanceId);
    if (pDesign->coreid_objectid == 0)
    {
        ctx_p->pDesign = pDesign;
    }
    return awemgr_RC_OK;

FAIL_RET:
    fclose(fp);
    if(rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("AWECore returned error");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(rc == AWECTRL_RC_TIMEOUT)
    {
        return awemgr_RC_COMM_TIMEOUT;
    }
    return awemgr_RC_ERR;
}


enum awemgr_rc awemgr_unload_design(struct awemgr_ctx *ctx_p, const char *designName)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    awc_design_t *pDesign = ctx_p->pDesign;
    if (designName != NULL)
    {
        pDesign = awc_get_design(ctx_p->awc, designName);
        if (pDesign == NULL)
        {
            AWEMGR_API_LOGE("Could not find design in AWC: %s", designName);
            return awemgr_RC_ERR;
        }
    }

    if (pDesign != NULL)
    {
        struct awecmd_st *buf_p;

        int rc = awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, pDesign->coreid_objectid, &buf_p);

        awecmd_Destroy(buf_p, ctx_p->instanceId);

        rc = awectrl_transact(ctx_p->parent->comm_2_awe, (pDesign->coreid_objectid != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            if(rc == AWECTRL_RC_TIMEOUT)
            {
                return awemgr_RC_COMM_TIMEOUT;
            }
            return awemgr_RC_ERR;
        }

        // todo: local response_buffer; have central response message error check in awe_CMD !

        AWEMGR_API_LOGI("AWE design stopped and unloaded: %s", pDesign->name);
        if (pDesign->coreid_objectid == 0)
        {
            ctx_p->pDesign = NULL;
        }
    }
    return awemgr_RC_OK;
}


enum awemgr_rc  awemgr_get_design_info(struct awemgr_ctx *ctx_p, int index, struct awemgr_design_info *info)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info);

    awc_design_t *design_p = awc_get_design_by_index(ctx_p->awc, index);
    if(design_p == NULL)
    {
        AWEMGR_API_LOGE("Could not find design at index: %d", index);
        return awemgr_RC_ERR;
    }

    info->name = design_p->name;
    info->coreid_objectid = design_p->coreid_objectid;
    info->size = design_p->size;
    info->md5sum = design_p->md5sum;
    return awemgr_RC_OK;
}

enum awemgr_rc awemgr_skip_unload_design_on_exit(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    if (ctx_p->pDesign) {
        AWEMGR_API_LOGI("Marking AWC %s so that signal flow will NOT be stopped on AWE-Manager exit.", ctx_p->pDesign->name);
        ctx_p->skip_unload_on_exit = true;
    }

    return awemgr_RC_OK;
}