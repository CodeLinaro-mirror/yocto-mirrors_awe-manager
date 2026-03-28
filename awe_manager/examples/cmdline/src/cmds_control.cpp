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
#include "awemgr_logging.h"  // for dumping buffer content
#include "hlp_dump_response.h"  // dump response
#include "awemgr_util.h"    // for float to fract functions
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ******************************************************************************************************

static int awemgr_type_to_logtype(enum awemgr_vartype typ)
{
    switch (typ) {
        case AWEMGR_VARTYPE_FLOAT:
            return AWEMGR_LOG_VARTYPE_FLOAT;
        case AWEMGR_VARTYPE_FRACT32:
            return AWEMGR_LOG_VARTYPE_FRACT32;
        case AWEMGR_VARTYPE_FRACT16:
            return AWEMGR_LOG_VARTYPE_FRACT16;
        default:
            return AWEMGR_LOG_VARTYPE_INT;
    }
    return AWEMGR_LOG_VARTYPE_INT;
}

static unsigned int *allocate_local_buffer(struct awemgr_ctx *awc_ctx_p, char* var_name, unsigned int *nr_items_p)
{
    struct awemgr_ctl_elem_info info;
    enum awemgr_rc rc = awemgr_get_control_info_by_name(awc_ctx_p, var_name, &info);
    if (rc != awemgr_RC_OK)
    {
        fprintf (stderr, "Could not get information for %s\n", var_name);
        *nr_items_p = 0;
        return NULL;
    }

    int data_buffer_sz = info.id.nr_items;
    unsigned int *data_buffer = (unsigned int*) calloc(data_buffer_sz, sizeof(unsigned int));

    if (!data_buffer)
        fprintf (stderr, "Local buffer allocation failed or nr of items is 0.!!!\n");

    // todo: handle error cases better

    *nr_items_p= data_buffer_sz;
    return data_buffer;
}


int ctrl_generic_get (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    char *var_name = IDBG_GET_STRING("-var", NULL, ARG_NEEDED);  // todo: subst 0 with real default value! does that work?
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-var <varname>",
                        "-var <varname>", "var name as in AWC file",
        				NULL, NULL));
        return IDBG_OK;
    }

    // first get the information how big the expected data will be, and allocate enough memory
    unsigned int nr_items;
    unsigned int *response_buffer = allocate_local_buffer(awc_ctx_p, var_name, &nr_items);
    if (response_buffer == NULL) {
        idbg_print(p, "error: Could not allocate memory\n");
        return IDBG_OK;
    }

    // now read into local memory
    unsigned int nr_words_in_buf;
    enum awemgr_vartype typ;
    enum awemgr_rc rc = awemgr_control_read(awc_ctx_p, var_name, response_buffer, nr_items, &nr_words_in_buf, &typ);

    if (rc == awemgr_RC_OK) {
        idbg_print(IDBG_HDL_VAR, "variable:\n");
        idbg_print(IDBG_HDL_VAR, "  name: %s\n", var_name);
        idbg_print(IDBG_HDL_VAR, "  data: [\n");
        dump_response(IDBG_HDL_VAR, "    ", response_buffer, nr_words_in_buf * sizeof(unsigned int), typ, false);
        idbg_print(IDBG_HDL_VAR, "  ]\n");
    }
    else
    {
        idbg_print(IDBG_HDL_VAR, "ctrl_generic_get command failed\n");
    }
    free(response_buffer);

    return IDBG_OK;
}

