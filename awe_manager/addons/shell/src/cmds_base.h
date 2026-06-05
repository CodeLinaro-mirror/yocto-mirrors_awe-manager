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


#ifndef INCLUSION_GUARD_CMDS_BASE_H
#define INCLUSION_GUARD_CMDS_BASE_H

#include "idbg.h"

#ifdef _MSC_VER
#define strtok_r strtok_s
#endif

/** initializing AWE Manager */
int amgr_init (IDBG_PARAMS);

/** freeing up resources of AWE Manager from system */
int amgr_exit (IDBG_PARAMS);

/** handle config object */
int amgr_config (IDBG_PARAMS);

/** loading a new AWC so that control become available */
int awc_load(IDBG_PARAMS);

/** unloading AWC, no controls available anymore */
int awc_unload(IDBG_PARAMS);

/** loads and applies an AWB (either main or preset) */
int design_load(IDBG_PARAMS);

/** stops audio processing and unloads design */
int design_unload(IDBG_PARAMS);

/** stops audio processing */
int audio_stop(IDBG_PARAMS);

/** starts audio processing */
int audio_start(IDBG_PARAMS);

/** shows the designs the currently selected AWC supports */
int design_enumerate (IDBG_PARAMS);

/** configures the communication to AWECore (especially, in case system runs with socket link) */
int sys_comm(IDBG_PARAMS);

/** enables/disabled tracing of tune CMDs to AWECore on console or file */
int sys_comm_trace(IDBG_PARAMS);

/** sets/or queries the current AWC endpoint/index */
int awc_select(IDBG_PARAMS);

/** loading and executing another script file */
int script(IDBG_PARAMS);

/** enable/disable time commands */
int sys_time_commands(IDBG_PARAMS);

#endif //INCLUSION_GUARD_CMDS_BASE_H
