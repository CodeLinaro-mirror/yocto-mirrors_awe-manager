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
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

enum awemgr_rc  awemgr_transact(struct awemgr_data* mgr_p, void* request_buffer, int request_buffer_size, void* response_buffer, int response_buffer_size)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);

    /* Serialize against all other awemgr_* callers that go through
     * awecomm_get_cmdbuf / awecomm_release_lock.  Without this lock the
     * TX-RX round-trip on the backend link can interleave with a
     * concurrent shell-command transaction, corrupting both responses.
     */
    if (awecomm_acquire_lock(mgr_p->comm_2_awe) != AWECOMM_RC_OK)
        return awemgr_RC_ERR;

    int comm_rc = awecomm_transact_explicit(mgr_p->comm_2_awe,
        request_buffer, request_buffer_size,
        response_buffer, response_buffer_size, 0);

    (void) awecomm_release_lock(mgr_p->comm_2_awe);

    if(comm_rc != AWECOMM_RC_OK)
    {
        if(comm_rc == AWECOMM_RC_TIMEOUT)
        {
            return awemgr_RC_COMM_TIMEOUT;
        }
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;

}
