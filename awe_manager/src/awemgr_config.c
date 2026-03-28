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

#include "awe_manager.h"
#include "types/awemgr_data.h"
#include "awe_ctrl.h"      // for awectrl_register_configs
#include "awe_event.h"     // for aweevent_register_configs
#include "awemgr_util.h"   // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h" // eg strlcpy, strdup etc
#include "awe_config.h"    // for aweconfig_add, aweconfig_set etc
#include <stdlib.h>        // for getenv

/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/

static int awemgr_register_configs(awe_config *cfg_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(cfg_p);

    static const struct
    {
        const char *key;
        const char *desc;
    } configItems[] = {
        {CFG_MGR_LOGLEVEL,  "API log level options:    { error | warning | info | debug }"},
        {CFG_AWC_LOGLEVEL,  "AWC log level options:    { error | warning | info | debug }"},
        {CFG_CMD_LOGLEVEL,  "CMD log level options:    { error | warning | info | debug }"},
        {CFG_CTRL_LOGLEVEL, "CTRL log level options:   { error | warning | info | debug }"},
        {CFG_OSAL_LOGLEVEL, "OSAL log level options:   { error | warning | info | debug }"},
        {CFG_CFG_LOGLEVEL,  "CONFIG log level options: { error | warning | info | debug }"}};

    int numItems = sizeof(configItems) / sizeof(configItems[0]);
    for (int i = 0; i < numItems; i++)
    {
        int rc = aweconfig_add(cfg_p, configItems[i].key, configItems[i].desc, LOG_LEVEL_INFO);
        if (rc != AWECFG_RC_OK)
            return awemgr_RC_ERR;
    }
    if (awectrl_register_configs(cfg_p) != AWECTRL_RC_OK)
        return awemgr_RC_ERR;
    if (aweevent_register_configs(cfg_p) != AWECTRL_RC_OK)
        return awemgr_RC_ERR;

    // Although config is initialized already, awemgr_config_set ensures the log level change is applied.
    int numKeys = sizeof(configItems) / sizeof(configItems[0]);
    for (int i = 0; i < numKeys; i++)
    {
        int rc = awemgr_config_set(cfg_p, configItems[i].key, LOG_LEVEL_INFO);
        if (rc != AWECFG_RC_OK)
            return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
}


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

// Helper function to map a configuration key to a component.
static int get_logging_component_from_key(const char *key)
{
    if (strcmp(key, CFG_MGR_LOGLEVEL) == 0)
        return AWEMGR_LOG_API;
    if (strcmp(key, CFG_AWC_LOGLEVEL) == 0)
        return AWEMGR_LOG_AWC;
    if (strcmp(key, CFG_CMD_LOGLEVEL) == 0)
        return AWEMGR_LOG_CMD;
    if (strcmp(key, CFG_CTRL_LOGLEVEL) == 0)
        return AWEMGR_LOG_CTRL;
    if (strcmp(key, CFG_OSAL_LOGLEVEL) == 0)
        return AWEMGR_LOG_OSAL;
    if (strcmp(key, CFG_CFG_LOGLEVEL) == 0)
        return AWEMGR_LOG_CONFIG;
    return -1;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/
enum awemgr_rc awemgr_config_create(awe_config **cfg_pp)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(cfg_pp);

    // config object not provided, create internally
    int rc = aweconfig_create(cfg_pp);
    if (rc == AWECFG_RC_FAIL)
    {
        AWEMGR_API_LOGE("Failed to create awe_config object");
        return awemgr_RC_ERR;
    }
    rc = awemgr_register_configs(*cfg_pp);
    if (rc != awemgr_RC_OK)
    {
        AWEMGR_API_LOGE("Failed to register configurations");
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
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
    int loglevel = get_loglevel_from_str(value);
    if (loglevel != -1)
    {
        int component = get_logging_component_from_key(key);
        if (component != -1)
        {
            set_loglevel(component, loglevel);
        }
    }
    return aweconfig_set(cfg_p, key, value);
}

const char *awemgr_config_get(struct awe_config *cfg_p, const char *key, const char **description)
{
    return aweconfig_get(cfg_p, key, description);
}
