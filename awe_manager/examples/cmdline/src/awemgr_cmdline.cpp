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


/* ****************************************************************************
 * Small interactive shell for AWE Manager
 *
 * This executable initializes AWE Manager with a given AWC "file",
 * the corresponding AWB is loaded and started, audio should be played,
 * the program then enters a shell which allows to enter commands,
 * depending on the command, certain "control actions" are performed,
 * like changing the volume, or applying a preset file
 *
 * Note: this command shell only works with "compatible" AWE signal flows,
 *       i.e., the signal flow must contain the control items referenced
 *       by the shell commands !
 * ***************************************************************************/


#include "app_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "idbg_srv.h"
#include "awe_ctrl.h"    // for setting configuration to AWECore comm, before AWEMgr starts

// include "callback" commands or tables
#include "cmds_signalflow_dtmf.h"
#include "cmds_base.h"
#include "cmds_control.h"
#include "cmds_events.h"
#include "cmds_userdata.h"
#include "cmds_showinfo.h"
#include "cmds_targetinfo.h"
#include "cmds_sendcommand.h"
#include "cmds_reset.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// ******************************************************************************************************

static int amgr_version(IDBG_PARAMS)
{
    idbg_print(IDBG_HDL_VAR, "awemgr_version: %s\n", awemgr_get_version());
    return IDBG_OK;
}

/** simple command which enables printing stuff via IDBG connection,
 * it simply echos the argc/argv structure to output,
 * note that " or ' cannot be printed, even not via escaping with \
 */
static int amgr_echo(IDBG_PARAMS)
{
    char line[512] = {0};
    char *c_p = line;
    char *end_p = line + sizeof(line);

    if (argc > 1) {
        for (int i = 1; (i < argc) && (end_p - c_p) > 0; i++)
        {
            c_p += snprintf(c_p, (end_p - c_p), "%s ", argv[i]);
        }
    }
    idbg_print(IDBG_HDL_VAR, "%s\n", line);
    return IDBG_OK;
}


// ******************************************************************************************************

/* This defines a table of available commands which can be invoked interactively of via file,
 * each command ends up in the callback specified and provides an argc/argv structure to the
 * callback.
 *
 * It is possible to create a hierarchical structure by introducing sub-tables.
 */
// IDBG_TBL_START(DynModulesMenu)
// IDBG_TBL_END
IDBG_TBL_START(DynControlsMenu)
IDBG_TBL_END
IDBG_TBL_START(MenuMain)
    IDBG_TBL_CMD(amgr_version, "version", "Prints AWE Manager version")
    IDBG_TBL_CMD(amgr_echo, "echo", "Prints the rest of the command line; no \" or ' allowed")
    IDBG_TBL_CMD(amgr_init, "mgr_init", "Initialize AWE Manager")
    IDBG_TBL_CMD(amgr_exit, "mgr_exit", "De-Allocate AWE Manager")
    IDBG_TBL_CMD(amgr_config, "cfg", "Handle AWE-Manager configuration")

    IDBG_TBL_CMD(awc_load, "load_awc", "Load control config from AWC file")
    IDBG_TBL_CMD(awc_unload, "unload_awc", "Unload the AWC information")
    IDBG_TBL_CMD(awc_select, "awc", "Selects endpoint in case multiple AWCs are loaded (multi-canvas)")

    IDBG_TBL_CMD(show_info_command, "show", "Prints information about controllable items and data in AWC file.")
    IDBG_TBL_CMD(handle_sendcommand, "send_command", "Transmits a special tuning command to the target (BSP command)")

    IDBG_TBL_CMD(target_info, "info", "Get target information")
    IDBG_TBL_CMD(reset_state_command, "reset", "Reset states of AWECore modules")

    IDBG_TBL_CMD(design_load, "load_design", "Load and apply a design (AWB)")
    IDBG_TBL_CMD(design_unload, "unload_design", "Stops AWB processing and unloads a design")

    IDBG_TBL_CMD(audio_stop, "audio_stop", "Stops audio pumping, if the design is loaded and pumping")
    IDBG_TBL_CMD(audio_start, "audio_start", "Starts audio pumping explicitly on a loaded design")

    IDBG_TBL_CMD(ctrl_enumerate, "enum_controls", "Populate an IDBG directory with control items from AWC")
    IDBG_TBL_CMD(module_generic, "module", "Handle module's operating state or meta-info")
    IDBG_TBL_SUB_DIR(DynControlsMenu, "controls", "Filled with commands from AWC by 'enum_controls'")

    IDBG_TBL_CMD(ctrl_generic_get, "get_value", "Generic value retrieval")
    IDBG_TBL_CMD(ctrl_generic_set, "set_value", "Generic value setting")
    IDBG_TBL_CMD(ctrl_transact, "transact", "Binary buffer in and out")

    IDBG_TBL_CMD(handle_event_cmd, "event", "Handle BSP system and AWE module events")
    IDBG_TBL_CMD(sys_comm, "comm", "Configuration of communication to AWEcore")
    IDBG_TBL_CMD(sys_comm_trace, "comm-trace", "Trace communication")
    IDBG_TBL_CMD(sys_time_commands, "time-cmds", "Enables/disables timing output for each command executed")
    // IDBG_TBL_SUB_DIR(MenuDTMF_SF_Control,  "dtmf", "Controls for DTMF test signalflow")

    IDBG_TBL_CMD(user_data_info, "user_data", "Show data attached to modules/controls by system integrator")

    IDBG_TBL_CMD(script, "script", "Load and execute another script file")

