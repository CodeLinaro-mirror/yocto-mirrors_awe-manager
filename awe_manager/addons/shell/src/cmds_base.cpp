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

#include "awosal_time.h"   // for aweosal_measure_start()/aweosal_measure_elapsed()

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <string>
#include <vector>


// ******************************************************************************************************
// Comm shell tap: prints raw TX/RX words to the shell socket via idbg_print_direct.
// Installed by sys_comm_trace when -on -on is given; ctx is idbg_t *.
//
// The tap is called from within the AWE Manager API calls a command performs.
// idbg_print_direct() is used so that the traces are never collected into a
// command's held output buffer (see IdbgOutputHold) - they stream out as they
// happen and the command's own output stays one contiguous block.

static void comm_observer_cb(const char *direction, const void *data,
                             int data_sz_words, void *ctx)
{
    if (ctx == nullptr)
        return;

    idbg_t *p = (idbg_t *)ctx;
    const uint32_t *words = (const uint32_t *)data;
    idbg_print_direct(p, "%s: [\n", direction);
    for (int i = 0; i < data_sz_words; ++i)
    {
        if (i % 8 == 0)
            idbg_print_direct(p, "   ");
        idbg_print_direct(p, "0x%08x", words[i]);
        bool last = (i + 1 == data_sz_words);
        if (!last)
            idbg_print_direct(p, (i + 1) % 8 == 0 ? ",\n" : ", ");
    }
    idbg_print_direct(p, "\n]\n");
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
        IDBG_CMDUSAGE ((p, "[-file <basename>] [-on][-off]",
                        "-file <basename>", "Store traces into files (TX -> <basename>.tx, RX -> <basename>.rx)",
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

/** how deeply script files may include further script files */
static const int MAX_SCRIPT_NESTING = 8;

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

    /* A script file may include further script files, but one that includes
     * itself - directly or through a chain of others - would recurse until the
     * stack is exhausted. */
    CmdNestingGuard nesting((appCtx_p != NULL) ? &appCtx_p->script_nesting : NULL,
                            MAX_SCRIPT_NESTING);
    if (!nesting.allowed())
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR,
            "script '%s' is nested more than %d levels deep - recursive include?\n",
            script_file, MAX_SCRIPT_NESTING);
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

// ******************************************************************************************************
// "repeat": executes another shell command over and over again for a given time.

/** a repeat run may not contain another repeat run */
static const int MAX_REPEAT_NESTING = 1;

int repeat_command(IDBG_PARAMS)
{
    char *cmd_s   = IDBG_GET_STRING("-cmd", NULL, ARG_NEEDED);
    int   seconds = IDBG_GET_INT("-sec", 1, ARG_OPTIONAL);
    int   throttle = IDBG_GET_INT("-throttle", 0, ARG_OPTIONAL);
    bool  verbose = IDBG_CHK_FLAG("-verbose");
    int   count = IDBG_GET_INT("-count", 0, ARG_OPTIONAL);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "-cmd <command> [-sec <seconds>][-count <number>][-verbose][-throttle <usec>]",
                        "-cmd <command>", "command line to repeat; quote it when it has parameters,",
                        NULL,             "e.g. -cmd \"info -cpu\"",
                        "-sec <seconds>", "time to keep repeating the command (default: 1);",
                        NULL,             "not used when -count is given",
                        "-count <number>", "number of executions to perform instead of a runtime;",
                        NULL,              "overrides -sec (default: 0 = end the run by -sec)",
                        "-verbose",       "print the output of the repeated command",
                        "-throttle <usec>",    "add a delay between command executions by the given microseconds (default: 0 = no throttle)",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (seconds <= 0)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "-sec must be greater than 0\n");
        return IDBG_OK;
    }

    if (count < 0)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "-count must be greater than or equal to 0\n");
        return IDBG_OK;
    }

    if (throttle < 0)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "-throttle must be greater than or equal to 0\n");
        return IDBG_OK;
    }

    /* A repeat inside a repeat is never what the user wants: the outer run
     * would count the inner runs instead of the repeated command, so its call
     * count and rate say nothing, and its runtime would be stretched to the
     * duration of the inner run. Reject it, also when the inner one is reached
     * through a script. */
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    CmdNestingGuard nesting((appCtx_p != NULL) ? &appCtx_p->repeat_nesting : NULL,
                            MAX_REPEAT_NESTING);
    if (!nesting.allowed())
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "repeat cannot be nested inside another repeat\n");
        return IDBG_OK;
    }

    /* The repeated command is dispatched on the same idbg handle, and that
     * re-uses the handle's argument vector and splits the command line in
     * place. Everything taken from argv has to be copied into own memory
     * before the first repetition runs - argv is invalid afterwards. */
    std::string       command(cmd_s);
    std::vector<char> line(command.size() + 2U);  // parser needs room behind the string


    unsigned long calls        = 0U;
    int           retval       = IDBG_OK;
    double        elapsed_ms   = 0.0;
    const double  duration_ms  = (double)seconds * 1000.0;
    /* -count ends the run after a number of executions instead of after a
     * runtime, so that a check can be defined by the work done rather than by
     * the time it takes. Zero means it was not given and -sec ends the run. */
    const unsigned long max_calls = (unsigned long)count;

    {
        // scope guard which drops output of repeated command if -verbose is not given
        IdbgOutputDrop output_drop(IDBG_HDL_VAR, !verbose);

        bool keep_going = false;

        aweosal_clock_time start_time = aweosal_measure_start();
        do
        {
            memcpy(line.data(), command.c_str(), command.size() + 1U);
            retval = idbg_parse_cmd(IDBG_HDL_VAR, (unsigned char*)line.data(), (int)line.size());
            calls++;
            elapsed_ms = aweosal_measure_elapsed(start_time);
            keep_going = (max_calls > 0U) ? (calls < max_calls)
                                         : (elapsed_ms < duration_ms);
            if ((throttle > 0) && (retval == IDBG_OK) && keep_going)
            {
                aweosal_usleep(throttle);
            }
        }
        while ((retval == IDBG_OK) && keep_going);
    }

    idbg_print(IDBG_HDL_VAR,
        "repeat:\n"
        "  command: \"%s\"\n"
        "  duration_sec: %.6f\n"
        "  calls: %lu\n"
        "  calls_per_sec: %.2f\n"
        ,
        command.c_str(),
        elapsed_ms / 1000.0,
        calls,
        (elapsed_ms > 0.0) ? (((double)calls * 1000.0) / elapsed_ms) : 0.0
    );

    if (retval != IDBG_OK)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "error: repeated command stopped the shell (return code %d)\n", retval);
    }

    return retval;
}
