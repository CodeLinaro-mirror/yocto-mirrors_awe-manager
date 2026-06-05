/* MIT License
**
** Copyright (c) 2025 DSP Concepts, Inc.
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


#ifndef INCLUSION_GUARD_HLP_FUNCTIONS_H
#define INCLUSION_GUARD_HLP_FUNCTIONS_H

#include "idbg.h"
#include <assert.h>


/**
 * Print an error message to the idbg handle, automatically prefixed with
 * "error: " so that shell clients (e.g. awemgr_client.py) can detect and
 * render it in red.  This macro also checks if there is an AWECore error available from the
 * AWE Manager context and if so, it prints the AWECore error code and description as well.  The format string must end with "\n".
 * Use this macro instead of bare idbg_print() for all failure paths related to AWECore operations, as it provides more detailed error information when available.
 *
 * Example:
 *   IDBG_PRINT_ERR_AWECORE(IDBG_HDL_VAR, "could not load design '%s'\n", name);
 * → prints something like:  error: could not load design 'Main'
 *                           error_awecore:
 *                             code: 123456
 *                             description: "Invalid design file"
 */
#define IDBG_PRINT_ERR_AWECORE(p, fmt, ...) {\
    idbg_print((p), "error: " fmt, ##__VA_ARGS__); \
    int _awe_err_code = awemgr_get_awe_error_code(); \
    if (_awe_err_code != 0) { \
         idbg_print((p), "error_awecore:\n  code: %d\n  description: %s\n", _awe_err_code, awemgr_get_awe_error_string()); \
    } \
}

#endif // INCLUSION_GUARD_HLP_FUNCTIONS_H
