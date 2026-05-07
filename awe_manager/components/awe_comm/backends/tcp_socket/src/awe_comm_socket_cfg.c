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


#include "awe_comm_socket_cfg.h"
#include "awe_comm_common_cfg.h"

#include "awe_comm_logging.h"
#include "awe_comm.h"

#define FAIL_ON_PTR(x) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!", #x); return AWECOMM_RC_FAIL_PARAM; }


int register_comm_backend_socket_configs(awe_config* cfg_p)
{
    FAIL_ON_PTR(cfg_p);

    int rc = AWECFG_RC_OK;

    aweconfig_init_tuple config_items[] = {
        {CFG_COMM_SOCKET_IP, DEFAULT_COMM_SOCKET_IP, "IP Address for socket based tuning/control", NULL, NULL},
        {CFG_COMM_SOCKET_PORT, DEFAULT_COMM_SOCKET_PORT, "Port for socket based tuning/control", NULL, NULL},
        {CFG_COMM_SOCKET_RECONNECT_INTERVAL_MS, DEFAULT_COMM_SOCKET_RECONNECT_INTERVAL_MS,
         "Milliseconds between automatic reconnect attempts to AWECore (-1 = disabled)", NULL, NULL},
    };

    size_t num_configs = sizeof(config_items) / sizeof(config_items[0]);

    rc = aweconfig_add_multiple(cfg_p, config_items, num_configs);
    if(rc != AWECFG_RC_OK)
        return AWECOMM_RC_FAIL_RESOURCES;

    return AWECOMM_RC_OK;
}
