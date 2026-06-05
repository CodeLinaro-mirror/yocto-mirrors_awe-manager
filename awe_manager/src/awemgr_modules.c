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
#include "awe_awc.h"
#include "awe_cmd.h"
#include "awe_comm.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL, copyInfoFromAwcModule

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int  awemgr_get_modules_count(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    return awc_module_count(ctx_p->awc);
}

enum awemgr_rc  awemgr_get_module_by_name(struct awemgr_ctx *ctx_p, const char *module_name, struct awemgr_module *mod)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(module_name);
    AWEMGR_FAIL_ON_HANDLE_NULL(mod);

    awc_module_t *awc_mdl = awc_get_module(ctx_p->awc, module_name);

    if (awc_mdl == NULL)
    {
        AWEMGR_API_LOGE("Could not find module %s", module_name);
        return awemgr_RC_ERR;
    }
    mod->instanceId = ctx_p->instanceId;
    copyInfoFromAwcModule(mod, awc_mdl);

    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_get_module_by_index(struct awemgr_ctx *ctx_p, int index, struct awemgr_module *mod)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(mod);

    awc_module_t *awc_mdl = awc_get_module_by_index(ctx_p->awc, index);

    if (awc_mdl == NULL)
    {
        AWEMGR_API_LOGE("Could not find module at position %d", index);
        return awemgr_RC_ERR;
    }
    mod->instanceId = ctx_p->instanceId;
    copyInfoFromAwcModule(mod, awc_mdl);

    return awemgr_RC_OK;
}


enum awemgr_rc  awemgr_module_set_state(struct awemgr_ctx *ctx_p, struct awemgr_module mod, enum awemgr_module_runtimestate operating_state)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    struct awecmd_st *buf_p = NULL;

    INT32 tunnelAddress = 0;
    UINT32 coreid = 0;
    awc_module_t* awcMod_p = awc_get_module(ctx_p->awc, mod.name);
    if(awcMod_p)
    {
        tunnelAddress = awcMod_p->tunnelAddress;
        coreid = awcMod_p->coreid;
    }

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnelAddress, &buf_p));

    awecmd_setModuleStatus(buf_p, (tunnelAddress != 0) ? 0 : ctx_p->instanceId, coreid, mod.objectId, operating_state);

    enum awemgr_rc rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, NULL, 0, NULL);

    (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);

    return rc;
}


enum awemgr_rc awemgr_module_get_state(struct awemgr_ctx *ctx_p, struct awemgr_module mod, enum awemgr_module_runtimestate *state_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(state_p);

    struct awecmd_st *buf_p = NULL;

    INT32 tunnelAddress = 0;
    UINT32 coreid = 0;
    awc_module_t* awcMod_p = awc_get_module(ctx_p->awc, mod.name);
    if(awcMod_p)
    {
        tunnelAddress = awcMod_p->tunnelAddress;
        coreid = awcMod_p->coreid;
    }

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnelAddress, &buf_p));

    awecmd_getModuleStatus(buf_p, (tunnelAddress != 0) ? 0 : ctx_p->instanceId, coreid, mod.objectId);

    UINT32 moduleState = 0;

    enum awemgr_rc rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, &moduleState, 1, NULL);

    (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);

    *state_p = (enum awemgr_module_runtimestate)moduleState;

    return rc;
}

enum awemgr_rc awemgr_module_get_class(struct awemgr_ctx *ctx_p, struct awemgr_module mod, unsigned int *classId_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(classId_p);

    struct awecmd_st *buf_p = NULL;

    INT32 tunnelAddress = 0;
    UINT32 coreid = 0;
    awc_module_t* awcMod_p = awc_get_module(ctx_p->awc, mod.name);
    if(awcMod_p)
    {
        tunnelAddress = awcMod_p->tunnelAddress;
        coreid = awcMod_p->coreid;
    }

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnelAddress, &buf_p));

    awecmd_getModuleClass(buf_p, (tunnelAddress != 0) ? 0 : ctx_p->instanceId, coreid, mod.objectId);

    unsigned int classId[1];

    enum awemgr_rc rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, classId, 1, NULL);

    (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);

    *classId_p = classId[0];

    return rc;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS - USER DATA
 * ***************************************************************************/

int awemgr_get_module_userdata_count(struct awemgr_ctx *ctx_p, const char* modulename)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    return awc_get_module_userdata_count(ctx_p->awc, modulename);
}

enum awemgr_rc awemgr_get_module_userdata_by_index(struct awemgr_ctx *ctx_p, const char* modulename, uint32_t index, awemgr_userdata* data)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(data);
    const awc_dict_element* elem_p =  awc_get_module_userdata_by_index(ctx_p->awc, modulename, index);
    if(elem_p != NULL)
    {
        awemgr_copy_usrdata(elem_p, data);
        return awemgr_RC_OK;
    }
    return awemgr_RC_ERR;
}

enum awemgr_rc awemgr_get_module_userdata_by_key(struct awemgr_ctx *ctx_p, const char* modulename, const char* key, awemgr_userdata* data)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(key);
    AWEMGR_FAIL_ON_HANDLE_NULL(data);
    const awc_dict_element* elem_p =  awc_get_module_userdata_by_key(ctx_p->awc, modulename, key);
    if(elem_p != NULL)
    {
        awemgr_copy_usrdata(elem_p, data);
        return awemgr_RC_OK;
    }
    return awemgr_RC_ERR;
}

