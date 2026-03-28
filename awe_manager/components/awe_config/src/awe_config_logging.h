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


#ifndef INCLUSION_GUARD_AWE_CONFIG_LOGGING_H
#define INCLUSION_GUARD_AWE_CONFIG_LOGGING_H

#include "awemgr_logging.h"
/* LCOV_EXCL_START */
#define AWE_CFG_LOGE(arg,...)   AWEMGR_LOGE(AWEMGR_LOG_CONFIG,arg,##__VA_ARGS__)
#define AWE_CFG_LOGW(arg,...)   AWEMGR_LOGW(AWEMGR_LOG_CONFIG,arg,##__VA_ARGS__)
#define AWE_CFG_LOGI(arg,...)   AWEMGR_LOGI(AWEMGR_LOG_CONFIG,arg,##__VA_ARGS__)
#define AWE_CFG_LOGD(arg,...)   AWEMGR_LOGD(AWEMGR_LOG_CONFIG,arg,##__VA_ARGS__)
/* LCOV_EXCL_END */

#endif // INCLUSION_GUARD_AWE_CONFIG_LOGGING_H