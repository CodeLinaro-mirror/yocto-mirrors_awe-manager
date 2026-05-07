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
#include <stdint.h>


// ******************************************************************************************************
// Comm shell tap: prints raw TX/RX words to the shell socket via idbg_print.
// Installed by sys_comm_trace when -on -on is given; ctx is idbg_t *.

static void comm_observer_cb(const char *direction, const void *data,
                             int data_sz_words, void *ctx)
{
    if (ctx == nullptr)
        return;

    idbg_t *p = (idbg_t *)ctx;
    const uint32_t *words = (const uint32_t *)data;
    idbg_print(p, "%s: [\n", direction);
    for (int i = 0; i < data_sz_words; ++i)
    {
        if (i % 8 == 0)
            idbg_print(p, "   ");
        idbg_print(p, "0x%08x", words[i]);
        bool last = (i + 1 == data_sz_words);
        if (!last)
            idbg_print(p, (i + 1) % 8 == 0 ? ",\n" : ", ");
    }
    idbg_print(p, "\n]\n");
}

// ******************************************************************************************************

int amgr_init (IDBG_PARAMS)
{
    char *cfg_s = IDBG_GET_STRING("-cfg", NULL, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) {
        IDBG_CMDUSAGE ((p, "-cfg <cfgstring>",
                        "-cfg <cfgstring>", "Configuration string, like 'mgr.api.log.level=debug;'",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    if (cfg_s)
    {
        if (aweconfig_from_string(appCtx_p->cfg_p, cfg_s) != AWECFG_RC_OK)
        {
            idbg_print(IDBG_HDL_VAR, "warn: Could not parse AWE-Manager string config: %s! Using default\n", cfg_s);
        }
    }

    enum awemgr_rc rc = awemgr_init(&appCtx_p->cfg_p, &(appCtx_p->mgr_p));
    if (rc != awemgr_RC_OK)
    {
        if (appCtx_p->mgr_externally_initialized)
        {
            IDBG_PRINT_WARN(IDBG_HDL_VAR, "AWE-Manager handle is externally initialized. You may not modify it.\n");
        }
        else
        {
            IDBG_PRINT_ERR(IDBG_HDL_VAR, "AWE-Manager initialization failed!\n");
        }
        return IDBG_OK;
    }
    return IDBG_OK;
}

int amgr_exit (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    bool doDetachOnly = IDBG_CHK_FLAG("-detach");
    bool doForceExit = IDBG_CHK_FLAG("-force");
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) {
        IDBG_CMDUSAGE ((p, "[-detach][-force]",
                        "-detach", "Quit AWE-Manager but do not stop AWECore processing.",
                        "-force", "Force exit even for externally initialized handles.",
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

    if (appCtx_p->mgr_externally_initialized && !doForceExit)
    {
        IDBG_PRINT_WARN(IDBG_HDL_VAR, "AWE-Manager handle is externally initialized. You may not modify it unless with -force parameter.\n");
        return IDBG_OK;
    }

    enum awemgr_rc rc = awemgr_exit(&appCtx_p->mgr_p);
    if (rc != awemgr_RC_OK)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "Could not exit AWE-Manager!\n");
        return IDBG_STOP;
    }
    return IDBG_OK;
}

static void printConfig(const char* key, const char* value, const char* description, void* context)
{
    idbg_t *p = (idbg_t *) context;
    idbg_print(p, "  - key: %s\n    value: %s\n    description: %s\n", key, value, description);
}

int amgr_config(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *set_key = IDBG_GET_STRING("-key", NULL, ARG_OPTIONAL);
    char *set_value = IDBG_GET_STRING("-value", NULL, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((IDBG_HDL_VAR, "[-key <key> -value <value>]",
                        "-key <key>",     "name of the config object (as string!)",
                        "-value <val>",   "value for the config object (as string!)",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (set_key && set_value) {
        awemgr_config_set(appCtx_p->cfg_p, set_key, set_value);
        return IDBG_OK;
    }

    // TODO: print as structured data, like JSON/YML
    idbg_print(IDBG_HDL_VAR, "cfg:\n");

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
        IDBG_CMDUSAGE ((p, "-awc <fname> [-endpoint <idx>]",
                        "-awc <fname>",      "name of AWC to load",
                        "-endpoint <idx>",   "Applies this AWC to a specific core/instance/endpoint.",
        				NULL, NULL));
        return IDBG_OK;
    }

    enum awemgr_rc rc = awemgr_load_awc(appCtx_p->mgr_p, awc_file, endpoint);
    if (rc != awemgr_RC_OK)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "Failed to load AWC: %s\n", awc_file);
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
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "Unloading AWC failed!\n");
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
                        "-name <name>", "name as given in AWC index file",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    enum awemgr_rc rc = awemgr_load_design(awc_ctx_p, awb_label);

    if (rc != awemgr_RC_OK)
    {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Failed to load AWB Design: %s\n", awb_label);
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
                        "-name <name>", "name as given in AWC index file",
        				NULL, NULL));
        return IDBG_OK;
    }

    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    if (awc_ctx_p)
    {
        enum awemgr_rc rc = awemgr_unload_design(awc_ctx_p, awb_label);

        if (rc != awemgr_RC_OK)
        {
            IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Failed to unload AWB Design: %s\n", awb_label);
        }
    }
    return IDBG_OK;
}

int audio_stop(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    enum awemgr_rc rc = awemgr_audio_stop(awc_ctx_p);
    if (rc != awemgr_RC_OK)
    {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Audio stop failed.\n");
    }
    return IDBG_OK;
}

int audio_start(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);
    enum awemgr_rc rc = awemgr_audio_start(awc_ctx_p);
    if (rc != awemgr_RC_OK)
    {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Audio start failed.\n");
    }
    return IDBG_OK;
}

