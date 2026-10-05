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

/**
 * Scope guard around idbg_output_hold() / idbg_output_flush().
 *
 * Declare one at the top of a command handler that issues several AWE Manager
 * API calls while printing its output.  All idbg_print() output of the command
 * is then collected and emitted as one contiguous block when the guard goes
 * out of scope - on every return path.  Output produced from inside the API
 * calls, i.e. the comm-trace tap (see comm_observer_cb()), bypasses the hold
 * and keeps streaming, so it can no longer break up the command's YAML.
 *
 * Example:
 *   static void my_cmd_impl(idbg_t *p, ...)
 *   {
 *       IdbgOutputHold hold(p);
 *       idbg_print(p, "my_cmd:\n");
 *       ...
 *   }
 */
class IdbgOutputHold
{
public:
    explicit IdbgOutputHold(idbg_t *p) : m_p(p) { (void) idbg_output_hold(m_p); }
    ~IdbgOutputHold() { (void) idbg_output_flush(m_p); }

    IdbgOutputHold(const IdbgOutputHold &) = delete;
    IdbgOutputHold &operator=(const IdbgOutputHold &) = delete;

private:
    idbg_t *m_p;
};

/**
 * Scope guard around idbg_output_disable() / idbg_output_enable().
 *
 * Declare one around the part of a command handler whose idbg_print() output
 * shall not reach the console or socket, e.g. around the execution of a
 * command a handler runs on behalf of the user.  The output sink in use is
 * put back when the guard goes out of scope - on every return path.  Passing
 * *drop* as false leaves the output untouched, so an option such as
 * "repeat -verbose" does not need a second code path.
 *
 * Example:
 *   static void my_cmd_impl(idbg_t *p, bool verbose, ...)
 *   {
 *       {
 *           IdbgOutputDrop drop(p, !verbose);
 *           run_something(p);          // its output is dropped
 *       }
 *       idbg_print(p, "my_cmd:\n");    // the summary is printed again
 *   }
 */
class IdbgOutputDrop
{
public:
    explicit IdbgOutputDrop(idbg_t *p, bool drop) : m_p(p), m_drop(drop) {
        if (m_drop) {
            (void) idbg_output_disable(m_p);
        }
    }
    ~IdbgOutputDrop() {
        if (m_drop) {
            (void) idbg_output_enable(m_p);
        }
    }

    IdbgOutputDrop(const IdbgOutputDrop &) = delete;
    IdbgOutputDrop &operator=(const IdbgOutputDrop &) = delete;

private:
    idbg_t *m_p;
    bool m_drop;
};

/**
 * Scope guard which counts the active invocations of a command handler that
 * dispatches further command lines on the same idbg handle.
 *
 * "repeat" and "script" both run other commands through idbg_parse_cmd(), so
 * they can end up invoking themselves - directly, or through each other.  A
 * script file which includes itself recurses until the stack is exhausted, and
 * a "repeat" inside a "repeat" makes the call count and the rate reported by
 * the outer one meaningless.  Declare one guard in every such handler and
 * leave the command when allowed() returns false.  The counter is decremented
 * again when the guard goes out of scope - on every return path.
 *
 * *counter_p* is the nesting counter of the command, *max_depth* the number of
 * invocations that may be active at the same time.  A NULL *counter_p*
 * disables the check, so a handler keeps working on an idbg handle that
 * carries no shell context.
 *
 * Example:
 *   CmdNestingGuard nesting(&appCtx_p->script_nesting, MAX_SCRIPT_NESTING);
 *   if (!nesting.allowed())
 *   {
 *       IDBG_PRINT_ERR(p, "too deeply nested\n");
 *       return IDBG_OK;
 *   }
 */
class CmdNestingGuard
{
public:
    CmdNestingGuard(int *counter_p, int max_depth) : m_counter_p(counter_p)
    {
        if (m_counter_p != NULL)
        {
            m_allowed = (*m_counter_p < max_depth);
            if (m_allowed)
            {
                (*m_counter_p)++;
            }
        }
    }
    ~CmdNestingGuard()
    {
        if ((m_counter_p != NULL) && m_allowed)
        {
            (*m_counter_p)--;
        }
    }

    /** false when the command must not run because it is nested too deeply */
    bool allowed() const { return m_allowed; }

    CmdNestingGuard(const CmdNestingGuard &) = delete;
    CmdNestingGuard &operator=(const CmdNestingGuard &) = delete;

private:
    int *m_counter_p;
    bool m_allowed{true};
};

#endif // INCLUSION_GUARD_HLP_FUNCTIONS_H
