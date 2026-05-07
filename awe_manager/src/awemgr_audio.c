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
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/
enum awemgr_rc awemgr_audio_stop(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, ctx_p->tunnel_address, &buf_p));

    awecmd_AudioStop(buf_p, 0);

    enum awemgr_rc rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, NULL, 0, NULL);

    (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);

    return rc;
}


enum awemgr_rc awemgr_audio_start(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, ctx_p->tunnel_address, &buf_p));

    awecmd_AudioStart(buf_p, ctx_p->instanceId);

    enum awemgr_rc rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, NULL, 0, NULL);

    (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);

    return rc;
}

