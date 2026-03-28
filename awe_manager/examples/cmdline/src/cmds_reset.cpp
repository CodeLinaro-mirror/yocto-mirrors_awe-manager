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

#include "cmds_showinfo.h"

#include "app_ctx.h"     // own application related types and interactive cmdline

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int reset_state_command(IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
    IDBG_CMDUSAGE ((p, "[-core <index>][-endpoint <index>]",
                    "-endpoint <index>", "Endpoint/Index of the AWC (NOT USED YET!)",
                    "-core <index>", "In a multi-core system, index of the CPU (NOT USED YET!)",
                    NULL, NULL));
        return IDBG_OK;
    }

    enum awemgr_rc rc = awemgr_reset_state_all(appCtx_p->mgr_p);

    if (rc == awemgr_RC_OK)
    {
        idbg_print(IDBG_HDL_VAR, "status: Reset states command sent successfully to all cores.\n");
    }
    return IDBG_OK;
}