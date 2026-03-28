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
#include "awe_awc.h"
#include "awe_ctrl.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h"  // for eg strlcpy

#include <string.h>

/* ****************************************************************************
 * PRIVATE FUNCTIONS
 * ***************************************************************************/

/**
 * @brief Load or unload a single plugin library on the AWE core
 *
 * This implements both loading and unloading of a plugin library, depending on the cmdId
 * passed in (1 = load, 2 = unload).
 *
 * This is super ugly, as AWE-Manager only works correctly now within AWE-Q which defines
 * those command identifiers.
 *
 */

#define PLUGIN_LIST_DELIMITER "|"
#define PLUGIN_NAME_INSTANCEID_DELIMITER "@"

typedef enum awemgr_rc (*plugin_cb)(const char* libname, int instanceid, void* usr_data_p);

static enum awemgr_rc plugin_load_unload(struct awemgr_ctx *ctx_p, uint8_t coreId, int cmdId, char* libname)
{
    size_t name_len = strnlen(libname, MAX_AWEMGR_FILENAME_LEN) / 4 + 1;
    enum awemgr_rc rc = awemgr_send_command(ctx_p, cmdId, coreId,
        libname, name_len,
        NULL, 0,
        NULL);
    return rc;
}

enum awemgr_rc load_plugin(const char* libname, int instanceid, void* usr_data_p)
{
    struct awemgr_ctx *ctx_p = (struct awemgr_ctx *)usr_data_p;
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    enum awemgr_rc rc = plugin_load_unload(ctx_p, instanceid, 1, (char*)libname);
    {
        AWEMGR_API_LOGE("Error loading plugin library: %s@%d", libname, instanceid);
        return rc;
    }
    return awemgr_RC_OK;
}

enum awemgr_rc unload_plugin(const char* libname, int instanceid, void* usr_data_p)
{
    struct awemgr_ctx *ctx_p = (struct awemgr_ctx *)usr_data_p;
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    enum awemgr_rc rc = plugin_load_unload(ctx_p, instanceid, 2, (char*)libname);
    {
        AWEMGR_API_LOGE("Error unloading plugin library: %s@%d", libname, instanceid);
        return rc;
    }
    return awemgr_RC_OK;
}


/**
 * @brief Parse plugin libraries listed in pluginList (separated by | and @ character e.g SomePlugin_For_1.so@1|Another_on_2.so@2)
 */
static enum awemgr_rc parse_plugin_string(const char* pluginList, plugin_cb cb, void* usr_data_p)
{
    char *context = NULL;
    char *libname_instanceid_pair = strtok_r((char*)pluginList, PLUGIN_LIST_DELIMITER, &context);
    while (libname_instanceid_pair != NULL)
    {
        AWEMGR_API_LOGI("Loading plugin library: %s\n", libname_instanceid_pair);
        // Split each pair by PLUGIN_NAME_INSTANCEID_DELIMITER
        char *inner_ctx = NULL;
        char *libname = strtok_r(libname_instanceid_pair, PLUGIN_NAME_INSTANCEID_DELIMITER, &inner_ctx);
        char *instance_id_str = strtok_r(NULL, PLUGIN_NAME_INSTANCEID_DELIMITER, &inner_ctx);
        if (!libname || !instance_id_str)
        {
            return awemgr_RC_ERR; // malformed string
        }
        int instance_id = atoi(instance_id_str);
        enum awemgr_rc rc = cb((const char*)libname, instance_id, usr_data_p);
        if (rc != awemgr_RC_OK)
        {
            return rc;
        }
        libname_instanceid_pair = strtok(NULL, PLUGIN_LIST_DELIMITER);
    }
    return awemgr_RC_OK;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int  awemgr_get_design_count(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    return awc_design_count(ctx_p->awc);
}


enum awemgr_rc awemgr_load_design(struct awemgr_ctx *ctx_p, const char *designName)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(designName);

    enum awemgr_rc rc = awemgr_RC_ERR;
    awc_design_t *pDesign = awc_get_design(ctx_p->awc, designName);
    if (pDesign == NULL)
    {
        AWEMGR_API_LOGE("Could not find design in AWC: %s", designName);
        return awemgr_RC_ERR;
    }

    //uint8_t reserved = (ctx_p->pDesign->coreid_objectid >> 24) & 0xff;
    uint8_t coreId = AWECMD_COREID_FROM_TUNNELADDRESS(pDesign->coreid_objectid);
    uint32_t objectId = AWECMD_OBJECTID_FROM_TUNNELADDRESS(pDesign->coreid_objectid);

    AWEMGR_API_LOGI("Found design: %s; instanceId: %d, objectId: %d, file: %s", pDesign->name, coreId, objectId, pDesign->file);

    char fullpath[MAX_AWEMGR_FILENAME_LEN];
    int written = snprintf(fullpath, sizeof(fullpath), "%s/%s", ctx_p->filename_path, pDesign->file);
    if (written >= sizeof(fullpath)) {
        AWEMGR_API_LOGE("Full File path %s/%s longer than buffer size (%d bytes)", ctx_p->filename_path, pDesign->file, MAX_AWEMGR_FILENAME_LEN);
        return awemgr_RC_ERR;
    }

