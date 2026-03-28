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

#include "cmds_showinfo.h"

#include "app_ctx.h"     // own application related types and interactive cmdline

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void show_user_data_item(idbg_t *p, const char *label, awemgr_userdata data)
{
    const char *type = awemgr_userdata_type_to_string(data.type);
    idbg_print(p, "\n  - name: %s\n", label);
    idbg_print(p, "    key: %s\n", data.key);
    idbg_print(p, "    type: %s\n", type);
    switch (data.type) {
        case AWEMGR_USRDATA_INT:
            idbg_print(p, "    value: %d\n", data.value.i32);
            break;
        case AWEMGR_USRDATA_UINT:
            idbg_print(p, "    value: %d\n", data.value.u32);
            break;
        case AWEMGR_USRDATA_STR:
            idbg_print(p, "    value: %s\n", data.value.str);
            break;
        case AWEMGR_USRDATA_FLOAT:
            idbg_print(p, "    value: %g\n", data.value.f32);
            break;
        default: idbg_print(p, "error: unknown data type\n"); break;
    }
}


void show_available_designs(idbg_t *p, struct awemgr_ctx *ctx_p)
{
    int nr_designs = awemgr_get_design_count(ctx_p);
    if (nr_designs == 0)
    {
        idbg_print(IDBG_HDL_VAR, "designs: []\n");
        return;
    }

    idbg_print(IDBG_HDL_VAR, "designs:\n");
    for (int x = 0; x < nr_designs; x++)
    {
        struct awemgr_design_info info;
        enum awemgr_rc rc = awemgr_get_design_info(ctx_p, x, &info);
        if (rc == awemgr_RC_OK)
        {
            idbg_print(IDBG_HDL_VAR, "\n  - name: %s\n", info.name);
            idbg_print(IDBG_HDL_VAR, "    size_in_bytes: %d\n", info.size);
            if (info.coreid_objectid != 0)
            {
                idbg_print(IDBG_HDL_VAR, "    core_id: %d\n", (info.coreid_objectid >> 16) & 0xff);
                idbg_print(IDBG_HDL_VAR, "    objectId: %d\n", info.coreid_objectid & 0xffff);
            }
        }
    }
}


void show_available_modules(idbg_t *p, struct awemgr_ctx *ctx_p)
{
    int nr_modules = awemgr_get_modules_count(ctx_p);
    if (nr_modules == 0)
    {
        idbg_print(IDBG_HDL_VAR, "modules: []\n");
        return;
    }
    idbg_print(IDBG_HDL_VAR, "modules:\n");
    for (int x = 0; x < nr_modules; x++)
    {
        struct awemgr_module mod_info;
        enum awemgr_rc rc = awemgr_get_module_by_index(ctx_p, x, &mod_info);
        if (rc == awemgr_RC_OK)
        {
            idbg_print(IDBG_HDL_VAR, "\n  - name: %s\n", mod_info.name);
            idbg_print(IDBG_HDL_VAR, "    obj_id: %d\n", mod_info.objectId);
            idbg_print(IDBG_HDL_VAR, "    class_id: 0x%08X\n", mod_info.classId);
            if (mod_info.alias)
                idbg_print(IDBG_HDL_VAR, "    alias: %s\n", mod_info.alias);
        }
    }
}


void show_available_event_modules(idbg_t *p, struct awemgr_ctx *ctx_p)
{
    int nr_event_modules = awemgr_get_event_count(ctx_p);  // error (-1) does not need to be handled; ctx_p is good!
    if (nr_event_modules == 0)
    {
        idbg_print(IDBG_HDL_VAR, "event_modules: []\n");
        return;
    }
    idbg_print(IDBG_HDL_VAR, "event_modules:\n");
    for (int x = 0; x < nr_event_modules; x++)
    {
        struct awemgr_module mod_info;
        enum awemgr_rc rc = awemgr_get_event_by_index(ctx_p, x, &mod_info);
        if (rc == awemgr_RC_OK)
        {
            idbg_print(IDBG_HDL_VAR, "\n  - name: %s\n", mod_info.name, mod_info.objectId, mod_info.classId);
            idbg_print(IDBG_HDL_VAR, "    object_id: %d\n", mod_info.objectId);
            idbg_print(IDBG_HDL_VAR, "    class_id: 0x%08X\n", mod_info.classId);
        }
    }
}


