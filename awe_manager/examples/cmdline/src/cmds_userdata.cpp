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

#include "cmds_userdata.h"

#include "app_ctx.h"     // own application related types and interactive cmdline 
#include "awe_manager.h" // the AWE Manager include

#include "cmds_showinfo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void user_data_usage(idbg_t *p)
{
    IDBG_CMDUSAGE ((p, "-name <mod or var name>[-key <userkey>]",
                    "-name <mod or var name>", "module or variable name as in AWC file",
                    "-list", "prints all user data info attached to modules/variables",
                    "-key <userkey>", "name of the user data item; only shows data items with this name; useful with '-list'",
                    NULL, NULL));
}


int user_data_info(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    awemgr_userdata data;

    char *mod_or_var_name = IDBG_GET_STRING("-name", NULL, ARG_OPTIONAL);
    char *key = IDBG_GET_STRING("-key", NULL, ARG_OPTIONAL);
    bool show_list = IDBG_CHK_FLAG("-list");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) 
    {
        user_data_usage(IDBG_HDL_VAR);
        return IDBG_OK;
    }

    if (show_list) 
    {
        show_available_userdata(IDBG_HDL_VAR, awc_ctx_p, key, "");
        return IDBG_OK;
    }

    if (!mod_or_var_name) 
    {
        user_data_usage(IDBG_HDL_VAR);
        return IDBG_OK;
    }

    // a module or a variable name is given; check if there is a match...

    bool is_module = true;
    int nr_items = awemgr_get_module_userdata_count(awc_ctx_p, mod_or_var_name);
    if (nr_items <= 0) 
    {
        nr_items = awemgr_get_control_userdata_count(awc_ctx_p, mod_or_var_name);
        is_module = false;
    }

    // there was an error; neither a module or a variable with that name was found
    if (nr_items < 0) 
    {
        idbg_print(p, "No module or variable found for %s\n", mod_or_var_name);
    } 
    else
    {
        // if no user defined items are attached, bail out
        if (nr_items == 0) 
        {
            idbg_print(p, "No user data for %s\n", mod_or_var_name);
        } 
        else 
        {

            enum awemgr_rc rc;

            if (key) 
            {
                // get a specific data item by key name directly

                if (is_module)
                    rc = awemgr_get_module_userdata_by_key(awc_ctx_p, mod_or_var_name, key, &data);
                else
                    rc = awemgr_get_control_userdata_by_key(awc_ctx_p, mod_or_var_name, key, &data);

                if (rc != awemgr_RC_OK) 
                {
                    idbg_print(p, "ERROR: could not find user data for key '%s'\n", key);
                }
                else 
                {
                    show_user_data_item(p, mod_or_var_name, data, "");
                }
            } 
            else 
            {

                // loop over all items of this module/variable

                for (int idx = 0; idx < nr_items; idx++)
                {
                    if (is_module)
                        rc = awemgr_get_module_userdata_by_index(awc_ctx_p, mod_or_var_name, idx, &data);
                    else
                        rc = awemgr_get_control_userdata_by_index(awc_ctx_p, mod_or_var_name, idx, &data);

                    if (rc != awemgr_RC_OK) 
                    {
                        idbg_print(p, "ERROR: could not get user data for index %d\n", idx);
                        // note: should actually never get here ;-)
                    } else 
                    {
                        show_user_data_item(p, mod_or_var_name, data, "");
                    }
                } 
            }
        }
    }

    return IDBG_OK;
}