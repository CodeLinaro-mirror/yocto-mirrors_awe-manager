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
 * PRIVATE FUNCTIONS
 * ***************************************************************************/


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

enum awemgr_rc  awemgr_get_layout_info(struct awemgr_data* mgr_p, int endpointId, int coreId, awemgr_layoutinfo* info_buffer_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info_buffer_p);
    AWEMGR_FAIL_ON_ENDPOINT_INVALID(endpointId);
    AWEMGR_FAIL_ON_CORE_INVALID(coreId);

    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));

    awecmd_getAllProfilingData(buf_p, endpointId, coreId, 0, MAX_AWEMGR_LAYOUTS);

    // the response is encoded into rx_buffer (0: AverageCycles, 1: TimePerProc )
    unsigned int data_buffer[6*16+4];  // 16max threads * 6 values + 4 overhead
    unsigned int nr_words_in_buffer;

    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, data_buffer, sizeof(data_buffer) / sizeof(data_buffer[0]), &nr_words_in_buffer);

    (void) awecomm_release_lock(mgr_p->comm_2_awe);

    if (rc == awemgr_RC_OK)
    {
        // AWECore protocol is not very consistent, as it does
        // not return the number of layouts directly in the payload,
        // so we have to calculate it from the number of returned words
        //
        // -3 for the 3 overhead words, /6 for the 6 values per layout
        // theorectically we could also get profilingValuesPerLayout later to "adjust" nr_layouts, but before doing
        // that we should rather fix the AWECore protocol to be more consistent
        info_buffer_p->nr_layouts = (nr_words_in_buffer - 3) / 6;

        unsigned int *dp = data_buffer;
        info_buffer_p->averageCyclesAllCombined = *dp++;
        info_buffer_p->overflowCountAllLayouts = *dp++;
        info_buffer_p->profilingValuesPerLayout = *dp++;
        for (unsigned int idx = 0; idx < info_buffer_p->nr_layouts; idx++)
        {
            info_buffer_p->layouts[idx].timePerProcess = *dp++;
            info_buffer_p->layouts[idx].timePerProcessExpected = *dp++;
            info_buffer_p->layouts[idx].averageCycles = *dp++;
            info_buffer_p->layouts[idx].instCycles = *dp++;
            info_buffer_p->layouts[idx].peakCycles = *dp++;
            info_buffer_p->layouts[idx].overflowCount = *dp++;
        }
        AWEMGR_API_LOGD("LayoutInfo for %d designs obtained", info_buffer_p->nr_layouts);
    }

    return rc;
}
