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

/**
 * @file awemgr_shell.h
 * @brief AWE Manager shell library — public C API
 *
 * Provides a command-dispatch engine built on top of the idbg framework.
 * Callers create a shell context, optionally with a pre-initialised
 * awemgr_data handle, and then drive it via awemgr_shell_execute() or one
 * of the interactive run helpers.
 *
 * The library is intentionally optional: production builds may omit it
 * by setting AWEMGR_BUILD_SHELL=OFF at CMake configure time.
 */

#ifndef INCLUSION_GUARD_AWEMGR_SHELL_H
#define INCLUSION_GUARD_AWEMGR_SHELL_H

#include "awe_manager.h"
#include "awe_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle to a shell context. */
typedef struct awemgr_shell_ctx awemgr_shell_ctx;

/**
 * Register shell configuration keys with their defaults.
 *
 * Call this before awemgr_init() so that user overrides (environment variable,
 * config string) are applied on top of the defaults.
 *
 * Registered keys:
 *   - mgr.shell.input_buf_chars    Maximum byte length of a single command
 *                                  string passed to awemgr_shell_execute()
 *                                  (default: 4096).
 *   - mgr.shell.socket.timeoutms   Socket session timeout in milliseconds;
 *                                  -1 = wait indefinitely
 *                                  (default: -1, reserved for future use).
 *
 * @param cfg_p  AWE Manager configuration object.  Must not be NULL.
 * @return  0 on success, negative value on error.
 */
int awemgr_shell_register_configs(awe_config *cfg_p);

/**
 * Create a shell context.
 *
 * @param mgr_p  Pre-initialised AWE Manager handle, or NULL.
 *               When NULL the shell starts without a manager; the user can
 *               initialise one later with the "mgr_init" shell command.
 * @param cfg_p  AWE Manager configuration object, or NULL.
 *               When NULL the shell allocates and owns its own configuration
 *               with default log levels set to WARN.
 *               When non-NULL the caller retains ownership of the object —
 *               it will not be freed by awemgr_shell_destroy().
 * @return  New context on success, NULL on allocation failure.
 */
awemgr_shell_ctx *awemgr_shell_create(struct awemgr_data *mgr_p, awe_config *cfg_p);

/**
 * Destroy a shell context and release all resources owned by it.
 * After this call @p ctx must not be used.
 */
void awemgr_shell_destroy(awemgr_shell_ctx *ctx);

/**
 * Execute a single shell command string (printf-style).
 *
 * @param ctx  Shell context.
 * @param fmt  printf-style format string, e.g. "/load_awc -awc %s", path.
 * @return  IDBG_OK (0) on success, non-zero on error or IDBG_STOP.
 */
int awemgr_shell_execute(awemgr_shell_ctx *ctx, const char *fmt, ...);

/**
 * Run an interactive console session (reads from stdin).
 * Blocks until the user types "exit" or EOF.
 */
int awemgr_shell_run_console(awemgr_shell_ctx *ctx);

/**
 * Execute all commands in a script file.
 */
int awemgr_shell_run_file(awemgr_shell_ctx *ctx, const char *filename);

/**
 * Accept commands over a TCP socket on the given host:port.
 * Blocks until the remote side disconnects.
 */
int awemgr_shell_run_socket(awemgr_shell_ctx *ctx, const char *host, const char *port);

/**
 * Unblock a running awemgr_shell_run_socket() call from another thread.
 *
 * Closes the listening and/or connected client socket so that any blocking
 * accept or read returns immediately.  The run_socket() call will then return
 * and the caller's loop can check its stop condition.
 * Safe to call concurrently with awemgr_shell_run_socket().
 */
void awemgr_shell_stop(awemgr_shell_ctx *ctx);

/**
 * Return the AWE Manager handle stored in the context.
 * May be NULL if "mgr_init" has not been called yet.
 */
struct awemgr_data *awemgr_shell_get_mgr(const awemgr_shell_ctx *ctx);

/**
 * Return the currently selected endpoint/canvas index (-1 = none selected).
 */
int awemgr_shell_get_endpoint_id(const awemgr_shell_ctx *ctx);

/**
 * Set the active endpoint (canvas) index used by shell commands.
 *
 * Call this after awemgr_shell_create() when the AWC was loaded externally
 * (i.e. not via the "load_awc" shell command) so that commands such as
 * "show -designs" know which canvas to query.
 *
 * @param endpoint_id  Zero-based endpoint index (0–15).
 */
void awemgr_shell_set_endpoint_id(awemgr_shell_ctx *ctx, int endpoint_id);

/**
 * Enable or disable per-command timing output.
 * @param enable  Non-zero to enable, 0 to disable.
 */
void awemgr_shell_set_time_commands(awemgr_shell_ctx *ctx, int enable);

#ifdef __cplusplus
}
#endif

#endif /* INCLUSION_GUARD_AWEMGR_SHELL_H */
