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

int amgr_init (IDBG_PARAMS)
{
    char *cfg_s = IDBG_GET_STRING("-cfg", NULL, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) {
        IDBG_CMDUSAGE ((p, "-cfg <cfgstring>",
                        "cfg <cfgstring>", "Configuration string, like 'mgr.api.log.level=debug;'",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    if (cfg_s)
    {
        if (aweconfig_from_string(appCtx_p->cfg_p, cfg_s) != AWECFG_RC_OK)
        {
            idbg_print(IDBG_HDL_VAR, "Could parse AWE-Manager string config: %s! Using default\n", cfg_s);
        }
    }

    int rc = awemgr_init(&appCtx_p->cfg_p, &(appCtx_p->mgr_p));
    if (rc != awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "Could not initialize AWE-Manager!\n");
        return IDBG_STOP;
    }
    return IDBG_OK;
}

int amgr_exit (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    bool doDetachOnly = IDBG_CHK_FLAG("-detach");
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) {
        IDBG_CMDUSAGE ((p, "[-detach]",
                        "-detach", "Quit AWE-Manager but do not stop AWECore processing.",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (doDetachOnly)
    {
        for (int idx = 0; idx < MAX_AWE_ENDPOINTS; idx++)
        {
            struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, idx);
            if (awc_ctx_p != NULL)
                (void)awemgr_skip_unload_design_on_exit(awc_ctx_p);
        }
    }

    int rc = awemgr_exit(&appCtx_p->mgr_p);
    if (rc != awemgr_RC_OK)
    {
        return IDBG_STOP;
    }
    return IDBG_OK;
}

static void printConfig(const char* key, const char* value, const char* description, void* context)
{
    idbg_t *p = (idbg_t *) context;
    idbg_print(p, "%-30s%-20s%s\n", key, value, description);
}

int amgr_config(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *set_key = IDBG_GET_STRING("-key", NULL, ARG_OPTIONAL);
    char *set_value = IDBG_GET_STRING("-value", NULL, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((IDBG_HDL_VAR, "[-key <key> -value <value>]",
                        "key \"<key>\"", "name of the config object (as string!)",
                        "value \"<val>\"", "value for the config object (as string!)",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (set_key && set_value) {
        awemgr_config_set(appCtx_p->cfg_p, set_key, set_value);
        return IDBG_OK;
    }

    // TODO: print as structured data, like JSON/YML
    idbg_print(IDBG_HDL_VAR, "%-30s%-20s%s\n", "Key", "Value", "Description");
    idbg_print(IDBG_HDL_VAR, "%-30s%-20s%s\n", "------------------------------", "--------------------", "------------------------------");

    aweconfig_foreach_item(appCtx_p->cfg_p, printConfig, IDBG_HDL_VAR);

    return IDBG_OK;
}

int awc_load(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *awc_file = IDBG_GET_STRING("-awc", NULL, ARG_NEEDED);
    int endpoint = IDBG_GET_INT("-endpoint", 0, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-awc <fname> -endpoint <idx>",
                        "awc <fname>", "name of AWC to load",
                        "endpoint <idx>", "Applies this AWC to a specific core/instance/endpoint.",
        				NULL, NULL));
        return IDBG_OK;
    }

    enum awemgr_rc rc = awemgr_load_awc(appCtx_p->mgr_p, awc_file, endpoint);
    if (rc != awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "No AWC loaded!\n");
    }
    else
    {
        appCtx_p->endpointId = endpoint;

    }
    // idbg_print(IDBG_HDL_VAR, "AWC context loaded endpointId=%d.\n", cardId);
    return IDBG_OK;
}

int awc_unload(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    enum awemgr_rc rc = awemgr_unload_awc(appCtx_p->mgr_p, appCtx_p->endpointId);
    if (rc != awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "AWC unloaded failed!\n");
    }
    return IDBG_OK;
}

int design_load(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *awb_label = IDBG_GET_STRING("-name", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-name <name>",
                        "name <name>", "name as given in AWC index file",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    enum awemgr_rc rc = awemgr_load_design(awc_ctx_p, awb_label);

    if (rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "AWB Design loaded and started.\n");
    }

    return IDBG_OK;
}

int design_unload(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    char *awb_label = IDBG_GET_STRING("-name", NULL, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "Stops audio processing and unloads current design  [-name <name>]",
                        "name <name>", "name as given in AWC index file",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    if (awc_ctx_p)
    {
        enum awemgr_rc rc = awemgr_unload_design(awc_ctx_p, awb_label);

        if (rc != awemgr_RC_OK)
        {
            idbg_print(IDBG_HDL_VAR, "AWB Design unload failed.\n");
        }
    }
    return IDBG_OK;
}

int audio_stop(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    int rc = awemgr_audio_stop(awc_ctx_p);
    if (rc != awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "Audio stop Failed.\n");
    }
    return IDBG_OK;
}

int audio_start(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    int rc = awemgr_audio_start(awc_ctx_p);
    if (rc != awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "Audio start Failed.");
    }
    return IDBG_OK;
}

int sys_comm(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "[<ip>@<port>]",
                        "<ip>@<port>", "host IP and port of AWE Server or target",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (argc < 2)
    {
        const char* ip = awemgr_config_get(appCtx_p->cfg_p, "mgr.ctrl.socket.ip", NULL);
        const char* port = awemgr_config_get(appCtx_p->cfg_p, "mgr.ctrl.socket.port", NULL);

        idbg_print(IDBG_HDL_VAR, "> awe_CTRL socket : %s@%s\n", ip, port);
    }
    else
    {
        char *ip = strtok(argv[1], "@");
        char *port = strtok(NULL, "@");
        (void)awemgr_config_set(appCtx_p->cfg_p, "mgr.ctrl.socket.ip", ip);
        (void)awemgr_config_set(appCtx_p->cfg_p, "mgr.ctrl.socket.port", port);

        idbg_print(IDBG_HDL_VAR, "> awe_CTRL socket : %s@%s\n", ip, port);
    }

    return IDBG_OK;
}

int sys_comm_trace(IDBG_PARAMS)
{
    char *trace_file = IDBG_GET_STRING("-file", NULL, ARG_OPTIONAL);
    bool enable_trace = IDBG_CHK_FLAG("-on");
    bool disable_trace = IDBG_CHK_FLAG("-off");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR || (enable_trace && disable_trace))
    {
        IDBG_CMDUSAGE ((p, "[-file <filepath>] [-on][-off]",
                        "file <filepath>", "Store traces into file",
                        "on", "enable COMM traces",
                        "off", "disable COMM traces",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (enable_trace)
      awectrl_set_traces(true);

    if (disable_trace)
    {
        awectrl_set_traces(false);
        FILE *fp = awectrl_get_trace_file();
        if (fp)
            fclose(fp);
        awectrl_set_trace_file(NULL);
    }

    if (trace_file)
    {
        FILE *fp = fopen(trace_file, "wb");
        if (fp)
        {
            awectrl_set_trace_file(fp);
        }
    }

    idbg_print(IDBG_HDL_VAR, "> awe_CTRL print traces: %s\n", awectrl_get_traces() ? "ON" : "-off-");
    idbg_print(IDBG_HDL_VAR, "> awe_CTRL tuneAWB dump: %s\n", awectrl_get_trace_file() ? trace_file : "-off-");

    return IDBG_OK;
}

int awc_select(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    int endpoint = IDBG_GET_INT("-endpoint", 0, ARG_OPTIONAL);
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-endpoint <index>",
                        "endpoint", "Endpoint/Index of the AWC (first loaded AWC = 0)",
        				NULL, NULL));
        return IDBG_OK;
    }

    int nr_supported_awcs = awemgr_get_max_awcs();
    if(endpoint < 0 || endpoint >= nr_supported_awcs)
    {
        idbg_print(IDBG_HDL_VAR, "> Invalid endpointId. Endpoint should be between 0 and %d\n", nr_supported_awcs - 1);
    }

    idbg_print(IDBG_HDL_VAR, "> AWC endpointId: %d (supported endpointIds 0 - %d)\n", appCtx_p->endpointId, nr_supported_awcs - 1);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, endpoint);
    if (awc_ctx_p)
    {
        appCtx_p->endpointId = endpoint;
        idbg_print(IDBG_HDL_VAR, "> AWC instanceId: %d\n", appCtx_p->endpointId);
    }
    else
        idbg_print(IDBG_HDL_VAR, "No AWC loaded on the endpoint\n");

    return IDBG_OK;
}

int script(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *script_file = IDBG_GET_STRING("-file", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-file <fname>",
                        "file <fname>", "path of script file to execute",
        				NULL, NULL));
        return IDBG_OK;
    }
    int retval = appCtx_p->idbg_srv_p->runFile(script_file);
    if (retval) {
        idbg_print(IDBG_HDL_VAR, "Error executing script %s\n", script_file);
    }
    return IDBG_OK;
}