int ctrl_generic_set (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    char *var_name = IDBG_GET_STRING("-var", NULL, ARG_NEEDED);  // todo: subst 0 with real default value! does that work?
    int offset = IDBG_GET_INT("-offset", 0, ARG_OPTIONAL);
    char *values = IDBG_GET_STRING("-values", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-var <varname> [-offset <offset>] -values '<val1> <val2> ...' ",
                        "-var <varname>", "var name as in AWC file",
                        "-offset <offset>", "Offset into variable's array. default: 0",
                        "-values '<val1> <val2> ...' ", "space separated list of values (in a string!); integer (eg 1234), hex (0xdead) or float (eg 10.5) supported",
                        NULL, NULL));
        return IDBG_OK;
    }

    // get information about variable (type) first
    struct awemgr_ctl_elem_info ctrl_info;
    enum awemgr_rc rc = awemgr_get_control_info_by_name(awc_ctx_p, var_name, &ctrl_info);
    if (rc != awemgr_RC_OK)
    {
        idbg_print(p, "error: Could not get information for %s\n", var_name);
        return IDBG_OK;
    }

    unsigned int nr_items_max;
    unsigned int *data_buffer = allocate_local_buffer(awc_ctx_p, var_name, &nr_items_max);
    if (data_buffer == NULL) {
        idbg_print(p, "error: Could not allocate memory\n");
        return IDBG_OK;
    }
    unsigned int *dp = data_buffer;
    unsigned int tmp;

    char *token = strtok(values, " ");
    while (token != NULL)
    {
        if (ctrl_info.id.type == AWEMGR_VARTYPE_FLOAT)
        {
            float v = strtof(token, NULL);

            memcpy(&tmp, &v, sizeof tmp);
            *dp++ = tmp;
        }
        else if (ctrl_info.id.type == AWEMGR_VARTYPE_FRACT32)
        {
            float v = strtof(token, NULL);
            int32_t vfract32 = float_to_fract32(v);

            memcpy(&tmp, &vfract32, sizeof tmp);
            *dp++ = tmp;
        }
        else if (ctrl_info.id.type == AWEMGR_VARTYPE_FRACT16)
        {
            float v = strtof(token, NULL);
            int16_t vfract16 = float_to_fract16(v);
            int32_t extended = (int32_t)vfract16; // to 32 bit

            memcpy(&tmp, &extended, sizeof tmp);
            *dp++ = tmp;
        }
        else
        {
            char *endptr;
            *dp++ = (unsigned int) strtol(token, &endptr, 0);
        }
        token = strtok(NULL, " ");
    }

    unsigned int nr_values = (unsigned int)(dp - data_buffer);
    if (nr_values > nr_items_max)
    {
        idbg_print(p, "error: Used too many values for variable %s: max: %d\n", var_name, ctrl_info.id.nr_items);
    }
    else
    {
        enum awemgr_rc rc = awemgr_control_write(awc_ctx_p, var_name, offset, (void*) data_buffer, nr_values);
        if (rc != awemgr_RC_OK)
        {
            idbg_print(p, "error: Could not write data to %s\n", var_name);
        }
    }
    free(data_buffer);
    return IDBG_OK;
}

int ctrl_transact (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *values = IDBG_GET_STRING("-values", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-values '<val1> <val2> ...' ",
                        "-values '<val1> <val2> ...' ", "space separated list of values (in a string!); integer (eg 1234), hex (0xdead) or float (eg 10.5) supported",
                        NULL, NULL));
        return IDBG_OK;
    }

    unsigned int request_buffer[264];  // todo: fix this hardcoded stuff
    unsigned int *dp = request_buffer;
    unsigned int response_buffer[264];

    char *token = strtok(values, " ");
    while (token != NULL)
    {
        char *endptr;
        *dp++ = (unsigned int) strtol(token, &endptr, 0);
        token = strtok(NULL, " ");
    }

    unsigned int nr_values = (unsigned int)(dp - request_buffer);
    enum awemgr_rc rc = awemgr_transact(appCtx_p->mgr_p, request_buffer, nr_values, response_buffer, 264);
    if (rc == awemgr_RC_OK)
    {
        unsigned int ret = (unsigned int)response_buffer[1];
        if (ret != 0)
        {
            idbg_print(p, "ResponseBuffer contains error! errcode=%d\n", ret);
        }
        else
        {
            int sz = response_buffer[0] >> 16;
            dump_response(p, "RX: ", response_buffer, sz * sizeof(unsigned int), AWEMGR_VARTYPE_INTEGER, false);
        }
    }
    else
    {
        idbg_print(p, "ctrl_transact command failed\n");
    }

    return IDBG_OK;
}

struct dumpctx
{
    idbg_t *p;
    char const *prefix;
};

static void my_cb(char *line, int index, void *ctx)
{
    struct dumpctx *d = (struct dumpctx*) ctx;
    idbg_print(d->p, "  => %s %4d : %s\n", d->prefix, index, line);
}


