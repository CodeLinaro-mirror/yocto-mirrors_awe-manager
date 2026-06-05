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


#include "awe_awc.h"
#include "awc_internal.h"
#include "awc_logging.h"
#include "awosal_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


void *awc_init(const char *filename)
{
    if (!filename)
        return NULL;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        AWC_LOGE("Error opening file: %s", filename);
        return NULL;
    }

    awe_awc_t *awc = calloc(1, sizeof(awe_awc_t));
    if (!awc)
    {
        fclose(file);
        return NULL;
    }

    char line[MAX_LINE_SIZE];
    int lineNumber = 0;
    int status = E_AWC_SUCCESS;

    while (fgets(line, sizeof(line), file))
    {
        lineNumber++;
        status = parseLine(line, awc);
        if(status == E_AWC_CTL_TYPE_ERROR)
        {
            AWC_LOGW("Ignoring invalid control type in line %d:%s", lineNumber, line);
            status = E_AWC_SUCCESS;
        }
        if (status == E_AWC_ERROR)
        {
            AWC_LOGE("Error during Parsing line %d:%s", lineNumber -1, line);
            break;
        }
    }
    fclose(file);

    if (status == E_AWC_ERROR || validateAwc(awc) == E_AWC_ERROR)
    {
        awc_uninit((void **)&awc);
        return NULL;
    }
    return awc;
}

int awc_uninit(void **pHandle)
{
    if ((pHandle != NULL) && (*pHandle != NULL))
    {
        awe_awc_t *awc = *pHandle;
        awcDestructor(awc);
        free(awc);
        *pHandle = NULL;
        AWC_LOGI("AWC memory released");
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}

awc_info_t* awc_get_info(const void* handle)
{
    awe_awc_t* awc = (awe_awc_t*)handle;
    if (awc != NULL)
    {
        return &awc->info;
    }
    return NULL;
}

awc_design_t *awc_get_design(const void *handle, const char *name)
{
    const awe_awc_t *awc = (awe_awc_t*)handle;
    if ((awc != NULL) && (name != NULL))
    {
        for (UINT32 idx = 0; idx < awc->info.designcount; idx++)
        {
            awc_design_t* pDesign = &awc->designs[idx];
            if (strcmp(pDesign->name, name) == 0)
            {
                return pDesign;
            }
        }
    }
    return NULL;
}

UINT32 awc_design_count(const void* handle)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if (awc != NULL)
    {
        return awc->info.designcount;
    }
    return 0;
}

awc_design_t* awc_get_design_by_index(const void* handle, UINT32 index)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if ((awc != NULL) && awc->info.designcount > index)
    {
        return &awc->designs[index];
    }
    return NULL;
}

awc_module_t *awc_get_module(const void *handle, const char *name)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    return getModuleByName(awc, name);
}

UINT32 awc_module_count(const void* handle)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if (awc != NULL)
    {
        return awc->info.modulecount;
    }
    return 0;
}

awc_module_t* awc_get_module_by_index(const void* handle, UINT32 index)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if ((awc != NULL) && (awc->info.modulecount > index))
    {
        return &awc->modules[index];
    }
    return NULL;
}

UINT32 awc_control_count(const void* handle)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if (awc != NULL)
    {
        return awc->info.controlcount;
    }
    return 0;
}

awc_ctl_t* awc_get_control_by_index(const void* handle, UINT32 index)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if ((awc != NULL) && (awc->info.controlcount > index))
    {
        return &awc->controls[index];
    }
    return NULL;
}

UINT32 awc_module_control_count(const awc_module_t* module)
{
    if (module != NULL)
    {
        return module->size;
    }
    return 0;
}

awc_ctl_t* awc_get_module_control_by_index(const awc_module_t* module, UINT32 index)
{
    if ((module != NULL) && (module->size > index))
    {
        return module->controls[index];
    }
    return NULL;
}

awc_ctl_t *awc_get_control_from_awc(const void *handle, const char *name)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    return getControlByName(awc, name);
}

awc_ctl_t *awc_get_control_from_module(const awc_module_t *module, const char *name)
{
    if ((module != NULL) && (name != NULL))
    {
        for (UINT32 idx = 0; idx < module->size; idx++)
        {
            awc_ctl_t* pCtl = module->controls[idx];
            if (strcmp(pCtl->varname, name) == 0)
            {
                return pCtl;
            }
        }
    }
    return NULL;
}

const char *awc_get_typename(const awc_ctl_type_t type)
{
    switch (type)
    {
    case AWC_CTL_BOOL:
        return "bool";
        break;
    case AWC_CTL_INT32:
        return "int";
        break;
    case AWC_CTL_FRACT32:
        return "fract32";
        break;
    case AWC_CTL_FRACT16:
        return "fract16";
        break;
    case AWC_CTL_UINT32:
        return "uint";
        break;
    case AWC_CTL_FLOAT:
        return "float";
        break;
    case AWC_CTL_ENUM:
        return "enum";
        break;
    default:
        return "invalid";
        break;
    };
}

