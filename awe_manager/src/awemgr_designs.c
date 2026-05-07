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
#include "awe_comm.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include "awosal_string.h"  // for eg strlcpy
#include "awosal_time.h"    // for eg aweosal_mssleep

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

// this is private, but not a static function, as it is used in test case code
enum awemgr_rc parse_plugin_string(const char* pluginList, struct awemgr_design_info *design_info)
{
    if (!pluginList || !design_info) {
        return awemgr_RC_ERR;
    }

    char *input_copy = strdup(pluginList);
    if (!input_copy) {
        AWEMGR_API_LOGE("Failed to allocate memory");
        return awemgr_RC_ERR;
    }

    enum awemgr_rc rc = awemgr_RC_OK;
    char *context_outer = NULL;
    unsigned int plugin_count = 0;

    char *pair = strtok_r(input_copy, PLUGIN_LIST_DELIMITER, &context_outer);
    while (pair != NULL)
    {
        if (plugin_count >= AWEMGR_MAX_PLUGINS_PER_DESIGN) {
            AWEMGR_API_LOGE("Exceeded max plugin count (%d)", AWEMGR_MAX_PLUGINS_PER_DESIGN);
            rc = awemgr_RC_ERR;
            break;
        }

        char *context_inner = NULL;
        char *libname = strtok_r(pair, PLUGIN_NAME_INSTANCEID_DELIMITER, &context_inner);
        char *id_str = strtok_r(NULL, PLUGIN_NAME_INSTANCEID_DELIMITER, &context_inner);

        if (!libname || !id_str) {
            AWEMGR_API_LOGE("Malformed plugin string near: %s", pair);
            rc = awemgr_RC_ERR;
            break;
        }

        char *endptr;
        long core_id = strtol(id_str, &endptr, 10);
        if (*endptr != '\0' || core_id < 0 || core_id >= MAX_AWE_ENDPOINTS) {
            AWEMGR_API_LOGE("Invalid Core ID: %s", id_str);
            rc = awemgr_RC_ERR;
            break;
        }

        struct awemgr_plugin *plugin = &design_info->plugins[plugin_count];

        // ensure name fits within the buffer + null terminator
        size_t copied = strlcpy(plugin->name, libname, sizeof(plugin->name));
        if (copied >= sizeof(plugin->name)) {
            AWEMGR_API_LOGE("Plugin name truncated: %s", libname);
            rc = awemgr_RC_ERR;
            break;
        }

        plugin->core = (int)core_id;
        plugin_count++;

        pair = strtok_r(NULL, PLUGIN_LIST_DELIMITER, &context_outer);
    }

    design_info->plugin_count = plugin_count;
    free(input_copy);
    return rc;
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
    struct awecmd_st *buf_p = NULL;
    int tunnel_address = pDesign->coreid_objectid;

    AWEMGR_API_LOGI("Loading AWB file %s (instanceId: %d)", fullpath, ctx_p->instanceId);
    FILE *fp = fopen(fullpath, "rb");
    if (!fp)
    {
        AWEMGR_API_LOGE("AWC knows about design '%s'. But file could not be loaded: %s",
                        designName, fullpath);
        return awemgr_RC_ERR;
    }

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnel_address, &buf_p));
    int cnt = awecmd_from_stream(buf_p, fp, ctx_p->instanceId, coreId, objectId);

    AWEMGR_API_LOGD(" - obtained next cmd with %d words (instanceId: %d)", cnt, ctx_p->instanceId);
    while (cnt > 0)
    {
        rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, NULL, 0, NULL);
        (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);
        if (rc != awemgr_RC_OK)
        {
            break;
        }

        // without the sleep statement this loop is so fast that it is not preempted
        aweosal_usleep(1);

        AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnel_address, &buf_p));

        awecmd_reset(buf_p, tunnel_address);
        cnt = awecmd_from_stream(buf_p, fp, ctx_p->instanceId, coreId, objectId);
        AWEMGR_API_LOGD(" - obtained next cmd with %d words (instanceId: %d)", cnt, ctx_p->instanceId);
    }

    (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);
    fclose(fp);

    if (rc == awemgr_RC_OK)
    {
        AWEMGR_API_LOGI("AWB data applied for %s (instanceId: %d)", pDesign->name, ctx_p->instanceId);
        if (pDesign->coreid_objectid == 0)
        {
            ctx_p->pDesign = pDesign;
        }
    }
    return rc;
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

    enum awemgr_rc rc = awemgr_RC_OK;
    if (pDesign != NULL)
    {

        struct awecmd_st *buf_p = NULL;

        AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(ctx_p->parent->comm_2_awe, pDesign->coreid_objectid, &buf_p));

        awecmd_Destroy(buf_p, ctx_p->instanceId);

        rc = safe_transact(ctx_p->parent->comm_2_awe, buf_p, NULL, 0, NULL);

        (void) awecomm_release_lock(ctx_p->parent->comm_2_awe);

        if (rc == awemgr_RC_OK)
        {
            AWEMGR_API_LOGI("AWE design stopped and unloaded: %s", pDesign->name);
            if (pDesign->coreid_objectid == 0)
            {
                ctx_p->pDesign = NULL;
            }
        }

    }
    return rc;
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
    info->plugin_count = 0;
    // check if this design is connected to a list of plugin libraries to load
    const awc_dict_element *pluginlist_info_p = awc_get_top_userdata_by_key(ctx_p->awc, info->name);
    if (pluginlist_info_p != NULL)
    {
        AWEMGR_API_LOGI("Design %s requires plugins: %s", info->name, pluginlist_info_p->value.str);
        memset(info->plugins, 0, sizeof(info->plugins));
        return parse_plugin_string(pluginlist_info_p->value.str, info);
    }

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
