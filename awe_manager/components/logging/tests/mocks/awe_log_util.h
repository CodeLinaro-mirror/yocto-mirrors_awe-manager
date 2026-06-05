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
/* Mock implementation of AWE-Q's awe_log_util.h for unit testing.
**
** Placed on the include path BEFORE any system directories so the preprocessor
** finds this file instead of the real customer header when
** -DAWEMGR_LOGGING_AWEQ is active.
**
** All calls to AUDIO_BASE_LOG() are forwarded to mock_audio_base_log_impl()
** which is defined in the test source and records arguments for assertions.
*/

#pragma once

#ifdef __cplusplus
#include <cstdarg>
extern "C" {
#else
#include <stdarg.h>
#endif

/* Log level constants - must match what awemgr_logging.h passes to AUDIO_BASE_LOG */
#define BASE_LOG_ERROR  0
#define BASE_LOG_INFO   1
#define BASE_LOG_DEBUG  2

/* Module identifier used in every AWEMGR log call */
#define MODULE_AWE_MGR  99

/* Implemented in test_logging_aweq.cpp - records calls for test assertions */
void mock_audio_base_log_impl(int level, int module, const char *format, ...);

#define AUDIO_BASE_LOG(level, module, format, ...) \
    mock_audio_base_log_impl((level), (module), (format), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