int awc_foreach_control(const awc_module_t* module, control_cb cb, void *usr_data_p)
{
    if ((module != NULL) && (cb != NULL))
    {
        for (UINT32 idx = 0; idx < module->size; idx++)
        {
            cb((const awc_ctl_t*)module->controls[idx], usr_data_p);
        }
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}

int awc_foreach_module(const void *handle, module_cb cb, void *usr_data_p)
{
    awe_awc_t* awc = (awe_awc_t*)handle;
    if ((awc != NULL) && (cb != NULL))
    {
        for (UINT32 idx = 0; idx < awc->info.modulecount; idx++)
        {
            cb((const awc_module_t*)&awc->modules[idx], usr_data_p);
        }
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}

int awc_foreach_design(const void *handle, design_cb cb, void *usr_data_p)
{
    awe_awc_t* awc = (awe_awc_t*)handle;
    if ((awc != NULL) && (cb != NULL))
    {
        for (UINT32 idx = 0; idx < awc->info.designcount; idx++)
        {
            cb((const awc_design_t*)&awc->designs[idx], usr_data_p);
        }
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}

UINT32 awc_event_count(const void* handle)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if (awc != NULL)
    {
        return awc->info.eventcount;
    }
    return 0;
}

awc_event_t* awc_get_event_by_index(const void* handle, UINT32 index)
{
    const awe_awc_t* awc = (const awe_awc_t*)handle;
    if ((awc != NULL) && (awc->info.eventcount > index))
    {
        return &awc->events[index];
    }
    return NULL;
}

int awc_foreach_event(const void *handle, event_cb cb, void *usr_data_p)
{
    awe_awc_t* awc = (awe_awc_t*)handle;
    if ((awc != NULL) && (cb != NULL))
    {
        for (UINT32 idx = 0; idx < awc->info.eventcount; idx++)
        {
            cb((const awc_event_t*)&awc->events[idx], usr_data_p);
        }
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}

#if 0 // TODO: Enable this when the type information is available in the awc_index file
int awc_foreach_event_type(const void *handle, event_cb cb, UINT32 type, void *usr_data_p)
{
    awe_awc_t* awc = (awe_awc_t*)handle;
    if ((awc != NULL) && (cb != NULL))
    {
        for (UINT32 idx = 0; idx < awc->info.eventcount; idx++)
        {
            if(awc->events[idx].type == type)
            {
                cb((const awc_event_t*)&awc->events[idx], usr_data_p);
            }
        }
        return E_AWC_SUCCESS;
    }
    return E_AWC_ERROR;
}
#endif

awc_event_t* awc_get_event(const void *handle, const char* name)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    if (awc != NULL)
    {
        for (UINT32 idx = 0; idx < awc->info.eventcount; idx++)
        {
            awc_event_t* pEvt = &awc->events[idx];
            if (strcmp(pEvt->mod->name, name) == 0)
            {
                return pEvt;
            }
        }
    }
    return NULL;
}

awc_event_t* awc_get_event_by_objectid(const void *handle, UINT32 objectid)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    if (awc != NULL)
    {
        for (UINT32 idx = 0; idx < awc->info.eventcount; idx++)
        {
            awc_event_t* pEvt = &awc->events[idx];
            if (pEvt->mod->objectid == objectid)
            {
                return pEvt;
            }
        }
    }
    return NULL;
}

int awc_get_module_userdata_count(const void *handle, const char* modulename)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    awc_module_t* mod_p = getModuleByName(awc, modulename);
    if(mod_p != NULL)
    {
        return mod_p->userdata.size;
    }
    return -1;
}

int awc_get_control_userdata_count(const void *handle, const char* controlname)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    awc_ctl_t* ctl_p = getControlByName(awc, controlname);
    if(ctl_p != NULL)
    {
        return ctl_p->userdata.size;
    }
    return -1;
}

const awc_dict_element* awc_get_module_userdata_by_index(const void *handle, const char* modulename, uint32_t index)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    awc_module_t* mod_p = getModuleByName(awc, modulename);
    if(mod_p != NULL)
    {
        if(mod_p->userdata.data && index < mod_p->userdata.size)
        {
            return &mod_p->userdata.data[index];
        }
    }
    return NULL;
}

const awc_dict_element* awc_get_control_userdata_by_index(const void *handle, const char* controlname, uint32_t index)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    awc_ctl_t* ctl_p = getControlByName(awc, controlname);
    if(ctl_p != NULL)
    {
        if(ctl_p->userdata.data && index < ctl_p->userdata.size)
        {
            return &ctl_p->userdata.data[index];
        }
    }
    return NULL;
}

const awc_dict_element* awc_get_module_userdata_by_key(const void *handle, const char* modulename, const char* key)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    awc_module_t* mod_p = getModuleByName(awc, modulename);
    if(mod_p != NULL)
    {
        return searchInDictionary(&mod_p->userdata, key);
    }
    return NULL;
}

const awc_dict_element* awc_get_control_userdata_by_key(const void *handle, const char* controlname, const char* key)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    awc_ctl_t* ctl_p = getControlByName(awc, controlname);
    if(ctl_p != NULL)
    {
        return searchInDictionary(&ctl_p->userdata, key);
    }
    return NULL;
}

const awc_dict_element* awc_get_top_userdata_by_key(const void *handle, const char* key)
{
    const awe_awc_t *awc = (const awe_awc_t*)handle;
    if(awc != NULL)
    {
        return searchInDictionary(&awc->userdata, key);
    }
    return NULL;
}