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

#include "awemgr_shell.h"
#include "awe_config.h"
#include "shell_ctx.h"

#include "cmds_base.h"
#include "cmds_control.h"
#include "cmds_events.h"
#include "cmds_reset.h"
#include "cmds_sendcommand.h"
#include "cmds_showinfo.h"
#include "cmds_targetinfo.h"
#include "cmds_userdata.h"
#include "cmds_signalflow_dtmf.h"

#include <stdarg.h>
#include <stdio.h>
#include <new>
#include <vector>

#define CFG_SHELL_INPUT_BUF_CHARS   "mgr.shell.input_buf_chars"
#define CFG_SHELL_SOCKET_TIMEOUTMS  "mgr.shell.socket.timeoutms"
#define DEFAULT_SHELL_INPUT_BUF_CHARS  "4096"
#define DEFAULT_SHELL_SOCKET_TIMEOUTMS "-1"


// ---- Built-in shell utilities -----------------------------------------------

static int amgr_version(IDBG_PARAMS)
{
    idbg_print(IDBG_HDL_VAR, "awemgr_version: %s\n", awemgr_get_version());
    return IDBG_OK;
}

/** Echo the rest of the command line (no " or ' allowed). */
static int amgr_echo(IDBG_PARAMS)
{
    char line[512] = {0};
    char *c_p = line;
    char *end_p = line + sizeof(line);

    if (argc > 1) {
        for (int i = 1; (i < argc) && (end_p - c_p) > 0; i++)
        {
            c_p += snprintf(c_p, (size_t)(end_p - c_p), "%s ", argv[i]);
        }
    }
    idbg_print(IDBG_HDL_VAR, "%s\n", line);
    return IDBG_OK;
}


// ---- Command tables ---------------------------------------------------------

IDBG_TBL_START(DynControlsMenu)
IDBG_TBL_END

IDBG_TBL_START(ShellMenuMain)
    IDBG_TBL_CMD(amgr_version,      "version",      "Prints AWE Manager version")
    IDBG_TBL_CMD(amgr_echo,         "echo",         "Prints the rest of the command line; no \" or ' allowed")
    IDBG_TBL_CMD(amgr_init,         "mgr_init",     "Initialize AWE Manager")
    IDBG_TBL_CMD(amgr_exit,         "mgr_exit",     "De-Allocate AWE Manager")
    IDBG_TBL_CMD(amgr_config,       "cfg",          "Handle AWE-Manager configuration")

    IDBG_TBL_CMD(awc_load,          "load_awc",     "Load control config from AWC file")
    IDBG_TBL_CMD(awc_unload,        "unload_awc",   "Unload the AWC information")
    IDBG_TBL_CMD(awc_select,        "awc",          "Selects endpoint in case multiple AWCs are loaded (multi-canvas)")

    IDBG_TBL_CMD(show_info_command, "show",         "Prints information about controllable items and data in AWC file.")
    IDBG_TBL_CMD(handle_sendcommand,"send_command",  "Transmits a special tuning command to the target (BSP command)")

    IDBG_TBL_CMD(target_info,       "info",         "Get target information")
    IDBG_TBL_CMD(reset_state_command,"reset",       "Reset states of AWECore modules")

    IDBG_TBL_CMD(design_load,       "load_design",  "Load and apply a design (AWB)")
    IDBG_TBL_CMD(design_unload,     "unload_design","Stops AWB processing and unloads a design")

    IDBG_TBL_CMD(audio_stop,        "audio_stop",   "Stops audio pumping, if the design is loaded and pumping")
    IDBG_TBL_CMD(audio_start,       "audio_start",  "Starts audio pumping explicitly on a loaded design")

    IDBG_TBL_CMD(ctrl_enumerate,    "enum_controls","Populate an IDBG directory with control items from AWC")
    IDBG_TBL_CMD(module_generic,    "module",       "Handle module's operating state or meta-info")
    IDBG_TBL_SUB_DIR(DynControlsMenu, "controls",  "Filled with commands from AWC by 'enum_controls'")

    IDBG_TBL_CMD(ctrl_generic_get,  "get_value",    "Generic value retrieval")
    IDBG_TBL_CMD(ctrl_generic_set,  "set_value",    "Generic value setting")
    IDBG_TBL_CMD(ctrl_transact,     "transact",     "Binary buffer in and out")

    IDBG_TBL_CMD(handle_event_cmd,  "event",        "Handle BSP system and AWE module events")
    IDBG_TBL_CMD(sys_comm,          "comm",         "Configuration of communication to AWEcore")
    IDBG_TBL_CMD(sys_comm_trace,    "comm-trace",   "Trace communication")
    IDBG_TBL_CMD(sys_time_commands, "time-cmds",    "Enables/disables timing output for each command executed")

    IDBG_TBL_CMD(user_data_info,    "user_data",    "Show data attached to modules/controls by system integrator")

    IDBG_TBL_CMD(script,            "script",       "Load and execute another script file")
