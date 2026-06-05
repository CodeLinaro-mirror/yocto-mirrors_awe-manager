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

#include "cmds_sendcommand.h"

#include "shell_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "awe_comm.h"    // for setting configuration to AWECore
#include "awemgr_logging.h"  // for dumping buffer content
#include "hlp_dump_response.h"  // dump response
#include "hlp_functions.h" // for helper functions like IDBG_PRINT_ERR

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
        IDBG_CMDUSAGE ((IDBG_HDL_VAR, "[OPTIONS]",
            "-cmd <id>", "Command ID value",
            "-core <id>", "ID of the CPU core this library shall be loaded on (default: 0)",
            "-payload", "A string which is used as a payload in the command",
            "-binary", "A space-separated list of values to form a binary buffer, like \"123 0x10\"",
            "-float", "Indicates that the float values are given in -binary. Floats will be stored in 32bit values",
        	NULL, NULL));
        return IDBG_OK;
    }

    // get tuning buffer size and allocate memory here
    // default value, should be overwritten by config value; note:
    // this is the total buffer size, including header, so actual payload buffer size is smaller by 4 words (16 bytes)
    uint32_t tuning_buffer_size = 264;

    if (aweconfig_get_as_uint(appCtx_p->cfg_p, "mgr.comm.buffersize", &tuning_buffer_size) != AWECFG_RC_OK)
    {
        IDBG_PRINT_ERR(IDBG_HDL_VAR, "Failed to get tuning buffer size from config. Check %s\n", "mgr.comm.buffersize");
        return IDBG_OK;
    }

    uint32_t max_payload_size_in_words = tuning_buffer_size - 4;
    unsigned int *payload_buffer = (unsigned int*) calloc(max_payload_size_in_words, sizeof(unsigned int));
    unsigned int *response_buffer = (unsigned int*) calloc(max_payload_size_in_words, sizeof(unsigned int));
    if (payload_buffer == NULL || response_buffer == NULL)
    {
        idbg_print(IDBG_HDL_VAR, "Memory allocation problems. Tuning buffer size (for tx and rx) is %u!\n", tuning_buffer_size);
        if (payload_buffer != NULL) free(payload_buffer);
        if (response_buffer != NULL) free(response_buffer);
        return IDBG_STOP;
    }
    unsigned int payload_size = 0;
    void *payload = NULL;

    if (payload_binary)
    {
        unsigned int *dp = payload_buffer;
        unsigned int tmp;

        char *context = NULL;
        char *token = strtok_r(payload_binary, " ", &context);
        while (token != NULL)
        {
            if (isFloat)
            {
                float v = strtof(token, NULL);

                memcpy(&tmp, &v, sizeof tmp);
                *dp++ = tmp;
            }
            else
            {
                char *endptr;
                *dp++ = (unsigned int) strtol(token, &endptr, 0);
            }
            token = strtok_r(NULL, " ", &context);
        }

        payload_size = (unsigned int) (dp - payload_buffer);
        payload = (void*)payload_buffer;

    }
    else if (payload_string)
    {
        unsigned int sz_bytes_incl_term = (unsigned int) strlen(payload_string) + 1;
        payload_size = (sz_bytes_incl_term % 4 == 0) ? sz_bytes_incl_term / 4 : sz_bytes_incl_term / 4 + 1;
        // note: when /4+1 is used, we could theoretically include 3 bytes garbage behind the \0 termination byte; that should not matter!
        strlcpy((char*) payload_buffer, payload_string, max_payload_size_in_words * sizeof(unsigned int));
        payload = (void*) payload_buffer;
    }

    unsigned int nr_words_received;
    enum awemgr_rc rc = awemgr_send_command(awc_ctx_p, cmdId, coreId, payload, payload_size,
                                            response_buffer, sizeof(response_buffer), &nr_words_received);
    if (rc != awemgr_RC_OK) {
        IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "Sending command (cmd = %d) failed.\n", cmdId);
    }
    else
    {
        if (nr_words_received > 0) {
            idbg_print(IDBG_HDL_VAR, "bspcmd_response:\n");
            idbg_print(IDBG_HDL_VAR, "  cmd_id: %d\n", cmdId);
            idbg_print(IDBG_HDL_VAR, "  data: [\n");
            dump_response(IDBG_HDL_VAR, "    ", response_buffer, nr_words_received * sizeof(unsigned int), AWEMGR_VARTYPE_INTEGER, false, NULL);
            idbg_print(IDBG_HDL_VAR, "  ]\n");
        }
    }

    free(payload_buffer);
    free(response_buffer);

    return IDBG_OK;
}
