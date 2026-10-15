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

#include "cmds_base.h"

#include "shell_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "awe_comm.h"    // for setting configuration to AWECore
#include "hlp_functions.h" // for helper functions like IDBG_PRINT_ERR

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// ******************************************************************************************************

static void targetinfo_software(idbg_t *p, struct awemgr_data *mgr_p, bool extended)
{
    awemgr_targetinfo info_buffer;
    awemgr_rc rc = awemgr_get_target_info(mgr_p, &info_buffer);
    if(rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "target_info:\n", info_buffer.nr_awe_instances);
        for (unsigned int idx = 0; idx < info_buffer.nr_awe_instances; idx++)
        {
            idbg_print(IDBG_HDL_VAR,
                "  - name: %s\n"
                "    awecore_version: %s\n"
                ,
                info_buffer.instance[idx].targetName,
                info_buffer.instance[idx].version_long
            );
            if (info_buffer.instance[idx].build_nr != 0)
            {
                idbg_print(IDBG_HDL_VAR,
                    "    build_nr: %d\n"
                    ,
                    info_buffer.instance[idx].build_nr
                );
            }
            else
            {
                idbg_print(IDBG_HDL_VAR,
                    "    build_nr: -not set-\n"
                );
            }
            idbg_print(IDBG_HDL_VAR,
                "    proc_type: %s\n"
                "    instance_id: %d\n"
                "    block_size: %d\n"
                "    sample_rate: %.2f\n"
                ,
                info_buffer.instance[idx].proc_type,
                info_buffer.instance[idx].instance_id,
                info_buffer.instance[idx].block_size,
                info_buffer.instance[idx].m_sampleRate
            );
            if (extended)
            {
                idbg_print(IDBG_HDL_VAR,
                    "    extended:\n"
                    "      user_version: %d\n"
                    "      hotfix_version: %d\n"
                    "      nr_threads: %d\n"
                    "      nr_cores: %d\n"
                    "      commbuffer_size: %d\n"
                    "      alignment_size: %d\n"
                    "      supports_module_reset: %s\n"
                    "      supports_fract16: %s\n"
                    ,
                    info_buffer.instance[idx].user_version,
                    info_buffer.instance[idx].hotfix_version,
                    info_buffer.instance[idx].nr_threads,
                    info_buffer.instance[idx].nr_cores,
                    info_buffer.instance[idx].commbuffer_size,
                    info_buffer.instance[idx].alignment_size,
                    info_buffer.instance[idx].supports_module_reset ? "true" : "false",
                    info_buffer.instance[idx].supports_fract16 ? "true" : "false"
                );
                idbg_print(IDBG_HDL_VAR,
                    "    pins:\n"
                    "      nr_in: %d\n"
                    "      nr_out: %d\n"
                    ,
                    info_buffer.instance[idx].nr_chan_in,
                    info_buffer.instance[idx].nr_chan_out
                );
            }
        }
    }
    else
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "failed to get target info\n");
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
        for (unsigned int classIdx = 0; classIdx < classinfo.nr_classes; classIdx++)
        {
            idbg_print(IDBG_HDL_VAR, " - 0x%04X\n", classinfo.classes[classIdx].classId);
        }
    }
    else
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "failed to get class list info\n");
    }
}

static void target_info_heaps(idbg_t *p, struct awemgr_data *mgr_p, int endpointId, int coreId)
{
    awemgr_heapinfo heapinfo;
    enum awemgr_rc  rc = awemgr_get_heap_info(mgr_p, endpointId, coreId, &heapinfo);

    if (rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "heaps:\n");
        idbg_print(IDBG_HDL_VAR, " - fast_a:\n    used:%d\n    total:%d\n    percent:%.2f%%\n", awemgr_heapinfo_allocated(heapinfo.fast_a), heapinfo.fast_a.size, awemgr_heapinfo_allocated_percent(heapinfo.fast_a));
        idbg_print(IDBG_HDL_VAR, " - fast_b:\n    used:%d\n    total:%d\n    percent:%.2f%%\n", awemgr_heapinfo_allocated(heapinfo.fast_b), heapinfo.fast_b.size, awemgr_heapinfo_allocated_percent(heapinfo.fast_b));
        idbg_print(IDBG_HDL_VAR, " - slow  :\n    used:%d\n    total:%d\n    percent:%.2f%%\n", awemgr_heapinfo_allocated(heapinfo.slow), heapinfo.slow.size, awemgr_heapinfo_allocated_percent(heapinfo.slow));
        idbg_print(IDBG_HDL_VAR, " - shared:\n    used:%d\n    total:%d\n    percent:%.2f%%\n", awemgr_heapinfo_allocated(heapinfo.shared), heapinfo.shared.size, awemgr_heapinfo_allocated_percent(heapinfo.shared));
    }
    else
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "failed to get heap info\n");
    }
}

