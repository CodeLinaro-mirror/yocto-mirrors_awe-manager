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


#ifndef INCLUSION_GUARD_SHELL_CTX_H
#define INCLUSION_GUARD_SHELL_CTX_H

#include "awe_manager.h"
#include "idbg_srv.h"

#include <thread>

struct ev_reader_ctx {
    std::thread event_reading_thread; // thread for continuously reading the event data

    bool  stop_flag {false};
    char* ev_dump_file_name {nullptr};  // file name of the current event buffer dump file
};

/** Context structure for the shell addon.
 * Stored as the idbg_t user data pointer.
 *
 * Contains the AWE Manager context and any shell-specific state.
 *
 * */
typedef struct app_ctx_
{
    app_ctx_()
    {
        awemgr_config_create(&cfg_p);
        awemgr_config_set(cfg_p, CFG_MGR_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_AWC_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_CMD_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_COMM_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_OSAL_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_CFG_LOGLEVEL, LOG_LEVEL_WARN);
    }
    ~app_ctx_()
    {
        awemgr_config_destroy(&cfg_p);
    }

    // handle to AWE Manager instance
    struct awemgr_data *mgr_p{nullptr};

    // true if shell did not create the manager instance, so it should not destroy it on exit
    bool                mgr_externally_initialized{false};

    // handle to AWE Config
    awe_config         *cfg_p{nullptr};

    // "MultiCanvas endpoint" - set by the user via the -endpointId flag; used to get the AWC context for manager API calls
    int                 endpointId{-1};

    // if shell is used to tap into the event stream, this context will be used for the event reading thread and callback
    ev_reader_ctx       ev_reader;

    // the shell server instance
    CIdbgSrv           *idbg_srv_p{nullptr};

    // Nesting depth of the commands which dispatch further command lines on the
    // same idbg handle, i.e. "repeat" and "script". Both can end up invoking
    // themselves, which is either useless or fatal, so each of them counts its
    // active invocations here and refuses to go deeper (see CmdNestingGuard).
    int                 repeat_nesting{0};
    int                 script_nesting{0};

} app_ctx;

#define DEF_VAR_MGR_DATA(p)  app_ctx *appCtx_p = (app_ctx *) idbg_get_userdata(p);
#define DEF_VAR_AWC_CTX(p)  struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(p->mgr_p, p->endpointId);
#define DEF_VARS_MGR_AND_CTX(p)    app_ctx *appCtx_p = (app_ctx *) idbg_get_userdata(p); \
                                   struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

#endif // INCLUSION_GUARD_SHELL_CTX_H