void show_available_controls(idbg_t *p, struct awemgr_ctx *ctx_p)
{
    int nr_ctls = awemgr_get_controls_count(ctx_p);
    if (nr_ctls == 0)
    {
        idbg_print(IDBG_HDL_VAR, "controls: []\n");
        return;
    }
    for (int idx = 0; idx < nr_ctls; idx++) {

        struct awemgr_ctl_elem_info info;
        enum awemgr_rc rc = awemgr_get_control_info(ctx_p, idx, &info);

        if (rc == awemgr_RC_OK) {

            idbg_print(IDBG_HDL_VAR, "\n  - name: %s\n", info.id.name);
            idbg_print(IDBG_HDL_VAR, "    size: %d\n", info.id.nr_items);
            idbg_print(IDBG_HDL_VAR, "    type: %s\n", awemgr_vartype_to_string(info.id.type));
            if (info.id.alias)
                idbg_print(IDBG_HDL_VAR, "    alias: %s\n", info.id.alias);

            if(info.range_enabled)
            {
                idbg_print(IDBG_HDL_VAR, "    range:\n");
                switch(info.id.type)
                {
                    case AWEMGR_VARTYPE_UNSIGNED_INTEGER:
                            idbg_print(IDBG_HDL_VAR, "      min: %u\n", info.range.u32.min);
                            idbg_print(IDBG_HDL_VAR, "      max: %u\n", info.range.u32.max);
                            idbg_print(IDBG_HDL_VAR, "      step: %u\n", info.range.u32.step);
                        break;

                    case AWEMGR_VARTYPE_INTEGER:
                            idbg_print(IDBG_HDL_VAR, "      min: %d\n", info.range.i32.min);
                            idbg_print(IDBG_HDL_VAR, "      max: %d\n", info.range.i32.max);
                            idbg_print(IDBG_HDL_VAR, "      step: %d\n", info.range.i32.step);
                        break;

                    case AWEMGR_VARTYPE_FRACT32:
                    case AWEMGR_VARTYPE_FRACT16:
                    case AWEMGR_VARTYPE_FLOAT:
                        idbg_print(IDBG_HDL_VAR, "      min: %.6g\n", info.range.f32.min);
                        idbg_print(IDBG_HDL_VAR, "      max: %.6g\n", info.range.f32.max);
                        idbg_print(IDBG_HDL_VAR, "      step: %.6g\n", info.range.f32.step);
                        break;

                    default:
                        // already checked in AWEManager
                        break;
                }
            }
        }
    }
}


void show_available_userdata(idbg_t *p, struct awemgr_ctx *ctx_p, const char* specific_key)
{
    const char *currentParent = NULL;

    int nr_ctls = awemgr_get_controls_count(ctx_p);
    int nr_userdata_count = 0;

    idbg_print(IDBG_HDL_VAR, "userdata:\n");  // note: no newline yet! and also no []
    for (int idx = 0; idx < nr_ctls; idx++) {

        struct awemgr_ctl_elem_info info;
        enum awemgr_rc rc = awemgr_get_control_info(ctx_p, idx, &info);
        if (rc == awemgr_RC_OK) {
            awemgr_userdata data;

            // if this control comes from a new (different) module then print info about its user data
            // skip printing info when key is given and does not match
            if ((currentParent == NULL) || currentParent != info.id.parentName) {

                currentParent = info.id.parentName;

                int nr_items = awemgr_get_module_userdata_count(ctx_p, currentParent);

                for (int ud_idx = 0; ud_idx < nr_items; ud_idx++) {
                    rc = awemgr_get_module_userdata_by_index(ctx_p, currentParent, ud_idx, &data);
                    if (rc == awemgr_RC_OK) {
                        if (specific_key && (strncmp(specific_key, data.key, strlen(data.key)) != 0))
                            continue;
                        if (nr_userdata_count++ == 0)
                            idbg_print(IDBG_HDL_VAR, "\n");
                        show_user_data_item(p, currentParent, data);
                    }
                }

            }

            // now show the user data info from the control itself
            int nr_items = awemgr_get_control_userdata_count(ctx_p, info.id.name);

            for (int ud_idx = 0; ud_idx < nr_items; ud_idx++) {
                rc = awemgr_get_control_userdata_by_index(ctx_p, info.id.name, ud_idx, &data);
                if (rc == awemgr_RC_OK) {
                    if (specific_key && (strncmp(specific_key, data.key, strlen(data.key)) != 0))
                        continue;
                    if (nr_userdata_count++ == 0)
                        idbg_print(IDBG_HDL_VAR, "\n");
                    show_user_data_item(p, info.id.name, data);
                }
            }
        }
    }
    if (nr_userdata_count == 0)
        idbg_print(IDBG_HDL_VAR, " []\n");

}


static void show_info_command_usage(idbg_t *p)
{
    IDBG_CMDUSAGE ((p, "[-designs][-modules][-events][-controls][-userdata][-all]",
                    "-designs", "Lists all designs (AWB) - use with 'load_design' command",
                    "-modules", "Lists all modules (incl. event modules) - use with 'module' command",
                    "-events", "All event modules - use with 'event' command",
                    "-controls", "All controllable variables in './controls' or use with 'set/get_value' commands",
                    "-userdata", "Lists all user defined attributes of modules and controls",
                    "-all", "Show all of the above (in that order!)",
                    NULL, NULL));
}


int show_info_command(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    bool show_designs = IDBG_CHK_FLAG("-designs");
    bool show_modules = IDBG_CHK_FLAG("-modules");
    bool show_controls = IDBG_CHK_FLAG("-controls");
    bool show_events = IDBG_CHK_FLAG("-events");
    bool show_userdata = IDBG_CHK_FLAG("-userdata");

    bool show_all = IDBG_CHK_FLAG("-all");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        show_info_command_usage(IDBG_HDL_VAR);
        return IDBG_OK;
    }

    if (!show_designs && !show_modules && !show_controls && !show_events && !show_userdata && !show_all)
    {
        show_info_command_usage(IDBG_HDL_VAR);
        return IDBG_OK;
    }

    if (!awc_ctx_p) {
        idbg_print(p, "No AWC loaded. Nothing to show!\n");
        return IDBG_OK;
    }
    if (show_all)
        show_designs = show_modules = show_controls = show_events = show_userdata = true;


    if (show_designs)
        show_available_designs(p, awc_ctx_p);

    if (show_modules)
        show_available_modules(p, awc_ctx_p);

    if (show_events)
        show_available_event_modules(p, awc_ctx_p);

    if (show_controls)
        show_available_controls(p, awc_ctx_p);

    if (show_userdata)
        show_available_userdata(p, awc_ctx_p, NULL);

    return IDBG_OK;
}