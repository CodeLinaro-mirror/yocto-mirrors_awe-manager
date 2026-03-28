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
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/
enum awemgr_rc awemgr_audio_stop(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    struct awecmd_st *buf_p;
    awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, ctx_p->tunnel_address, &buf_p);

    awecmd_AudioStop(buf_p, 0);

    int rc = awectrl_transact(ctx_p->parent->comm_2_awe, (ctx_p->tunnel_address != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
    if(rc != AWECTRL_RC_OK)
    {
        AWEMGR_API_LOGE("Failed to transact AudioStop Command");
        if(rc == AWECTRL_RC_TIMEOUT)
        {
            return awemgr_RC_COMM_TIMEOUT;
        }
        return awemgr_RC_ERR;
    }

    rc = awecmd_response_getData(buf_p, NULL, 0, NULL);

    COPY_AWE_ERROR(ctx_p->parent, buf_p);
    if(rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("AWECore returned error for AudioStop Command");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(rc == AWECMD_RC_ERR)
    {
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
}


enum awemgr_rc awemgr_audio_start(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    struct awecmd_st *buf_p;
    awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, ctx_p->tunnel_address, &buf_p);

    awecmd_AudioStart(buf_p, ctx_p->instanceId);

    int rc = awectrl_transact(ctx_p->parent->comm_2_awe, (ctx_p->tunnel_address != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
    if(rc != AWECMD_RC_OK)
    {
        AWEMGR_API_LOGE("Failed to transact AudioStart Command");
        if(rc == AWECTRL_RC_TIMEOUT)
        {
            return awemgr_RC_COMM_TIMEOUT;
        }
        return awemgr_RC_ERR;
    }

    rc = awecmd_response_getData(buf_p, NULL, 0, NULL);

    COPY_AWE_ERROR(ctx_p->parent, buf_p);
    if(rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("AWECore returned error for AudioStart Command");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(rc == AWECMD_RC_ERR)
    {
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
}