static int ctrl_work_on_var (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    char *var_name = argv[0];
    int offset = IDBG_GET_INT("-offset", 0, ARG_OPTIONAL);
    char *values = IDBG_GET_STRING("-values", NULL, ARG_OPTIONAL);
    bool showInfo = IDBG_CHK_FLAG("-info");
    bool showTx = IDBG_CHK_FLAG("-show_tx");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "[-offset <offset>] [-values '<val1> <val2> ...']",
                        "-offset <offset>", "Offset into variable's array. default: 0",
                        "-values '<val1> <val2> ...' ", "space separated list of values (in a string!); integer (eg 1234), hex (0xdead) or float (eg 10.5) supported",
                        NULL, NULL));
        return IDBG_OK;
    }

    // ************************************************************************
    // showing "meta" information of this variable, like range info, type etc
    if (showInfo)
    {
        char line_output[512];

        struct awemgr_ctl_elem_info info;
        enum awemgr_rc rc = awemgr_get_control_info_by_name(awc_ctx_p, var_name, &info);
        if (rc != awemgr_RC_OK)
        {
            idbg_print(p, "error: Could not get information for %s\n", var_name);
            return IDBG_OK;
        }

        snprintf(line_output, sizeof(line_output), "type: %d ", info.id.type);
        switch(info.id.type)
        {
            case AWEMGR_VARTYPE_UNSIGNED_INTEGER:
                snprintf(line_output, sizeof(line_output), "type: %s, min/max/step: %u/%u/%u\n",
                    awemgr_vartype_to_string(info.id.type),
                    info.range.u32.min, info.range.u32.max, info.range.u32.step);
                break;
            case AWEMGR_VARTYPE_INTEGER:
                snprintf(line_output, sizeof(line_output), "type: %s, min/max/step: %d/%d/%d\n",
                    awemgr_vartype_to_string(info.id.type),
                    info.range.i32.min, info.range.i32.max, info.range.i32.step);
                break;
            case AWEMGR_VARTYPE_FRACT32:
            case AWEMGR_VARTYPE_FLOAT:
                snprintf(line_output, sizeof(line_output), "type: %s, min/max/step: %f/%f/%f\n",
                    awemgr_vartype_to_string(info.id.type),
                    info.range.f32.min, info.range.f32.max, info.range.f32.step);
                break;
            default:
                // no error here as already checked in AWEManager before
                break;
        }

        idbg_print(p, line_output);
        return IDBG_OK;
    }

    if (values)
    {
        // ********************************************************************
        // write access - setting values on the variable

        struct awemgr_ctl_elem_info info;
        enum awemgr_rc rc = awemgr_get_control_info_by_name(awc_ctx_p, var_name, &info);

        unsigned int nr_items_max;
        unsigned int *data_buffer = allocate_local_buffer(awc_ctx_p, var_name, &nr_items_max);
        if (data_buffer == NULL) {
            idbg_print(p, "error: Could not allocate memory\n");
            return IDBG_OK;
        }
        unsigned int *dp = data_buffer;
        unsigned int tmp;

        bool isFloat = (info.id.type == AWEMGR_VARTYPE_FLOAT);

        char *token = strtok(values, " ");
        while (token != NULL)
        {
            if (isFloat)
            {
                float v = strtof(token, NULL);

                memcpy(&tmp, &v, sizeof tmp);
                *dp++ = tmp;
            }
            else
            {
                char *endptr;
                *dp++ = (unsigned int) strtol(token, &endptr, 0);
            }
            token = strtok(NULL, " ");
        }

        unsigned int nr_values = (unsigned int)(dp - data_buffer);
        if ((unsigned int)nr_values > nr_items_max)
        {
            idbg_print(p, "used with too many values for variable %s\n", var_name);
        }
        else
        {
            rc = awemgr_control_write(awc_ctx_p, var_name, offset, (void*) data_buffer, nr_values);
            if (rc != awemgr_RC_OK) {
                idbg_print(p, "Error writing variable %s\n", var_name);
            }
        }
        free(data_buffer);
    }
    else
    {
        // ********************************************************************
        // read access - obtaining values from the variable and print/dump it

        // get info from control to know about the required nr of items (size)
        struct awemgr_ctl_elem_info info;
        enum awemgr_rc rc = awemgr_get_control_info_by_name(awc_ctx_p, var_name, &info);

        // allocate memory to hold the receicved data
        unsigned int nr_items;
        unsigned int *response_buffer = allocate_local_buffer(awc_ctx_p, var_name, &nr_items);
        if (response_buffer == NULL) {
            idbg_print(p, "error: Could not allocate memory\n");
            return IDBG_OK;
        }


        unsigned int nr_words_in_buf;
        enum awemgr_vartype typ;
        rc = awemgr_control_read(awc_ctx_p, var_name, response_buffer, nr_items, &nr_words_in_buf, &typ);

        if (rc == awemgr_RC_OK)
        {
            // dump the buffer
            struct dumpctx c = {p, "RX"};
            awemgr_log_buffer(response_buffer, nr_words_in_buf * sizeof(unsigned int), awemgr_type_to_logtype(typ), my_cb, &c);

            if (showTx)
            {
                // also dumping the information how to transfer this data back to the design; assuming
                // it is the same variable (i.e. the variable is r/w)
                // snprintf(tx_prefix, sizeof(tx_prefix), "TX: %s -values \"", var_name);
                // dump_response(p, tx_prefix, response_buffer, nr_words_in_buf * sizeof(unsigned int), typ, true);
                idbg_print(p, "TX: %s -set >>DSTART", var_name);
                struct dumpctx c = {p, "TX"};
                awemgr_log_buffer(response_buffer, nr_words_in_buf * sizeof(unsigned int), awemgr_type_to_logtype(typ), my_cb, &c);
                idbg_print(p, "TX: <<DEND\n");
            }
        }
        free(response_buffer);
    }

    return IDBG_OK;
}