IDBG_TBL_END


// ******************************************************************************************************

int main(int argc, char* argv[])
{
    int script_err = 0;

    // handles to AWE Manager and to one AWC -
    //   init with NULL here in this app to allow checks on de-alloc
    struct app_ctx_ appCtx;

    CIdbgSrv myIdbgSrv((idbgtableentry_t *) MenuMain);

    appCtx.idbg_srv_p = &myIdbgSrv;

    idbg_t *p;
    idbg_init(&p, NULL);
    char *awc_file = IDBG_GET_STRING("-awc", NULL, ARG_OPTIONAL);
    char *awb_name = IDBG_GET_STRING("-design", NULL, ARG_OPTIONAL);
    char *socket_port = IDBG_GET_STRING("-s", NULL, ARG_OPTIONAL);
    char *script_file = IDBG_GET_STRING("-f", NULL, ARG_OPTIONAL);
    char *cfg_string = IDBG_GET_STRING("-cfg", NULL, ARG_OPTIONAL);
    int do_interactive = IDBG_CHK_FLAG("-i");
    char *awectrl_socket = IDBG_GET_STRING("-awe_socket", NULL, ARG_OPTIONAL);
    bool enable_trace = IDBG_CHK_FLAG("-trace");
    bool enable_timecmds = IDBG_CHK_FLAG("-time");
    bool detach_silently = IDBG_CHK_FLAG("-detach_silently");
    bool show_version = IDBG_CHK_FLAG("-version");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        char usage[1024];
        snprintf(usage, sizeof(usage), "%s [PARAMS]", argv[0]);
        IDBG_CMDUSAGE ((p, usage,
                "-awc <awcfile>", "initialize AWE Manager at start and load AWC",
                "-cfg <cfg>", "Configuration string for AWE Manager at startup",
                "-design <name>", "loads a given design (eg: Main) at startup, used together with '-awc'",
                "-s <port>", "Use socket mode for interactive commands",
                "-f <cli-cmd-file>", "Execute script commands from file",
                "-i", "Start interactive console after file has been consumed with '-f'",
                "-awe_socket <ip@port>", "IP@port of AWE Server (PC version of AWE Manager only)",
                "-trace", "enables awe_CTRL communication traces",
                "-time", "enables timing output for each command executed",
                "-version", "shows the program version (git tag)",
                NULL, NULL));
        printf("\nWhen no file is given, an interactive console is started.\n");
        idbg_exit(&p);
        return -1;
    }

    if (show_version)
    {
        printf("Using AWE-Manager version: %s \n", awemgr_get_version());
        idbg_exit(&p);
        return 0;
    }

    myIdbgSrv.setUserData(&appCtx);
    myIdbgSrv.setTimeCommands(enable_timecmds);

    (void) aweconfig_from_envvar(appCtx.cfg_p, AWEMGR_CFG_OVERRIDE);

    if (enable_trace)
    {
        script_err = myIdbgSrv.runCommand("comm-trace -on");
    }

    if (awectrl_socket)
    {
        script_err = myIdbgSrv.runCommand("/comm -uri %s", awectrl_socket);
    }

    // check if AWC file has been given on "main commandline", and initialize
    // system accordingly already
    if (awc_file)
    {
        if (cfg_string == NULL)
        {
            script_err = myIdbgSrv.runCommand("/mgr_init");
        }
        else
        {
            script_err = myIdbgSrv.runCommand("/mgr_init -cfg \"%s\"", cfg_string);
        }

        if (!script_err)
        {
            script_err = myIdbgSrv.runCommand("/load_awc -awc %s", awc_file);

            if (!script_err)
            {
                // appCtx.endpointId = cardId;

                if (awb_name)
                {
                    script_err = myIdbgSrv.runCommand("/load_design -name %s", awb_name);
                    //struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx.mgr_p, cardId);
                    // rc = awemgr_load_design(awc_ctx_p, awb_name);
                }
            }
        }

    }

    // if all good so far, enter interactive debugging (possibly with executing script file before)
    if (! script_err)
    {
        int err = 0;
        if (socket_port)
        {
            printf("Starting IDBG Server localhost@: %s\n", socket_port);
            err = myIdbgSrv.runSocket((char*)"localhost", socket_port);
        }
        else if (script_file)
        {
            err = myIdbgSrv.runFile(script_file);

            if (!err && do_interactive)
            {
                err = myIdbgSrv.runConsole();
            }
        }
        else
        {
            err = myIdbgSrv.runConsole();
        }
    }

    if (!detach_silently)
    {
        if (appCtx.mgr_p)
        {
            if (appCtx.endpointId >= 0)
            {
                struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx.mgr_p, appCtx.endpointId);
                if (awc_ctx_p)
                {
                    script_err = myIdbgSrv.runCommand("/unload_awc");
                }
            }
            script_err = myIdbgSrv.runCommand("/mgr_exit");
        }
    }

    // cleanup cmdline handling to avoid mem-leak
    idbg_exit(&p);

    return script_err;
}
