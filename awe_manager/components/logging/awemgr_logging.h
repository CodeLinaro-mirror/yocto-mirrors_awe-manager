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


#ifndef INCLUSION_GUARD_AWEMGR_LOGGING_H
#define INCLUSION_GUARD_AWEMGR_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

// Logging levels in AWE-Manager
#define AWEMGR_LOG_LEVEL_ERROR  0
#define AWEMGR_LOG_LEVEL_WARN   1
#define AWEMGR_LOG_LEVEL_INFO   2
#define AWEMGR_LOG_LEVEL_DEBUG  3
#define AWEMGR_LOG_LEVEL_MAX    4

#define AWEMGR_LOG_LEVEL_MASK   0x3

// Logging components in AWE-Manager
#define AWEMGR_LOG_API          0
#define AWEMGR_LOG_AWC          1
#define AWEMGR_LOG_CMD          2
#define AWEMGR_LOG_CTRL         3
#define AWEMGR_LOG_OSAL         4
#define AWEMGR_LOG_CONFIG       5
#define AWEMGR_LOG_MAX_COMP     6

#include <stdarg.h>
#include <stdbool.h>

void set_loglevel(unsigned comp, unsigned lvl);
unsigned get_loglevel(unsigned comp);

#ifndef AWEMGR_DISABLE_LOGGING

#ifdef AWEMGR_LOGGING_STDIO
#include <stdio.h>
#define TF printf
#define TF_PRIO_ERR 
#define TF_PRIO_WRN 
#define TF_PRIO_INF 
#define TF_PRIO_DBG 
#endif

#ifdef AWEMGR_LOGGING_SYSLOG
#include <syslog.h>
#define TF syslog
#define TF_PRIO_ERR LOG_ERR|LOG_USER, 
#define TF_PRIO_WRN LOG_WARNING|LOG_USER, 
#define TF_PRIO_INF LOG_INFO|LOG_USER, 
#define TF_PRIO_DBG LOG_DEBUG|LOG_USER, 
#endif

#define AWEMGR_LOGE(comp,arg,...) \
    do { \
        if (((get_loglevel(comp)) & AWEMGR_LOG_LEVEL_MASK) >= AWEMGR_LOG_LEVEL_ERROR) { \
            (void)TF(TF_PRIO_ERR "[ERROR] %25s(%3d): " arg "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define AWEMGR_LOGW(comp,arg,...) \
    do { \
        if (((get_loglevel(comp)) & AWEMGR_LOG_LEVEL_MASK) >= AWEMGR_LOG_LEVEL_WARN) { \
            (void)TF(TF_PRIO_WRN "[WARN ] %25s(%3d): " arg "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define AWEMGR_LOGI(comp,arg,...) \
    do { \
        if (((get_loglevel(comp)) & AWEMGR_LOG_LEVEL_MASK) >= AWEMGR_LOG_LEVEL_INFO) { \
            (void)TF(TF_PRIO_INF "[INFO ] %25s(%3d): " arg "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define AWEMGR_LOGD(comp,arg,...) \
    do { \
        if (((get_loglevel(comp)) & AWEMGR_LOG_LEVEL_MASK) >= AWEMGR_LOG_LEVEL_DEBUG) { \
            (void)TF(TF_PRIO_DBG "[DEBUG] %25s(%3d): " arg "\n", __func__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

// unconditional macro to "debug" dump some message; mainly used for dumping data content
#define AWEMGR_LOG(arg,...) TF(TF_PRIO_DBG "[DUMP] %25s(%3d): "  arg "\n", __func__, __LINE__, ##__VA_ARGS__)

// central routine to safely dump large amounts of data to LOG system,
// note that this method works with a callback so that calling SW can properly forward the 
// constructred message buffer to either above mentioned macros or print elsewhere

#define AWEMGR_LOG_VARTYPE_INT 0
#define AWEMGR_LOG_VARTYPE_FLOAT 1
#define AWEMGR_LOG_VARTYPE_FRACT32 2
#define AWEMGR_LOG_VARTYPE_FRACT16 3

typedef void (*print_fct_cb)(char *line, int index, void *ctx);
void awemgr_log_buffer(void *data, unsigned int nr_bytes, int var_typ, print_fct_cb cb, void *ctx);

#define NR_ITEMS_PER_ROW 8  // adjust this to specify the number of data items per printed line

#else

#define AWEMGR_LOGE(comp,arg,...)
#define AWEMGR_LOGW(comp,arg,...)
#define AWEMGR_LOGI(comp,arg,...)
#define AWEMGR_LOGD(comp,arg,...)

#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUSION_GUARD_AWEMGR_LOGGING_H
