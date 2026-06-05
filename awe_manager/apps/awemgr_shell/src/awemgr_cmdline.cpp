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


#include "awemgr_shell.h"  // the shell library
#include "awe_manager.h"   // for aweconfig_from_envvar, AWEMGR_CFG_OVERRIDE
#include "idbg.h"          // for idbg_init / idbg_exit and argument parsing macros

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[])
{
    int script_err = 0;

    /* Use a standalone idbg instance solely for parsing main()'s arguments.
     * The shell library manages its own internal CIdbgSrv for command dispatch. */
    idbg_t *p;
    idbg_init(&p, NULL);
    char *awc_file        = IDBG_GET_STRING("-awc",        NULL, ARG_OPTIONAL);
    char *awb_name        = IDBG_GET_STRING("-design",     NULL, ARG_OPTIONAL);
    char *socket_port     = IDBG_GET_STRING("-s",          NULL, ARG_OPTIONAL);
    char *script_file     = IDBG_GET_STRING("-f",          NULL, ARG_OPTIONAL);
    char *cfg_string      = IDBG_GET_STRING("-cfg",        NULL, ARG_OPTIONAL);
    int   do_interactive  = IDBG_CHK_FLAG("-i");
    char *awe_socket      = IDBG_GET_STRING("-awe_socket", NULL, ARG_OPTIONAL);
    bool  enable_trace    = IDBG_CHK_FLAG("-trace");
    bool  enable_timecmds = IDBG_CHK_FLAG("-time");
    bool  detach_silently = IDBG_CHK_FLAG("-detach_silently");
    bool  show_version    = IDBG_CHK_FLAG("-version");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        char usage[1024];
        snprintf(usage, sizeof(usage), "%s [PARAMS]", argv[0]);
        IDBG_CMDUSAGE ((p, usage,
                "-awc <awcfile>",    "initialize AWE Manager at start and load AWC",
                "-cfg <cfg>",        "Configuration string for AWE Manager at startup",
                "-design <name>",    "loads a given design (eg: Main) at startup, used together with '-awc'",
                "-s <port>",         "Use socket mode for interactive commands",
                "-f <cli-cmd-file>", "Execute script commands from file",
                "-i",                "Start interactive console after file has been consumed with '-f'",
                "-awe_socket <ip@port>", "IP@port of AWE Server (PC version of AWE Manager only)",
                "-trace",            "enables awe_CTRL communication traces",
                "-time",             "enables timing output for each command executed",
                "-version",          "shows the program version (git tag)",
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

    /* Create shell context — starts with no manager; cfg is created internally. */
    awemgr_shell_ctx *shell = awemgr_shell_create(NULL, NULL);
    if (!shell)
    {
        fprintf(stderr, "Failed to allocate shell context\n");
        idbg_exit(&p);
        return -1;
    }

    awemgr_shell_set_time_commands(shell, enable_timecmds ? 1 : 0);

    /* Environment-variable config overrides are applied automatically inside
     * awemgr_shell_create() when the library owns its config (cfg_p == NULL). */

    if (enable_trace)
    {
        script_err = awemgr_shell_execute(shell, "comm-trace -on");
    }

    if (awe_socket)
    {
        script_err = awemgr_shell_execute(shell, "/comm -uri %s", awe_socket);
    }

    /* Optional startup sequence when -awc is supplied on the command line. */
    if (awc_file)
    {
        if (cfg_string == NULL)
        {
            script_err = awemgr_shell_execute(shell, "/mgr_init");
        }
        else
        {
            script_err = awemgr_shell_execute(shell, "/mgr_init -cfg \"%s\"", cfg_string);
        }

        if (!script_err)
        {
            script_err = awemgr_shell_execute(shell, "/load_awc -awc %s", awc_file);

            if (!script_err && awb_name)
            {
                script_err = awemgr_shell_execute(shell, "/load_design -name %s", awb_name);
            }
        }
    }

    /* Enter the main I/O loop. */
    if (!script_err)
    {
        if (socket_port)
        {
            printf("Starting IDBG Server localhost@: %s\n", socket_port);
            awemgr_shell_run_socket(shell, "localhost", socket_port);
        }
        else if (script_file)
        {
            int err = awemgr_shell_run_file(shell, script_file);

            if (!err && do_interactive)
            {
                awemgr_shell_run_console(shell);
            }
        }
        else
        {
            awemgr_shell_run_console(shell);
        }
    }

    /* Stop any running event listener thread before tearing down. */
    awemgr_shell_execute(shell, "/event -stop");

    if (!detach_silently)
    {
        if (awemgr_shell_get_mgr(shell))
        {
            if (awemgr_shell_get_endpoint_id(shell) >= 0)
            {
                script_err = awemgr_shell_execute(shell, "/unload_awc");
            }
            script_err = awemgr_shell_execute(shell, "/mgr_exit");
        }
    }

    awemgr_shell_destroy(shell);
    idbg_exit(&p);

    return script_err;
}
