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
 * IDbg commands for the DTMF generator signal flow
 * 
 * The DTMF genrator signal flow is provided as a test-design to show various
 * control methods (writing/reading/toggling).
 * 
 * ***************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>

#include "app_ctx.h"  // own application related types and interactive cmdline 
#include "idbg.h"     // IDBG_GET_STRING et al
#include "awe_manager.h" // the AWE Manager include

#include "cmds_signalflow_dtmf.h" // include own hdr file to avoid undefined references to table

// ******************************************************************************************************

/**
 * sets row and/or column frequency values
 */
static int ctrl_set_freq (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    char *col_freq = IDBG_GET_STRING("-col", NULL, ARG_NEEDED);
    char *row_freq = IDBG_GET_STRING("-row", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) 
    {
        IDBG_CMDUSAGE ((p, "-col <freq> -row <freq>",
                        "-col <freq>", "Frequency of the column generator",
                        "-row <freq>", "Frequency of the row generator",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (col_freq) 
    {
        float col_freq_f;
        sscanf(col_freq, "%f", &col_freq_f);
        idbg_print(p, "  col freq: %f\n", col_freq_f);

        int rc = awemgr_control_write(awc_ctx_p, "Sine_Column/freq", 0, (void*) &col_freq_f, sizeof(float) * 1, AWEMGR_VARTYPE_FLOAT); 
    }

    if (row_freq) 
    {
        float row_freq_f;
        sscanf(row_freq, "%f", &row_freq_f);
        idbg_print(p, "  row freq: %f\n", row_freq_f);

        int rc = awemgr_control_write(awc_ctx_p, "Sine_Row/freq", 0, (void*) &row_freq_f, sizeof(float) * 1, AWEMGR_VARTYPE_FLOAT); 
    }

    return IDBG_OK;
}

/**
 * explicitely set the volume control module
 */
static int ctrl_set_volume (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    char *gain_val = IDBG_GET_STRING("-gain", NULL, ARG_NEEDED);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) 
    {
        IDBG_CMDUSAGE ((p, "-gain <value>",
                        "-gain <value>", "Master gain dB value",
        				NULL, NULL));
        return IDBG_OK;
    }

    float gain_f;
    sscanf(gain_val, "%f", &gain_f);
    idbg_print(p, "  new gain: %f\n", gain_f);

    int rc = awemgr_control_write(awc_ctx_p, "sys_volume/masterGain", 0, (void*) &gain_f, sizeof(float) * 1, AWEMGR_VARTYPE_FLOAT); 

    return IDBG_OK;
}

/**
 * toggle the state of the mute module; enable or disable audio output
 */
static int ctrl_awc_mute (IDBG_PARAMS)
{
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(IDBG_HDL_VAR);
    struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(appCtx_p->mgr_p, appCtx_p->endpointId);

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR) 
    {
        IDBG_CMDUSAGE ((p, "(switches mute module on/off)",
        				NULL, NULL));
        return IDBG_OK;
    }

    if (!awc_ctx_p) 
    {
        idbg_print(p, "No AWC loaded!\n");
    }
    else 
    {
        unsigned int response_buffer[200];  // todo: fix this! 

        int rc = awemgr_control_read(awc_ctx_p, "FullMute/isMuted", response_buffer, 200, NULL, NULL);

        // todo: check if explicit is really so much leaner!
        // int rc = awemgr_get_variable_by_name(awc_ctx_p, "FullMute/isMuted", &var);
        // rc = awemgr_variable_read(awc_ctx_p, var, 0, response_buffer, sizeof(response_buffer));
        idbg_print(p, " currently mute state -> %d\n", response_buffer[0]);

        response_buffer[0] = 1 - response_buffer[0];
        rc = awemgr_control_write(awc_ctx_p, "FullMute/isMuted", 0, (void*) response_buffer, sizeof(unsigned int) * 1, AWEMGR_VARTYPE_INTEGER); 
        // todo: error checks
        
        rc = awemgr_control_read(awc_ctx_p, "FullMute/isMuted", response_buffer, 200, NULL, NULL); 
        idbg_print(p, " updated mute state -> %d\n", response_buffer[0]);
    }
    return IDBG_OK;
}

IDBG_TBL_START(MenuDTMF_SF_Control)
    IDBG_TBL_CMD(ctrl_set_volume, "vol", NULL)
    IDBG_TBL_CMD(ctrl_set_freq, "freq", NULL)
    IDBG_TBL_CMD(ctrl_awc_mute, "toggle_mute", NULL)
IDBG_TBL_END

