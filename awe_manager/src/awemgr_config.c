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

#include "awe_manager.h"
#include "types/awemgr_data.h"
#include "awe_comm.h"      // for awecomm_register_configs
#include "awe_event.h"     // for aweevent_register_configs
#include "awemgr_util.h"   // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h" // eg strlcpy, strdup etc
#include "awe_config.h"    // for aweconfig_add, aweconfig_set etc
#include <stdlib.h>        // for getenv
#include <stdint.h>        // for intptr_t

/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/

// Helper function to convert a log level string to its numeric value.
static int get_loglevel_from_str(const char *value)
{
    if (strcmp(value, LOG_LEVEL_ERROR) == 0)
        return AWEMGR_LOG_LEVEL_ERROR;
    if (strcmp(value, LOG_LEVEL_WARN) == 0)
        return AWEMGR_LOG_LEVEL_WARN;
    if (strcmp(value, LOG_LEVEL_DEBUG) == 0)
        return AWEMGR_LOG_LEVEL_DEBUG;
    if (strcmp(value, LOG_LEVEL_INFO) == 0)
        return AWEMGR_LOG_LEVEL_INFO;
    return -1;
}

// callback for all log level configurations
static void LogLevelCb(const char* key, const char* value, const char* description, void* context)
{
    // Retrieve the component that was passed in via the context pointer
    int component = (int)(intptr_t)context;
    int loglevel = get_loglevel_from_str(value);

    set_loglevel(component, loglevel);
}

static int awemgr_register_configs(awe_config *cfg_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(cfg_p);

    aweconfig_init_tuple component_configs[] = {
        {CFG_MGR_LOGLEVEL, LOG_LEVEL_INFO, "API log level options (error/warning/info/debug)", LogLevelCb, (void*)(intptr_t)AWEMGR_LOG_API},
        {CFG_AWC_LOGLEVEL, LOG_LEVEL_INFO, "AWC log level options (error/warning/info/debug)", LogLevelCb, (void*)(intptr_t)AWEMGR_LOG_AWC},
        {CFG_CMD_LOGLEVEL, LOG_LEVEL_INFO, "CMD log level options (error/warning/info/debug)", LogLevelCb, (void*)(intptr_t)AWEMGR_LOG_CMD},
        {CFG_COMM_LOGLEVEL, LOG_LEVEL_INFO, "COMM log level options (error/warning/info/debug)", LogLevelCb, (void*)(intptr_t)AWEMGR_LOG_COMM},
        {CFG_OSAL_LOGLEVEL, LOG_LEVEL_INFO, "OSAL log level options (error/warning/info/debug)", LogLevelCb, (void*)(intptr_t)AWEMGR_LOG_OSAL},
        {CFG_CFG_LOGLEVEL, LOG_LEVEL_INFO, "CONFIG log level options (error/warning/info/debug)", LogLevelCb, (void*)(intptr_t)AWEMGR_LOG_CONFIG},

        {CFG_MGR_RANGECHECK_ENABLED, CFG_VAL_TRUE, "API range check enabled (on/off)", NULL, NULL}
    };

    size_t num_configs = sizeof(component_configs) / sizeof(component_configs[0]);
    int rc = aweconfig_add_multiple(cfg_p, component_configs, num_configs);
    if (rc != AWECFG_RC_OK)
        return awemgr_RC_ERR;

    if (awecomm_register_configs(cfg_p) != AWECOMM_RC_OK)
        return awemgr_RC_ERR;
    if (aweevent_trace_register_configs(cfg_p) != AWECOMM_RC_OK)  // special handling for evt traces for now
        return awemgr_RC_ERR;
    if (aweevent_register_configs(cfg_p) != AWECOMM_RC_OK)
        return awemgr_RC_ERR;

    return awemgr_RC_OK;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/
enum awemgr_rc awemgr_config_create(awe_config **cfg_pp)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(cfg_pp);

    // config object not provided, create internally
    int cfg_rc = aweconfig_create(cfg_pp);
    if (cfg_rc == AWECFG_RC_FAIL)
    {
        AWEMGR_API_LOGE("Failed to create awe_config object");
        return awemgr_RC_ERR;
    }
    enum awemgr_rc rc = awemgr_register_configs(*cfg_pp);
    if (rc != awemgr_RC_OK)
    {
        AWEMGR_API_LOGE("Failed to register configurations");
        return awemgr_RC_ERR;
    }
    return rc;
}

enum awemgr_rc awemgr_config_destroy(awe_config **cfg_pp)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(cfg_pp);
    if (*cfg_pp == NULL)
    {
        AWEMGR_API_LOGW("AWE Config already uninitialized(NULL)");
        return awemgr_RC_OK;
    }
    aweconfig_destroy(cfg_pp);
    *cfg_pp = NULL;
    return awemgr_RC_OK;
}

int awemgr_config_set(struct awe_config *cfg_p, const char *key, const char *value)
{
    return aweconfig_set(cfg_p, key, value);
}

const char *awemgr_config_get(struct awe_config *cfg_p, const char *key, const char **description)
{
    return aweconfig_get(cfg_p, key, description);
}