IDBG_TBL_END


// ---- Internal C++ context ---------------------------------------------------

struct awemgr_shell_ctx
{
    app_ctx          ctx;
    CIdbgSrv         idbg_srv;
    bool             config_externally_owned;
    std::vector<char> exec_buf;

    awemgr_shell_ctx(struct awemgr_data *mgr_p, awe_config *cfg_p)
        : idbg_srv((idbgtableentry_t *)ShellMenuMain)
        , config_externally_owned(cfg_p != nullptr)
        , exec_buf(4096)
    {
        if (mgr_p) {
            ctx.mgr_p = mgr_p;
            ctx.mgr_externally_initialized = true;
            /* Default to endpoint 0 so commands like "show -designs" work
             * immediately when the AWC was loaded externally by the caller. */
            ctx.endpointId = 0;
        }

        if (cfg_p) {
            /* Replace the config the app_ctx constructor auto-created with the
             * caller-supplied one.  The caller retains ownership. */
            awemgr_config_destroy(&ctx.cfg_p);
            ctx.cfg_p = cfg_p;
            awemgr_shell_register_configs(ctx.cfg_p);

        } else {
            awemgr_shell_register_configs(ctx.cfg_p);
            /* Apply environment-variable overrides to the internally-created
             * config so they take effect before the user calls mgr_init. */
            (void) aweconfig_from_envvar(ctx.cfg_p, AWEMGR_CFG_OVERRIDE);
        }


        uint32_t val;
        if (aweconfig_get_as_uint(ctx.cfg_p, CFG_SHELL_INPUT_BUF_CHARS, &val) == AWECFG_RC_OK
                && val > 0)
            exec_buf.resize(val);

        ctx.idbg_srv_p = &idbg_srv;
        idbg_srv.setUserData(&ctx);
    }

    ~awemgr_shell_ctx()
    {
        /* Prevent the app_ctx destructor from freeing a caller-owned config. */
        if (config_externally_owned)
            ctx.cfg_p = nullptr;
    }
};


// ---- Public C API -----------------------------------------------------------

extern "C" {

int awemgr_shell_register_configs(awe_config *cfg_p)
{
    if (!cfg_p)
        return AWECFG_RC_FAIL;

    aweconfig_init_tuple configs[] = {
        {CFG_SHELL_INPUT_BUF_CHARS, DEFAULT_SHELL_INPUT_BUF_CHARS,
         "Maximum byte length of a command string in awemgr_shell_execute()", NULL, NULL},
        {CFG_SHELL_SOCKET_TIMEOUTMS, DEFAULT_SHELL_SOCKET_TIMEOUTMS,
         "Shell socket session timeout in milliseconds (-1 = wait indefinitely)", NULL, NULL},
    };
    return aweconfig_add_multiple(cfg_p, configs, sizeof(configs) / sizeof(configs[0]));
}

awemgr_shell_ctx *awemgr_shell_create(struct awemgr_data *mgr_p, awe_config *cfg_p)
{
    return new (std::nothrow) awemgr_shell_ctx(mgr_p, cfg_p);
}

void awemgr_shell_destroy(awemgr_shell_ctx *ctx)
{
    delete ctx;
}

int awemgr_shell_execute(awemgr_shell_ctx *ctx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(ctx->exec_buf.data(), ctx->exec_buf.size(), fmt, args);
    va_end(args);
    return ctx->idbg_srv.runCommand(ctx->exec_buf.data());
}

int awemgr_shell_run_console(awemgr_shell_ctx *ctx)
{
    return ctx->idbg_srv.runConsole();
}

int awemgr_shell_run_file(awemgr_shell_ctx *ctx, const char *filename)
{
    return ctx->idbg_srv.runFile((char *)filename);
}

int awemgr_shell_run_socket(awemgr_shell_ctx *ctx, const char *host, const char *port)
{
    return ctx->idbg_srv.runSocket((char *)host, (char *)port);
}

void awemgr_shell_stop(awemgr_shell_ctx *ctx)
{
    ctx->idbg_srv.stop();
}

struct awemgr_data *awemgr_shell_get_mgr(const awemgr_shell_ctx *ctx)
{
    return ctx->ctx.mgr_p;
}

int awemgr_shell_get_endpoint_id(const awemgr_shell_ctx *ctx)
{
    return ctx->ctx.endpointId;
}

void awemgr_shell_set_endpoint_id(awemgr_shell_ctx *ctx, int endpoint_id)
{
    ctx->ctx.endpointId = endpoint_id;
}

void awemgr_shell_set_time_commands(awemgr_shell_ctx *ctx, int enable)
{
    ctx->idbg_srv.setTimeCommands(enable != 0);
}

} // extern "C"
