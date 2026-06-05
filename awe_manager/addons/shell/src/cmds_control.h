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


#ifndef INCLUSION_GUARD_CMDS_CONTROL_H
#define INCLUSION_GUARD_CMDS_CONTROL_H

#include "idbg.h"

/** generic reading a control value of AWC */
int ctrl_generic_get (IDBG_PARAMS);

/** generic writing a control value (or values) */
int ctrl_generic_set (IDBG_PARAMS);

/** transmit an AWE tuning command and get response */
int ctrl_transact (IDBG_PARAMS);

/** loop over all controllable items */
int ctrl_enumerate (IDBG_PARAMS);

int module_generic(IDBG_PARAMS);

#endif //INCLUSION_GUARD_CMDS_CONTROL_H