int ctrl_enumerate (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *dir_name = IDBG_GET_STRING("-dir", "controls", ARG_OPTIONAL);
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "[-dir <dir>]",
                        "-dir <dir>", "Specifies the directory to fill with control names (defaults: controls)",
                        NULL, NULL));
    }

    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    if (awc_ctx_p)
    {
        int nr_ctls = awemgr_get_controls_count(awc_ctx_p);
        if (nr_ctls > 0)
        {
            idbgtableentry_t *new_dir_p;
            // allocate a new directory in the IDbg table;
            // assuming we have lots of aliases : nr_ctls * 2
            int incl_al = 0;
            int rc = idbg_allocate_dir(p, dir_name, nr_ctls * 2, &new_dir_p);
            if (rc == 0)
            {
                struct awemgr_ctl_elem_info ctl_info;
                for (int ctl_idx = 0; ctl_idx < nr_ctls; ctl_idx++)
                {
                    enum awemgr_rc rc = awemgr_get_control_info(awc_ctx_p, ctl_idx, &ctl_info);
                    if (rc != awemgr_RC_OK)
                    {
                        idbg_print(p, "error: Could not get information for control index %d\n", ctl_idx);
                        continue;
                    }
                    new_dir_p[incl_al].pFunc = ctrl_work_on_var;
                    new_dir_p[incl_al].type = IDBG_TBL_TYPE_CMD;
                    new_dir_p[incl_al].pchName = (char*) ctl_info.id.name;
                    incl_al++;

                    if (ctl_info.id.alias != NULL)
                    {
                        new_dir_p[incl_al].pFunc = ctrl_work_on_var;
                        new_dir_p[incl_al].type = IDBG_TBL_TYPE_CMD;
                        new_dir_p[incl_al].pchName = (char*) ctl_info.id.alias;
                        incl_al++;
                    }
                }
            }
            else
            {
                idbg_print(p, "Could not find directory '%s' in current dir. Nothing added to IDbg.\n", dir_name);
            }
        }
        else
        {
            idbg_print(p, "Error enumerating controls. AWE Manager initialized and AWC loaded?\n");
        }
    }

    return IDBG_OK;
}

int module_generic (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    char *mod_name = IDBG_GET_STRING("-name", NULL, ARG_NEEDED);
    char *new_state = IDBG_GET_STRING("-state", NULL, ARG_OPTIONAL);
    bool show_all_module_info = IDBG_CHK_FLAG("-full");
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-name <modname> [-state <state>]",
                        "-name <modname>", "module name as in AWC file",
                        "-state <state>", "new operating state to set: ACTIVE, BYPASS, MUTED, INACTIVE",
                        "-full", "show more info rather than only operating state",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct awemgr_module module_info;
    enum awemgr_rc rc = awemgr_get_module_by_name(awc_ctx_p, mod_name, &module_info);

    if (rc == awemgr_RC_OK)
    {
        if (new_state) {

            enum awemgr_module_runtimestate new_state_v = MODULE_RUNTIME_UNKOWN;
            if (strcmp(new_state, "ACTIVE") == 0)   //strcasecmp ? // stricmp on Windows???
                new_state_v = MODULE_ACTIVE;
            else if (strcmp(new_state, "BYPASS") == 0)
                new_state_v = MODULE_BYPASS;
            else if (strcmp(new_state, "MUTED") == 0)
                new_state_v = MODULE_MUTED;
            else if (strcmp(new_state, "INACTIVE") == 0)
                new_state_v = MODULE_INACTIVE;

            if (new_state_v != MODULE_RUNTIME_UNKOWN) {
                rc = awemgr_module_set_state(awc_ctx_p, module_info, new_state_v);
                if (rc != awemgr_RC_OK)
                {
                    idbg_print(p, "Error! Module %s status not changed!\n", mod_name);
                }
            }
            else
            {
                idbg_print(p, "Error! State '%s' not understood. Module %s status not changed!\n", new_state, mod_name);
            }

        }
        else
        {
            enum awemgr_module_runtimestate state;
            rc = awemgr_module_get_state(awc_ctx_p, module_info, &state);

            if (rc == awemgr_RC_OK)
            {
                idbg_print(p, " > state = %s\n", awemgr_module_runtimestate_as_string  [state]);

                if (show_all_module_info)
                {
                    unsigned int classId;
                    rc = awemgr_module_get_class(awc_ctx_p, module_info, &classId);
                    if (rc == awemgr_RC_OK)
                    {
                        idbg_print(p, " > classId = %u (0x%X)\n", classId, classId);
                    }
                    else
                    {
                        idbg_print(p, "Failed to get classId for module %s.\n", mod_name);
                    }
                }
            }
            else
            {
                idbg_print(p, "Failed to get operating state of module %s.\n", mod_name);
            }

        }
    }
    else
    {
        idbg_print(p, "Module %s not found. Command failed\n", mod_name);
    }

    return IDBG_OK;
}