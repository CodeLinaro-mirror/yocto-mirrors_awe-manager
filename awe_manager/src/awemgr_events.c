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
#include "awosal_string.h"


// static event_cb_t  cb;

/**
 * Private structure used to pass the awemgr_ctx* and event subscribe/unsubscribe information to the eventSubscriptionHandler method
 */
struct ev_task_st
{
    bool bEnable;
    struct awemgr_ctx* ctx_p;
};



static void eventSubscriptionHandler(const awc_event_t* evt, void *usrData)
{
    struct awemgr_module module_info;
    if ((evt != NULL) && (usrData != NULL))
    {
        struct ev_task_st* task_p = (struct ev_task_st*) usrData;
        module_info.instanceId = task_p->ctx_p->instanceId;
        copyInfoFromAwcModule(&module_info, evt->mod);
        enum awemgr_module_runtimestate state = task_p->bEnable ? MODULE_ACTIVE : MODULE_INACTIVE;
        enum awemgr_rc rc = awemgr_module_set_state(task_p->ctx_p, module_info, state);
        if (rc == awemgr_RC_OK)
        {
            AWEMGR_API_LOGI("Event Module %s set to %s", evt->mod->name,
                            awemgr_module_runtimestate_as_string[state]);
        }
    }
}

/* USED IN GTESTS as extern C : so it is declared public instead of static */
int mgrevent_dispatcher(const aweevent_header* pHdr, const char* pPayload, const void* pUsrData)
{
    struct awemgr_ctx *ctx_p = (struct awemgr_ctx *)pUsrData;
    awemgr_event evt;
    event_cb_t category_listener = NULL;
    void* category_userdata = NULL;

    memset((void *)&evt, 0, sizeof(evt));
    evt.eventCategory = pHdr->eventCategory;
    if(evt.eventCategory < MAX_SUPPORTED_CATEGORIES && (ctx_p->parent->evt_category_cb[evt.eventCategory].cb != NULL))
    {
        category_listener = ctx_p->parent->evt_category_cb[evt.eventCategory].cb;
        category_userdata = ctx_p->parent->evt_category_cb[evt.eventCategory].user_data_p;
    }

    evt.module.instanceId = pHdr->instanceId;
    evt.module.objectId = pHdr->objectId;
    evt.module.classId = pHdr->classId;
    evt.eventType = pHdr->eventType;
    evt.timeStamp = pHdr->timeStamp;
    evt.sizeInBytes = pHdr->dataSize;
    evt.payload = pPayload;
    awc_event_t* evt_p = awc_get_event_by_objectid(ctx_p->awc, pHdr->objectId);
    if(evt_p != NULL && evt_p->mod != NULL)
    {
        evt.module.name = evt_p->mod->name;
        evt.module.alias = evt_p->mod->alias;
    }
    else
    {
        evt.module.name = "";
        evt.module.alias = "";
    }

    bool event_delivered = false;
    if(category_listener != NULL)
    {
        category_listener(&evt, category_userdata);
        event_delivered = true;
    }

    for(int i = 0; i < MAX_SUPPORTED_GENERIC_LISTENERS; i++)
    {
        if(ctx_p->parent->evt_generic_cb[i].cb != NULL)
        {
            ctx_p->parent->evt_generic_cb[i].cb(&evt, ctx_p->parent->evt_generic_cb[i].user_data_p);
            event_delivered = true;
        }
    }

    if (! event_delivered)
    {
        AWEMGR_API_LOGW("Received event (category: %d, type: %d) for which no listener is registered.", evt.eventCategory, evt.eventType);
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
}


/* ****************************************************************************
 * PUBLIC FUNCTIONS - EVENT SUBSYSTEM
 * ***************************************************************************/

enum awemgr_rc awemgr_events_start(struct awemgr_ctx *ctx_p, event_cb_t cb, void* userdata)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p->parent);
    if(ctx_p->parent->event_data != NULL)
    {
        AWEMGR_API_LOGE("Events already started");
        return awemgr_RC_ERR;
    }
    int rc = aweevent_init(ctx_p->parent->config, &ctx_p->parent->event_data, mgrevent_dispatcher, ctx_p);
    if(rc != AWECOMM_RC_OK)
    {
        AWEMGR_API_LOGE("event start failed");
        return awemgr_RC_ERR;
    }

    awemgr_events_set_listener(ctx_p, cb, userdata);
    ctx_p->events_handling_enabled = true;
    return awemgr_RC_OK;
}

enum awemgr_rc awemgr_events_set_listener(struct awemgr_ctx *ctx_p, event_cb_t cb, void* userdata)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(cb);
    for(int i = 0; i < MAX_SUPPORTED_GENERIC_LISTENERS; i++)
    {
        if(ctx_p->parent->evt_generic_cb[i].cb == cb || ctx_p->parent->evt_generic_cb[i].cb == NULL)
        {
            ctx_p->parent->evt_generic_cb[i].cb = cb;
            ctx_p->parent->evt_generic_cb[i].user_data_p = userdata;
            return awemgr_RC_OK;
        }
    }
    AWEMGR_API_LOGE("Maximum number of event inspectors (%d) reached.", MAX_SUPPORTED_GENERIC_LISTENERS);
    return awemgr_RC_ERR;
}