    // check which transmission buffer is required, if it needs to be a prefixed or "normal" one
    struct awecmd_st *buf_p;
    int tunnel_address = pDesign->coreid_objectid;
    (void) awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnel_address, &buf_p);

    // check if this design is connected to a list of plugin libraries to load
    //
    const awc_dict_element *pluginlist_info_p = awc_get_top_userdata_by_key(ctx_p->awc, designName);
    if (pluginlist_info_p != NULL)
    {
        AWEMGR_API_LOGI("Design %s requires loading plugins first: %s", designName, pluginlist_info_p->value.str);
        rc = parse_plugin_string(pluginlist_info_p->value.str, load_plugin, ctx_p);
        if(rc != AWECTRL_RC_OK)
        {
            return rc;
        }
    }

    AWEMGR_API_LOGI("Loading AWB file %s (instanceId: %d)", fullpath, ctx_p->instanceId);
    FILE *fp = fopen(fullpath, "rb");
    if (!fp)
    {
        AWEMGR_API_LOGE("AWC knows about design '%s'. But file could not be loaded: %s",
                        designName, fullpath);
        return awemgr_RC_ERR;
    }

    int cnt = awecmd_from_stream(buf_p, fp, ctx_p->instanceId, coreId, objectId);
    AWEMGR_API_LOGD(" - obtained next cmd with %d words (instanceId: %d)", cnt, ctx_p->instanceId);
    while (cnt > 0)
    {
        rc = awectrl_transact(ctx_p->parent->comm_2_awe, (tunnel_address != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            goto FAIL_RET;
        }

        rc = awecmd_response_getData(buf_p, NULL, 0, NULL);

        COPY_AWE_ERROR(ctx_p->parent, buf_p);
        if (rc != AWECMD_RC_OK)
        {
            goto FAIL_RET;
        }

        awecmd_reset(buf_p, tunnel_address);
        cnt = awecmd_from_stream(buf_p, fp, ctx_p->instanceId, coreId, objectId);
        AWEMGR_API_LOGD(" - obtained next cmd with %d words (instanceId: %d)", cnt, ctx_p->instanceId);
    }

    fclose(fp);

    AWEMGR_API_LOGI("AWB data applied for %s (instanceId: %d)", pDesign->name, ctx_p->instanceId);
    if (pDesign->coreid_objectid == 0)
    {
        ctx_p->pDesign = pDesign;
    }
    return awemgr_RC_OK;

FAIL_RET:
    fclose(fp);
    if(rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("AWECore returned error");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(rc == AWECTRL_RC_TIMEOUT)
    {
        return awemgr_RC_COMM_TIMEOUT;
    }
    return awemgr_RC_ERR;
}


enum awemgr_rc awemgr_unload_design(struct awemgr_ctx *ctx_p, const char *designName)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    awc_design_t *pDesign = ctx_p->pDesign;
    if (designName != NULL)
    {
        pDesign = awc_get_design(ctx_p->awc, designName);
        if (pDesign == NULL)
        {
            AWEMGR_API_LOGE("Could not find design in AWC: %s", designName);
            return awemgr_RC_ERR;
        }
    }

    if (pDesign != NULL)
    {
        struct awecmd_st *buf_p;

        int rc = awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, pDesign->coreid_objectid, &buf_p);

        awecmd_Destroy(buf_p, ctx_p->instanceId);

        rc = awectrl_transact(ctx_p->parent->comm_2_awe, (pDesign->coreid_objectid != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            if(rc == AWECTRL_RC_TIMEOUT)
            {
                return awemgr_RC_COMM_TIMEOUT;
            }
            return awemgr_RC_ERR;
        }

        // check if this design is connected to a list of plugin libraries to remove again
        //
        const awc_dict_element *pluginlist_info_p = awc_get_top_userdata_by_key(ctx_p->awc, designName);
        if (pluginlist_info_p != NULL)
        {
            AWEMGR_API_LOGI("Design %s may possible free following plugins: %s", designName, pluginlist_info_p->value.str);
            rc = parse_plugin_string(pluginlist_info_p->value.str, unload_plugin, ctx_p);
            if(rc != AWECTRL_RC_OK)
            {
                AWEMGR_API_LOGE("Error unloading plugins for design %s", designName);
            }
        }

        AWEMGR_API_LOGI("AWE design stopped and unloaded: %s", pDesign->name);
        if (pDesign->coreid_objectid == 0)
        {
            ctx_p->pDesign = NULL;
        }
    }
    return awemgr_RC_OK;
}


enum awemgr_rc  awemgr_get_design_info(struct awemgr_ctx *ctx_p, int index, struct awemgr_design_info *info)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info);

    awc_design_t *design_p = awc_get_design_by_index(ctx_p->awc, index);
    if(design_p == NULL)
    {
        AWEMGR_API_LOGE("Could not find design at index: %d", index);
        return awemgr_RC_ERR;
    }

    info->name = design_p->name;
    info->coreid_objectid = design_p->coreid_objectid;
    info->size = design_p->size;
    info->md5sum = design_p->md5sum;
    return awemgr_RC_OK;
}

enum awemgr_rc awemgr_skip_unload_design_on_exit(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    if (ctx_p->pDesign) {
        AWEMGR_API_LOGI("Marking AWC %s so that signal flow will NOT be stopped on AWE-Manager exit.", ctx_p->pDesign->name);
        ctx_p->skip_unload_on_exit = true;
    }

    return awemgr_RC_OK;
}