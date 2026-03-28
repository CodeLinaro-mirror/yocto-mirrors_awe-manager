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

#include "cmds_base.h"

#include "app_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "awe_ctrl.h"    // for setting configuration to AWECore

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// ******************************************************************************************************

static void targetinfo_software(idbg_t *p, struct awemgr_data *mgr_p)
{
    awemgr_targetinfo info_buffer;
    awemgr_rc rc = awemgr_get_target_info(mgr_p, &info_buffer);
    if(rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "target_info:\n", info_buffer.nr_awe_instances);
        for (unsigned int idx = 0; idx < info_buffer.nr_awe_instances; idx++)
        {
            idbg_print(IDBG_HDL_VAR, "  - name: %s\n    sw_version: %s\n",
                        info_buffer.instance[idx].targetName,
                        info_buffer.instance[idx].version_long);
        }
    }
    else
    {
        idbg_print(IDBG_HDL_VAR, "Failed to get target info\n");
    }
}

static void target_info_classlist(idbg_t *p, struct awemgr_data *mgr_p, int endpointId, int coreId)
{
    awemgr_modulelist_info classinfo;
    enum awemgr_rc  rc = awemgr_get_modulelist_info(mgr_p, endpointId, coreId, &classinfo);
    if (rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "nr_classes: %d\n", classinfo.nr_classes);
        if (classinfo.nr_classes) {
            idbg_print(IDBG_HDL_VAR, "classes:\n");
        }
        for (int classIdx = 0; classIdx < classinfo.nr_classes; classIdx++)
        {
            idbg_print(IDBG_HDL_VAR, " - 0x%04X\n", classinfo.classes[classIdx].classId);
        }
    }
}

static void target_info_heaps(idbg_t *p, struct awemgr_data *mgr_p, int endpointId)
{
    awemgr_heapinfo heapinfo;
    enum awemgr_rc  rc = awemgr_get_heap_info(mgr_p, endpointId, &heapinfo);

    if (rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "heaps used: %d\n", heapinfo.nr_heaps);
        idbg_print(IDBG_HDL_VAR, " - fast_a: %d/%d (%.2f%%)\n", awemgr_heapinfo_allocated(heapinfo.fast_a), heapinfo.fast_a.size, awemgr_heapinfo_allocated_percent(heapinfo.fast_a));
        idbg_print(IDBG_HDL_VAR, " - fast_b: %d/%d (%.2f%%)\n", awemgr_heapinfo_allocated(heapinfo.fast_b), heapinfo.fast_b.size, awemgr_heapinfo_allocated_percent(heapinfo.fast_b));
        idbg_print(IDBG_HDL_VAR, " - slow  : %d/%d (%.2f%%)\n", awemgr_heapinfo_allocated(heapinfo.slow), heapinfo.slow.size, awemgr_heapinfo_allocated_percent(heapinfo.slow));
        idbg_print(IDBG_HDL_VAR, " - shared: %d/%d (%.2f%%)\n", awemgr_heapinfo_allocated(heapinfo.shared), heapinfo.shared.size, awemgr_heapinfo_allocated_percent(heapinfo.shared));
    }
}

static void target_info_cpuload(idbg_t *p, struct awemgr_data *mgr_p, int endpointId, int coreId)
{
    awemgr_cpuinfo  cpuinfo;
    enum awemgr_rc  rc = awemgr_get_cpu_info(mgr_p, endpointId, coreId, &cpuinfo);
    if (rc == awemgr_RC_OK)
    {
        if (cpuinfo.AverageCycles == 0 && cpuinfo.TimePerProcess == 0)
        {
            idbg_print(IDBG_HDL_VAR, "CPU -/-\n");
        }
        else
        {
            float cpu_load = awemgr_getCpuPercentage(&cpuinfo);
            idbg_print(IDBG_HDL_VAR, "CPU %.2f%%  %s\n", cpu_load, (cpuinfo.AverageCycles >= cpuinfo.TimePerProcess) ? "!OVERLOAD!": "");
        }
    }
}

static void show_target_info_command_usage(idbg_t *p)
{
    IDBG_CMDUSAGE ((p, "[-classes][-cpu][-mem] [-endpoint <index>][-core <index>]",
                    "-classes", "Prints all AWE module class IDs available on the target",
                    "-cpu", "Shows the load on the CPUs",
                    "-mem", "Gets the information about the (heap) memories consumed",
                    "-endpoint <index>", "Endpoint/Index of the AWC",
                    "-core <index>", "In a multi-core system, index of the CPU",
                    NULL, NULL));
}

// ******************************************************************************************************

int target_info(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    bool info_classes = IDBG_CHK_FLAG("-classes");
    bool info_cpuload = IDBG_CHK_FLAG("-cpu");
    bool info_mem = IDBG_CHK_FLAG("-mem");
    int  endpoint = IDBG_GET_INT("-endpoint", -1, ARG_OPTIONAL);
    int  coreId = IDBG_GET_INT("-core", 0, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        show_target_info_command_usage(IDBG_HDL_VAR);
        return IDBG_OK;
    }

    if (!info_classes && !info_cpuload && !info_mem)
    {
        targetinfo_software(IDBG_HDL_VAR, appCtx_p->mgr_p);
        return IDBG_OK;
    }

    endpoint = (endpoint >= 0) ? endpoint : appCtx_p->endpointId;

    if (info_classes) {
        target_info_classlist(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint, coreId);
    }

    if (info_mem) {
        target_info_heaps(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint);
    }

    if (info_cpuload) {
        target_info_cpuload(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint, coreId);
    }

    return IDBG_OK;
}
