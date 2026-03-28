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


#ifndef INCLUSION_GUARD_APPCTX_H
#define INCLUSION_GUARD_APPCTX_H

#include "awe_manager.h"
#include "idbg_srv.h"

// TODO: !!! subst with aweOSAL when SHELL becomes part of AWE-Manager lib !!!
#include <thread>

struct ev_reader_ctx {
    std::thread event_reading_thread; // thread for continuously reading the event data

    bool  stop_flag {false};
    char* ev_dump_file_name {nullptr};  // file name of the current event buffer dump file
};

typedef struct app_ctx_
{
    app_ctx_()
    {
        awemgr_config_create(&cfg_p);
        awemgr_config_set(cfg_p, CFG_MGR_LOGLEVEL, LOG_LEVEL_INFO);
        awemgr_config_set(cfg_p, CFG_AWC_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_CMD_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_CTRL_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_OSAL_LOGLEVEL, LOG_LEVEL_WARN);
        awemgr_config_set(cfg_p, CFG_CFG_LOGLEVEL, LOG_LEVEL_WARN);
    }
    ~app_ctx_()
    {
        awemgr_config_destroy(&cfg_p);
    }
    struct awemgr_data *mgr_p{NULL};        // handle to AWE Manager instance
    awe_config         *cfg_p{NULL};        // handle to AWE Config
    int                 endpointId{-1};      // aka cardId - current design to handle
    ev_reader_ctx       ev_reader;

    CIdbgSrv           *idbg_srv_p{NULL};
} app_ctx;

#define DEF_VAR_MGR_DATA(p)  app_ctx *appCtx_p = (app_ctx *) idbg_get_userdata(p);
#define DEF_VAR_AWC_CTX(p)  struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(p->mgr_p, p->endpointId);
#define DEF_VARS_MGR_AND_CTX(p)    app_ctx *appCtx_p = (app_ctx *) idbg_get_userdata(p); \
                                   struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

#endif // INCLUSION_GUARD_APPCTX_H
