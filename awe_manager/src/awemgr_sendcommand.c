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
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "types/awemgr_data.h"
#include "awe_ctrl.h"
#include "awosal_string.h"


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

 enum awemgr_rc awemgr_send_command(struct awemgr_ctx *ctx_p,
    int cmdId, unsigned int coreId,
    void *payload, unsigned int data_sz_in_words,
    void* response_buffer, unsigned int response_buffer_size_in_words,
    unsigned int *response_buffer_filled_p)
 {
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    if ((payload == NULL && data_sz_in_words != 0) || (payload != NULL && data_sz_in_words == 0))
    {
        AWEMGR_API_LOGE("Invalid argument: payload and data_sz_in_words parameters do not fit together!");
        return awemgr_RC_ERR;
    }
    if ((response_buffer == NULL && response_buffer_size_in_words != 0) || (response_buffer != NULL && response_buffer_size_in_words == 0))
    {
        AWEMGR_API_LOGE("Invalid argument: response_buffer and response_buffer_size_in_words parameters do not fit together!");
        return awemgr_RC_ERR;
    }

    struct awecmd_st *buf_p;
    int rc = awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p);

    unsigned int nr_words_avail_in_buf = (unsigned int) (AWECMD_REQUESTBUFFER_SZ(buf_p) - 4);  // 4 = hdr+subCmd+payloadsize+CRC
    if (data_sz_in_words > nr_words_avail_in_buf) {
        AWEMGR_API_LOGE("Payload size (%d) given does not fit into the transmission buffer (%d)!",
            data_sz_in_words, nr_words_avail_in_buf);
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    awecmd_BSPCmd(buf_p, ctx_p->instanceId, coreId, cmdId, payload, data_sz_in_words);

    rc = awectrl_transact(ctx_p->parent->comm_2_awe, AWEMGR_CHANNEL_0);
    if(rc != AWECTRL_RC_OK)
    {
        if(rc == AWECTRL_RC_TIMEOUT)
        {
            return awemgr_RC_COMM_TIMEOUT;
        }
        return awemgr_RC_ERR;
    }

    // analyze response buffer
    // NOTE: we currently don't handle the "Subcommand Identifier" returned by the PFID_BspCmd
    rc = awecmd_response_getData(buf_p, response_buffer, response_buffer_size_in_words, response_buffer_filled_p);
    COPY_AWE_ERROR(ctx_p->parent, buf_p);
    if(rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("AWECore returned error");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(rc == AWECMD_RC_ERR)
    {
        return awemgr_RC_ERR;
    }

    return awemgr_RC_OK;
 }
