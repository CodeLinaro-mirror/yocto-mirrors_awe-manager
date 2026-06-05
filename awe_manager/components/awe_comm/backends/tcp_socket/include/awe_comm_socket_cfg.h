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

#ifndef INCLUSION_GUARD_AWE_COMM_SOCKET_CFG_H
#define INCLUSION_GUARD_AWE_COMM_SOCKET_CFG_H

#include "awe_config.h"

#define CFG_COMM_SOCKET_IP              "mgr.comm.socket.ip"
#define DEFAULT_COMM_SOCKET_IP          "127.0.0.1"

#define CFG_COMM_SOCKET_PORT            "mgr.comm.socket.port"
#define DEFAULT_COMM_SOCKET_PORT        "15002"

/* Minimum time in milliseconds between automatic reconnect attempts when
 * AWECore is unreachable.  Set to -1 to disable automatic reconnection. */
#define CFG_COMM_SOCKET_RECONNECT_INTERVAL_MS     "mgr.comm.socket.reconnect_interval_ms"
#define DEFAULT_COMM_SOCKET_RECONNECT_INTERVAL_MS "1000"

#ifdef __cplusplus
extern "C" {
#endif

int register_comm_backend_socket_configs(awe_config* cfg_p);

#ifdef __cplusplus
}
#endif

#endif // INCLUSION_GUARD_AWE_COMM_SOCKET_CFG_H
