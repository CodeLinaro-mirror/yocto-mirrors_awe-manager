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
#include <string.h>  // memset


/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/
static enum awemgr_rc nr_heaps(struct awemgr_data* mgr_p, unsigned int endpointId, int coreId, unsigned int *nr_heaps_p)
{
    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));

    awecmd_getHeapCount(buf_p, endpointId, coreId);

    *nr_heaps_p = 0;   // todo: if this becomes a public fct check on nr_heaps_p == NULL
    unsigned int data_buffer[2];
    unsigned int nr_words_in_data_buffer;
    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, data_buffer, 2, &nr_words_in_data_buffer);

    (void) awecomm_release_lock(mgr_p->comm_2_awe);

    if (rc == awemgr_RC_OK)
    {
        AWEMGR_API_LOGD("Found %d heaps on endpoint %d", data_buffer[0], 0);
        *nr_heaps_p = data_buffer[0];
    }
    return awemgr_RC_OK;
}

static enum awemgr_rc get_standard_heaps_raw(struct awemgr_data* mgr_p, unsigned int endpointId, unsigned int coreId, unsigned int *raw_heaps, unsigned int max_raw_heaps)
{
    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));

    awecmd_getHeapSize(buf_p, endpointId, coreId);

    unsigned int nr_words_payload;

    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, raw_heaps, max_raw_heaps, &nr_words_payload);

    (void) awecomm_release_lock(mgr_p->comm_2_awe);

    return rc;
}

static enum awemgr_rc get_shared_heap(struct awemgr_data* mgr_p, unsigned int endpointId, unsigned int coreId, awemgr_heap *heap_p)
{
    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));

    awecmd_getSharedHeapSize(buf_p, endpointId, coreId);

    unsigned int data_buffer[2];
    unsigned int nr_words_in_data_buffer;

    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, data_buffer, 2, &nr_words_in_data_buffer);

    (void) awecomm_release_lock(mgr_p->comm_2_awe);

    if (rc == awemgr_RC_OK)
    {
        AWEMGR_API_LOGD("Found %d heaps on endpoint %d", data_buffer[0], 0);
        heap_p->nr_free = data_buffer[0];
        heap_p->size = data_buffer[1];
    }

    return awemgr_RC_OK;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

 enum awemgr_rc awemgr_get_heap_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_heapinfo* info_buffer_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info_buffer_p);
    AWEMGR_FAIL_ON_ENDPOINT_INVALID(endpointId);
    AWEMGR_FAIL_ON_CORE_INVALID(coreId);

    enum awemgr_rc rc;

    memset(info_buffer_p, 0, sizeof(awemgr_heapinfo)); // ensure all heaps in output struct are 0

    // NUMBER OF HEAPS
    rc = nr_heaps(mgr_p, endpointId, coreId, &info_buffer_p->nr_heaps);
    if (rc != awemgr_RC_OK) {
        return rc;
    }

    // "standard" HEAP SIZES
    unsigned int rx_buffer[16];
    rc = get_standard_heaps_raw(mgr_p, endpointId, coreId, rx_buffer, 16);
    if (rc != awemgr_RC_OK) {
        return rc;
    }

    info_buffer_p->fast_a.nr_free = rx_buffer[0];
    info_buffer_p->fast_a.size = rx_buffer[3];
    info_buffer_p->fast_b.nr_free = rx_buffer[1];
    info_buffer_p->fast_b.size = rx_buffer[4];
    info_buffer_p->slow.nr_free = rx_buffer[2];
    info_buffer_p->slow.size = rx_buffer[5];

    if (info_buffer_p->nr_heaps == 2 || info_buffer_p->nr_heaps == 4)
    {
        rc = get_shared_heap(mgr_p, endpointId, coreId, &info_buffer_p->shared);
    }
    return rc;
}
