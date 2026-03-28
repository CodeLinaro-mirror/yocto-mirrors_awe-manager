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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "awemgr_api_logging.h"
#include "awemgr_util.h"
#include "awe_ctrl.h"

#include "awe_config.h"

/* ****************************************************************************
 * PRIVATE VARIABLES (STATIC)
 * ***************************************************************************/
#include "versioninfo.h"
static const char *awemgr_version = AWEMANAGER_VERSION;


/* ****************************************************************************
 * TYPE DEFINITIONS (NON PUBLIC)
 * ***************************************************************************/
#include "types/awemgr_data.h"

/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/
static void printConfig(const char* key, const char* value, const char* description, void* context)
{
    AWEMGR_API_LOGD("%-30s%-20s%s",key, value, description);
}


/* ****************************************************************************
 * PUBLIC FUNCTIONS - VERSION
 * ***************************************************************************/
const char *awemgr_get_version()
{
    return awemgr_version;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS - INIT/EXIT
 * ***************************************************************************/

enum awemgr_rc awemgr_init(awe_config** cfg_pp, struct awemgr_data **mgr_pp)
{
    if (mgr_pp == NULL)
    {
        AWEMGR_API_LOGE("Invalid argument: mgr_pp == NULL!");
        return awemgr_RC_ERR;  // todo: decent error code: invalid param
    }

    if (*mgr_pp != NULL)
    {
        AWEMGR_API_LOGE("Invalid argument: Either AWE Manager already initialized or *mgr_pp not initialized!");
        return awemgr_RC_ERR;  // todo: decent error code: already initialized!
    }

    // alloc internal own data structure
    struct awemgr_data *mgr_p = calloc(1, sizeof(struct awemgr_data));
    if (mgr_p == NULL)
    {
        AWEMGR_API_LOGE("Memory allocation failed");
        *mgr_pp = NULL;
        return awemgr_RC_ERR;
    }

    if (cfg_pp == NULL)
    {
        // config object not provided, create internally
        if(awemgr_config_create(&mgr_p->config) != awemgr_RC_OK)
        {
            AWEMGR_API_LOGE("Failed to create awe_config object");
            free(mgr_p); // free memory before returning
            return awemgr_RC_ERR;
        }
        mgr_p->config_internally_allocated = true;
    }
    else
    {
        AWEMGR_FAIL_ON_HANDLE_NULL(*cfg_pp);
        mgr_p->config = *cfg_pp;
        mgr_p->config_internally_allocated = false;
    }

    if (aweconfig_from_envvar(mgr_p->config, AWEMGR_CFG_OVERRIDE) != awemgr_RC_OK)
    {
        AWEMGR_API_LOGE("Failed to apply configuration from environment variable");
    }

    AWEMGR_API_LOGI("MGR Log configuration: %s", aweconfig_get(mgr_p->config, CFG_MGR_LOGLEVEL, NULL));
    AWEMGR_API_LOGD("AWC Log configuration: %s", aweconfig_get(mgr_p->config, CFG_AWC_LOGLEVEL, NULL));
    AWEMGR_API_LOGD("CMD Log configuration: %s", aweconfig_get(mgr_p->config, CFG_CMD_LOGLEVEL, NULL));
    AWEMGR_API_LOGD("OSAL Log configuration: %s", aweconfig_get(mgr_p->config, CFG_OSAL_LOGLEVEL, NULL));
    AWEMGR_API_LOGD("CTRL Log configuration: %s", aweconfig_get(mgr_p->config, CFG_CTRL_LOGLEVEL, NULL));
    AWEMGR_API_LOGD("CFG Log configuration: %s", aweconfig_get(mgr_p->config, CFG_CFG_LOGLEVEL, NULL));

    // initialize the communication to AWE HostTask (ADSP)
    int rc = awectrl_init(mgr_p->config, NULL, &mgr_p->comm_2_awe);
    if(rc != AWECTRL_RC_OK)
    {
        AWEMGR_API_LOGE("ctrl init failed");

        if(mgr_p->config_internally_allocated)
        {
            aweconfig_destroy(&mgr_p->config);
        }
        free(mgr_p); // free memory before returning
        return awemgr_RC_ERR;
    }

    AWEMGR_API_LOGD("AWE Manager initialized: version=%s", awemgr_version);
    AWEMGR_API_LOGD("Running with following Configurations:");
    AWEMGR_API_LOGD("%-30s%-20s%s", "Key", "Value", "Description");
    AWEMGR_API_LOGD("%-30s%-20s%s", "------------------------------", "--------------------", "------------------------------");

    aweconfig_foreach_item(mgr_p->config, printConfig, NULL);
    *mgr_pp = mgr_p;
    return awemgr_RC_OK;
}


enum awemgr_rc awemgr_exit(struct awemgr_data **mgr_pp)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(mgr_pp);
    if (*mgr_pp == NULL)
    {
        AWEMGR_API_LOGW("AWE Manager already uninitialized(NULL)");
        return awemgr_RC_OK;
    }
    struct awemgr_data *mgr_p = *mgr_pp;
    // release memory of all AWC handlers - if any!
    for (int idx = 0; idx < MAX_AWE_ENDPOINTS; idx++)
    {
        (void)awemgr_unload_awc(mgr_p, idx);
    }

    awectrl_exit(&mgr_p->comm_2_awe);
    if(mgr_p->event_data != NULL)
    {
        aweevent_exit(&mgr_p->event_data);
    }
    if(mgr_p->config_internally_allocated)
    {
        aweconfig_destroy(&mgr_p->config);
    }
    free(mgr_p);
    *mgr_pp = NULL;

    AWEMGR_API_LOGI("AWE Manager exited: rc=%d", awemgr_RC_OK);
    return awemgr_RC_OK;
}

awecore_error_t* awemgr_get_awe_error(struct awemgr_data* mgr_p)
{
    if(mgr_p == NULL)
    {
        AWEMGR_API_LOGE("Invalid argument: handle == NULL!");
        return NULL;
    }
    return &mgr_p->awe_error;
}

#ifdef POSSIBLY_FUTURE_EXTENSION

enum awemgr_rc  awemgr_get_variable(struct awemgr_ctx *ctx_p, int instanceId, int handle, struct awemgr_variable *var)
{
    if (instanceId >= ctx_p->instanceId) {
        AWEMGR_API_LOGE("Incorrect instanceID; does not fit to context");
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    var->handle = handle;
    var->instanceId = instanceId;

    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_get_variable_by_name(struct awemgr_ctx *ctx_p, const char *control_name, struct awemgr_variable *var_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    // todo: check control_name

    awc_ctl_t *ctl_p = awc_get_control_from_awc(ctx_p->awc, control_name);
    if (ctl_p == NULL)
    {
        AWEMGR_API_LOGE("Could not find handle for control name: %s", control_name);
        return awemgr_RC_ERR;  // todo: add correct error code
    }
    var_p->handle = ctl_p->handle;
    var_p->instanceId = ctx_p->instanceId;
    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_variable_write(struct awemgr_ctx *ctx_p, struct awemgr_variable var, int offset, void *data, int data_sz, bool noSetCall)
{
    // call awe_CMD to construct tuning message
    // EASEA: awectrl_transmit()
    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_variable_read(struct awemgr_ctx *ctx_p, struct awemgr_variable var, int offset, void *response_buffer, int response_buffer_size)
{
    // todo: check params
    return awemgr_RC_OK;
}
#endif // #ifdef POSSIBLY_FUTURE_EXTENSION