enum awemgr_rc awemgr_events_clear_inspector(struct awemgr_ctx *ctx_p, event_cb_t cb)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(cb);
    for(int i = 0; i < MAX_SUPPORTED_GENERIC_LISTENERS; i++)
    {
        if(ctx_p->parent->evt_generic_cb[i].cb == cb)
        {
            ctx_p->parent->evt_generic_cb[i].cb = NULL;
            ctx_p->parent->evt_generic_cb[i].user_data_p = NULL;
            return awemgr_RC_OK;
        }
    }
    return awemgr_RC_ERR;
}

enum awemgr_rc awemgr_events_add_category_listener(struct awemgr_ctx *ctx_p, uint32_t category, event_cb_t cb, void* userdata)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(cb);
    if(category >= 0 && category < MAX_SUPPORTED_CATEGORIES)
    {
        ctx_p->parent->evt_category_cb[category].cb = cb;
        ctx_p->parent->evt_category_cb[category].user_data_p = userdata;
        return awemgr_RC_OK;
    }
    else
    {
        AWEMGR_API_LOGE("Category callbacks are supported only from category 0 to category 31.");
    }
    return awemgr_RC_ERR;
}

enum awemgr_rc awemgr_events_remove_category_listener(struct awemgr_ctx *ctx_p, uint32_t category)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    if(category >= 0 && category < MAX_SUPPORTED_CATEGORIES)
    {
        ctx_p->parent->evt_category_cb[category].cb = NULL;
        return awemgr_RC_OK;
    }
    else
    {
        AWEMGR_API_LOGE("Category callbacks are supported only from 0 to 31.");
    }
    return awemgr_RC_ERR;
}


enum awemgr_rc awemgr_events_stop(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p->parent);

    ctx_p->events_handling_enabled = false;
    if(ctx_p->parent->event_data == NULL)
    {
        AWEMGR_API_LOGD("Events already stopped");
        return awemgr_RC_OK;
    }
    return aweevent_exit(&ctx_p->parent->event_data);
}


bool awemgr_events_started(struct awemgr_ctx *ctx_p)
{
    if (ctx_p == NULL)
    {
        AWEMGR_API_LOGE("Invalid argument: handle == NULL!");
        return false;
    }
    return ctx_p->events_handling_enabled;
}


enum awemgr_rc  awemgr_events_process_next(struct awemgr_ctx *ctx_p, uint32_t timeoutMs)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    int rc = aweevent_read(ctx_p->parent->event_data, timeoutMs);

    return awemgr_comm_rc_to_mgr_rc(rc);
}


/* ****************************************************************************
 * PUBLIC FUNCTIONS - EVENT SPECIFIC
 * ***************************************************************************/

int  awemgr_get_event_count(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    return awc_event_count(ctx_p->awc);
}


enum awemgr_rc  awemgr_get_event_by_index(struct awemgr_ctx *ctx_p, int index, struct awemgr_module *mod_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(mod_p);

    awc_event_t *event_p = awc_get_event_by_index(ctx_p->awc, index);
    if(!event_p)
    {
        return awemgr_RC_ERR;
    }
    mod_p->instanceId = ctx_p->instanceId;
    copyInfoFromAwcModule(mod_p, event_p->mod);
    return awemgr_RC_OK;
}


enum awemgr_rc  awemgr_enable_event(struct awemgr_ctx *ctx_p, const char* module_name)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(module_name);
    awc_event_t* pEvt = awc_get_event(ctx_p->awc, module_name);
    if(pEvt != NULL)
    {
        AWEMGR_API_LOGI("Subscribing to event module %s, context: %d;", module_name, ctx_p->instanceId);
        struct ev_task_st s;
        s.bEnable = true;
        s.ctx_p = ctx_p;
        eventSubscriptionHandler(pEvt,  &s);
        return awemgr_RC_OK;
    }
    return awemgr_RC_ERR;
}


enum awemgr_rc  awemgr_disable_event(struct awemgr_ctx *ctx_p, const char* module_name)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(module_name);
    awc_event_t* pEvt = awc_get_event(ctx_p->awc, module_name);
    if(pEvt != NULL)
    {
        AWEMGR_API_LOGI("Unsubscribing to event module %s, context: %d;", module_name, ctx_p->instanceId);
        struct ev_task_st s;
        s.bEnable = false;
        s.ctx_p = ctx_p;
        eventSubscriptionHandler(pEvt,  &s);
        return awemgr_RC_OK;
    }
    return awemgr_RC_ERR;
}


#if EVENT_API_WITH_TYPE // TODO: Enable this when the type information is available in the awc_index file
enum awemgr_rc  awemgr_enable_event_type(struct awemgr_ctx *ctx_p, unsigned int type)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_API_LOGI("Subscribing to event type, context: %d; type %d", ctx_p->instanceId, type);

    struct ev_task_st s;
    s.bEnable = true;
    s.ctx_p = ctx_p;
    awc_foreach_event_type(ctx_p->awc, eventSubscriptionHandler, type, &s);

    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_disable_event_type(struct awemgr_ctx *ctx_p, unsigned int type)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_API_LOGI("Unsubscribing from event type, context: %d; type %d", ctx_p->instanceId, type);

    struct ev_task_st s;
    s.bEnable = false;
    s.ctx_p = ctx_p;
    awc_foreach_event_type(ctx_p->awc, eventSubscriptionHandler, type, &s);

    return awemgr_RC_OK;
}
#endif