static void target_info_cpuload_all(idbg_t *p, struct awemgr_data *mgr_p, int endpointId)
{
    awemgr_targetinfo info_buffer;
    awemgr_rc rc = awemgr_get_target_info(mgr_p, &info_buffer);
    if (rc != awemgr_RC_OK)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "failed to get target info\n");
        return;
    }

    idbg_print(IDBG_HDL_VAR, "cpu_info:\n");
    for (unsigned int idx = 0; idx < info_buffer.nr_awe_instances; idx++)
    {
        int coreId = (int)info_buffer.instance[idx].instance_id;
        idbg_print(IDBG_HDL_VAR, "  - name: %s\n", info_buffer.instance[idx].targetName);

        awemgr_cpuinfo cpuinfo;
        if (awemgr_get_cpu_info(mgr_p, endpointId, coreId, &cpuinfo) == awemgr_RC_OK
            && (cpuinfo.AverageCycles != 0 || cpuinfo.TimePerProcess != 0))
            idbg_print(IDBG_HDL_VAR, "    cpu: %.2f\n", awemgr_getCpuPercentage(&cpuinfo));
        else
            idbg_print(IDBG_HDL_VAR, "    cpu: ~\n");

        awemgr_layoutinfo layoutinfo;
        if (awemgr_get_layout_info(mgr_p, endpointId, coreId, &layoutinfo) == awemgr_RC_OK
            && layoutinfo.nr_layouts > 0)
        {
            idbg_print(IDBG_HDL_VAR, "    layouts: [");
            for (unsigned int li = 0; li < layoutinfo.nr_layouts; li++)
            {
                awemgr_cpuinfo layout_cpu = {
                    layoutinfo.layouts[li].averageCycles,
                    layoutinfo.layouts[li].timePerProcess
                };
                idbg_print(IDBG_HDL_VAR, " %6.2f,", awemgr_getCpuPercentage(&layout_cpu));
            }
            idbg_print(IDBG_HDL_VAR, " ]\n");
        }
        else
        {
            idbg_print(IDBG_HDL_VAR, "    layouts: []\n");
        }
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
            idbg_print(IDBG_HDL_VAR, "cpu_load_percent: ~\n");
        }
        else
        {
            float cpu_load = awemgr_getCpuPercentage(&cpuinfo);
            idbg_print(IDBG_HDL_VAR, "cpu_load_percent: %.2f\n", cpu_load);
        }
        idbg_print(IDBG_HDL_VAR, "average_cycles: %u\n", cpuinfo.AverageCycles);
        idbg_print(IDBG_HDL_VAR, "time_per_process: %u\n", cpuinfo.TimePerProcess);
        idbg_print(IDBG_HDL_VAR, "overload: %s\n", (cpuinfo.AverageCycles >= cpuinfo.TimePerProcess) ? "true" : "false");
    }
    else
    {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Failed to get CPU info\n");
    }
}