int sys_comm(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *uri = IDBG_GET_STRING("-uri", NULL, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "[-uri <ip>@<port>]",
                        "-uri <ip>@<port>", "host IP and port of AWE Server or target",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (uri == NULL)
    {
        const char* ip = awemgr_config_get(appCtx_p->cfg_p, "mgr.comm.socket.ip", NULL);
        const char* port = awemgr_config_get(appCtx_p->cfg_p, "mgr.comm.socket.port", NULL);

        idbg_print(IDBG_HDL_VAR, "ctrl_socket:\n  - host: %s\n  - port: %s\n", ip, port);
    }
    else
    {
        char * context = NULL;
        char *ip = strtok_r(uri, "@", &context);
        char *port = strtok_r(NULL, "@", &context);
        (void)awemgr_config_set(appCtx_p->cfg_p, "mgr.comm.socket.ip", ip);
        (void)awemgr_config_set(appCtx_p->cfg_p, "mgr.comm.socket.port", port);
    }

    return IDBG_OK;
}

int sys_comm_trace(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *trace_file   = IDBG_GET_STRING("-file", NULL, ARG_OPTIONAL);
    bool enable_trace  = IDBG_CHK_FLAG("-on");
    bool disable_trace = IDBG_CHK_FLAG("-off");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR || (enable_trace && disable_trace))
    {
        IDBG_CMDUSAGE ((p, "[-file <filepath>] [-on][-off]",
                        "-file <filepath>", "Store traces into file",
                        "-on",              "Enable COMM log traces",
                        "-off",             "Disable COMM log traces",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (enable_trace)
    {
        awemgr_config_set(appCtx_p->cfg_p, "mgr.comm.trace.state", "on");
        awemgr_set_comm_observer(appCtx_p->mgr_p, comm_observer_cb, p);
    }

    if (disable_trace)
    {
        awemgr_config_set(appCtx_p->cfg_p, "mgr.comm.trace.state", "off");
        awemgr_config_set(appCtx_p->cfg_p, "mgr.comm.trace.file", "~");
        awemgr_set_comm_observer(appCtx_p->mgr_p, NULL, NULL);
    }

    if (trace_file)
    {
        awemgr_config_set(appCtx_p->cfg_p, "mgr.comm.trace.file", trace_file);
    }

    const char* state = awemgr_config_get(appCtx_p->cfg_p, "mgr.comm.trace.state", NULL);
    const char* file_name = awemgr_config_get(appCtx_p->cfg_p, "mgr.comm.trace.file", NULL);

    idbg_print(IDBG_HDL_VAR, "traces:\n  state: %s\n  file: %s\n", state, file_name);

    return IDBG_OK;
}

int awc_select(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    int endpoint = IDBG_GET_INT("-endpoint", 0, ARG_OPTIONAL);
    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-endpoint <index>",
                        "-endpoint <index>", "Endpoint/Index of the AWC (first loaded AWC = 0)",
        				NULL, NULL));
        return IDBG_OK;
    }

    int nr_supported_awcs = awemgr_get_max_awcs();
    if(endpoint < 0 || endpoint >= nr_supported_awcs)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "Invalid endpointId. Endpoint should be between 0 and %d\n", nr_supported_awcs - 1);
    }

    idbg_print(IDBG_HDL_VAR, "awc:\n");
    idbg_print(IDBG_HDL_VAR, "  endpoint_id: %d\n", appCtx_p->endpointId);
    idbg_print(IDBG_HDL_VAR, "  endpoint_id_max: %d\n", nr_supported_awcs - 1);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, endpoint);
    if (awc_ctx_p)
    {
        appCtx_p->endpointId = endpoint;
        idbg_print(IDBG_HDL_VAR, "  instance_id: %d\n", appCtx_p->endpointId);
    }
    else
        idbg_print(IDBG_HDL_VAR, "  instance_id: ~\n");

    return IDBG_OK;
}

int script(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    char *script_file = IDBG_GET_STRING("-file", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-file <fname>",
                        "-file <fname>", "path of script file to execute",
        				NULL, NULL));
        return IDBG_OK;
    }
    int retval = appCtx_p->idbg_srv_p->runFile(script_file);
    if (retval) {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Executing script %s failed\n", script_file);
    }
    return IDBG_OK;
}

int sys_time_commands(IDBG_PARAMS)
{
    CIdbgSrv *idbgSrv_p = (CIdbgSrv *)(((struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR))->idbg_srv_p);

    bool enable_timecmds = IDBG_CHK_FLAG("-on");
    bool disable_timecmds = IDBG_CHK_FLAG("-off");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR || (enable_timecmds && disable_timecmds))
    {
        IDBG_CMDUSAGE ((p, "[-on][-off]",
                        "-on",  "enable timing output for each command executed",
                        "-off", "disable timing output for each command executed",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (enable_timecmds)
        idbgSrv_p->setTimeCommands(true);

    if (disable_timecmds)
        idbgSrv_p->setTimeCommands(false);

    idbg_print(IDBG_HDL_VAR, "time_commands: %s\n", idbgSrv_p->getTimeCommands() ? "true" : "false");

    return IDBG_OK;
}
