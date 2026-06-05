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
 * @file awemgr_tuning_server.h
 * @brief AWE Manager tuning server — public C API
 *
 * Accepts AWE tuning packets over a TCP socket, forwards them to
 * awemgr_transact(), and returns the AWECore response to the client.
 *
 * Typical usage in a service main loop:
 *
 *   awemgr_tuning_server *srv = awemgr_tuning_server_create(mgr_p, cfg_p);
 *   while (!stop_requested)
 *       awemgr_tuning_server_run(srv, "7200");
 *   awemgr_tuning_server_destroy(srv);
 *
 * Each call to awemgr_tuning_server_run() blocks until the connected client
 * disconnects, then returns so the caller can re-enter the listen/accept cycle.
 * A SIGTERM or SIGINT received while blocked in accept() causes the call to
 * return 0 immediately (EINTR), allowing the loop condition to be re-evaluated.
 *
 * AWE tuning packet framing
 * -------------------------
 * Both request and response packets use the AWE wire format: the high 16 bits
 * of the first 32-bit word encode the total packet length in words (including
 * the header word itself).  The server reads exactly that many words from the
 * socket, passes them to awemgr_transact(), and writes the response words back.
 */

#ifndef INCLUSION_GUARD_AWEMGR_TUNING_SERVER_H
#define INCLUSION_GUARD_AWEMGR_TUNING_SERVER_H

#include "awe_manager.h"
#include "awe_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque handle to a tuning server context. */
typedef struct awemgr_tuning_server awemgr_tuning_server;

/**
 * Register tuning server configuration keys with their defaults.
 *
 * Call this before awemgr_init() so that user overrides (environment variable,
 * config string) are applied on top of the defaults.
 *
 * Registered keys:
 *   - mgr.tuning_server.buf_words   Size of the AWE tuning packet buffer in
 *                                   32-bit words (default: 4096).
 *   - mgr.tuning_server.timeoutms   Client socket connection timeout in
 *                                   milliseconds; -1 = wait indefinitely
 *                                   (default: -1, reserved for future use).
 *
 * @param cfg_p  AWE Manager configuration object.  Must not be NULL.
 * @return  0 on success, negative value on error.
 */
int awemgr_tuning_server_register_configs(awe_config *cfg_p);

/**
 * Create a tuning server context.
 *
 * @param mgr_p  Initialised AWE Manager handle.  Must not be NULL.
 * @param cfg_p  AWE Manager configuration object, or NULL.
 *               When non-NULL the buffer size is read from
 *               mgr.tuning_server.buf_words at creation time.
 *               The caller retains ownership — the config will not be freed
 *               by awemgr_tuning_server_destroy().
 * @return  New context on success, NULL on allocation failure or invalid arg.
 */
awemgr_tuning_server *awemgr_tuning_server_create(struct awemgr_data *mgr_p,
                                                   awe_config *cfg_p);

/**
 * Destroy a tuning server context and free all resources.
 * After this call @p srv must not be used.
 */
void awemgr_tuning_server_destroy(awemgr_tuning_server *srv);

/**
 * Unblock a running awemgr_tuning_server_run() call from another thread.
 *
 * Closes the listening and/or connected client socket so that any blocking
 * si_accept() or recv call returns immediately.  The run() call will then
 * return and the caller's loop can check its stop condition.
 * Safe to call concurrently with awemgr_tuning_server_run().
 * Has no effect if run() is not currently active.
 */
void awemgr_tuning_server_stop(awemgr_tuning_server *srv);

/**
 * Listen on @p port, accept **one** client, process AWE tuning packets
 * until the client disconnects, then close the listening socket and return.
 *
 * Note that really only one client is supported at a time.
 * This method must not be called with the same port argument concurrently from multiple threads!
 *
 * @param srv   Tuning server context.
 * @param port  TCP port number as a string (e.g. "7200").
 *
 * @return  0 on clean client disconnect or signal interruption (EINTR).
 *         -1 on a fatal error that prevents the server from starting
 *            (e.g. bind or listen failure).
 */
int awemgr_tuning_server_run(awemgr_tuning_server *srv,
                             const char *port);

#ifdef __cplusplus
}
#endif

#endif /* INCLUSION_GUARD_AWEMGR_TUNING_SERVER_H */