static void target_info_layout(idbg_t *p, struct awemgr_data *mgr_p, int endpointId, int coreId)
{
    awemgr_layoutinfo layoutinfo;
    enum awemgr_rc  rc = awemgr_get_layout_info(mgr_p, endpointId, coreId, &layoutinfo);
    if (rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "layoutinfo:\n");
        idbg_print(IDBG_HDL_VAR, "  nr_layouts: %d\n", layoutinfo.nr_layouts);
        idbg_print(IDBG_HDL_VAR, "  averageCyclesAllCombined: %d\n", layoutinfo.averageCyclesAllCombined);
        idbg_print(IDBG_HDL_VAR, "  overflowCountAllLayouts: %d\n", layoutinfo.overflowCountAllLayouts);
        idbg_print(IDBG_HDL_VAR, "  profilingValuesPerLayout: %d\n", layoutinfo.profilingValuesPerLayout);
        idbg_print(IDBG_HDL_VAR, "  layouts:\n");
        for (unsigned int idx = 0; idx < layoutinfo.nr_layouts; idx++)
        {
            awemgr_cpuinfo cpuinfo_for_layout = {
                layoutinfo.layouts[idx].averageCycles,
                layoutinfo.layouts[idx].timePerProcess
            };
            idbg_print(IDBG_HDL_VAR, "    - cpu_load_percent: %.2f\n", awemgr_getCpuPercentage(&cpuinfo_for_layout));
            idbg_print(IDBG_HDL_VAR, "      timePerProcess: %d\n", layoutinfo.layouts[idx].timePerProcess);
            idbg_print(IDBG_HDL_VAR, "      timePerProcessExpected: %d\n", layoutinfo.layouts[idx].timePerProcessExpected);
            idbg_print(IDBG_HDL_VAR, "      averageCycles: %d\n", layoutinfo.layouts[idx].averageCycles);
            idbg_print(IDBG_HDL_VAR, "      instCycles: %d\n", layoutinfo.layouts[idx].instCycles);
            idbg_print(IDBG_HDL_VAR, "      peakCycles: %d\n", layoutinfo.layouts[idx].peakCycles);
            idbg_print(IDBG_HDL_VAR, "      overflowCount: %d\n", layoutinfo.layouts[idx].overflowCount);
        }
    }
    else
    {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Failed to get layout info.\n");
    }
}


static void show_target_info_command_usage(idbg_t *p)
{
    IDBG_CMDUSAGE ((p, "[-classes][-cpu][-mem] [-endpoint <index>][-core <index>]\n      When used without classes/cpu/mem/layout, general target information is printed.",
                    "-classes", "Prints all AWE module class IDs available on the target",
                    "-cpu", "Shows the load on the CPUs",
                    "-mem", "Gets the information about the (heap) memories consumed",
                    "-layout", "Shows detailed profiling information for every layout/thread",
                    "-endpoint <index>", "Endpoint/Index of the AWC",
                    "-core <index>", "In a multi-core system, index of the CPU",
                    "-extended", "Show extended target information",
                    NULL, NULL));
}

// ******************************************************************************************************

int target_info(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    /* Collect all output of this command and emit it as one block: the sub
     * commands below issue several API calls, and with comm tracing enabled
     * the trace output would otherwise be interleaved with - and thereby
     * break - the YAML document printed here. */
    IdbgOutputHold output_hold(IDBG_HDL_VAR);

    bool info_classes = IDBG_CHK_FLAG("-classes");
    bool info_cpuload = IDBG_CHK_FLAG("-cpu");
    bool info_mem = IDBG_CHK_FLAG("-mem");
    bool info_layout = IDBG_CHK_FLAG("-layout");
    int  endpoint = IDBG_GET_INT("-endpoint", -1, ARG_OPTIONAL);
    int  coreId   = IDBG_GET_INT("-core",     -1, ARG_OPTIONAL);
    bool extended = IDBG_CHK_FLAG("-extended");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        show_target_info_command_usage(IDBG_HDL_VAR);
        return IDBG_OK;
    }

    if (!info_classes && !info_cpuload && !info_mem && !info_layout)
    {
        targetinfo_software(IDBG_HDL_VAR, appCtx_p->mgr_p, extended);
        return IDBG_OK;
    }

    bool explicit_target = (endpoint >= 0 || coreId >= 0);
    endpoint = (endpoint >= 0) ? endpoint : appCtx_p->endpointId;
    int effectiveCoreId = (coreId >= 0) ? coreId : 0;

    /* Without a loaded AWC the session has no endpoint selected (-1); the
     * queries below must not be issued with such an endpoint index. */
    if (endpoint < 0 || endpoint >= awemgr_get_max_awcs())
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR,
                       "no endpoint selected: load an AWC or pass -endpoint <index> (0 to %d)\n",
                       awemgr_get_max_awcs() - 1);
        return IDBG_OK;
    }

    if (info_classes) {
        target_info_classlist(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint, effectiveCoreId);
    }

    if (info_mem) {
        target_info_heaps(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint, effectiveCoreId);
    }

    if (info_cpuload) {
        if (!explicit_target)
            target_info_cpuload_all(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint);
        else
            target_info_cpuload(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint, effectiveCoreId);
    }

    if (info_layout) {
        target_info_layout(IDBG_HDL_VAR, appCtx_p->mgr_p, endpoint, effectiveCoreId);
    }
    return IDBG_OK;
}
