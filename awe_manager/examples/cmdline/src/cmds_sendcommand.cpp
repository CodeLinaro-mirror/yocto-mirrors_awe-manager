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

#include "cmds_sendcommand.h"

#include "app_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "awe_ctrl.h"    // for setting configuration to AWECore
#include "awemgr_logging.h"  // for dumping buffer content
#include "hlp_dump_response.h"  // dump response

#include <stdlib.h>
#include <stdio.h>
#include <awosal_string.h>


/*
 * main entry point for command; analyze parameters and call sub-routines to handle specific tasks
 */
int handle_sendcommand (IDBG_PARAMS)
{
    DEF_VARS_MGR_AND_CTX(IDBG_HDL_VAR);

    int cmdId = IDBG_GET_INT("-cmd", 0, ARG_NEEDED);
    int coreId = IDBG_GET_INT("-core", 0, ARG_OPTIONAL);
    char *payload_string = IDBG_GET_STRING("-payload", NULL, ARG_OPTIONAL);
    char *payload_binary = IDBG_GET_STRING("-binary", NULL, ARG_OPTIONAL);
    bool isFloat = IDBG_CHK_FLAG("-float");

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR)
    {
        IDBG_CMDUSAGE ((p, "[OPTIONS]",
            "-cmd <id>", "Command ID value",
            "-core <id>", "ID of the CPU core this library shall be loaded on (default: 0)",
            "-payload", "A string which is used as a payload in the command",
            "-binary", "A space-separated list of values to form a binary buffer (not yet implemented)",
            "-float", "Indicates that the float values are given in -binary. Floats will be stored in 32bit values",
        	NULL, NULL));
        return IDBG_OK;
    }

    // ugly, but send_command does not support more than this anyhow right now, this applies to request and response
    unsigned int payload_buffer[260];
    int payload_size = 0;
    void *payload = NULL;

    if (payload_binary)
    {
        unsigned int *dp = payload_buffer;

        char *token = strtok(payload_binary, " ");
        while (token != NULL)
        {
            if (isFloat)
            {
                float v = strtof(token, NULL);

                *dp++ = *((unsigned int*)&v);
            }
            else
            {
                char *endptr;
                *dp++ = (unsigned int) strtol(token, &endptr, 0);
            }
            token = strtok(NULL, " ");
        }

        payload_size = dp - payload_buffer;
        payload = (void*)payload_buffer;

    }
    else if (payload_string)
    {
        int sz_bytes_incl_term = strlen(payload_string) + 1;
        payload_size = (sz_bytes_incl_term % 4 == 0) ? sz_bytes_incl_term / 4 : sz_bytes_incl_term / 4 + 1;
        // note: when /4+1 is used, we could theoretically include 3 bytes garbage behind the \0 termination byte; that should not matter!
        payload = (void*) payload_string;
    }

    unsigned int response_buffer[260];
    unsigned int nr_words_received;
    enum awemgr_rc rc = awemgr_send_command(awc_ctx_p, cmdId, coreId, payload, payload_size,
                                            response_buffer, sizeof(response_buffer), &nr_words_received);
    if (rc != awemgr_RC_OK) {
        idbg_print(p, "Sending command (cmd = %d) failed\n", cmdId);
    }
    else
    {
        if (nr_words_received > 0) {
            dump_response(p, "RX: ", response_buffer, nr_words_received * sizeof(unsigned int), AWEMGR_VARTYPE_INTEGER, false);
        }
    }
    return IDBG_OK;
}
