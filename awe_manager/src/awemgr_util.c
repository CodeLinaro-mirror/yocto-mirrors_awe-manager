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

#include "awemgr_util.h"
#include "awemgr_api_logging.h"
#include "types/awemgr_limits.h"


enum awemgr_vartype awctype_2_awemgrtype(awc_ctl_type_t awc_type)
{
    enum awemgr_vartype awemgr_type = AWEMGR_VARTYPE_UNDEF;

    switch(awc_type)
    {
        case AWC_CTL_FLOAT:
            awemgr_type = AWEMGR_VARTYPE_FLOAT;
            break;
        case AWC_CTL_INT32:
        case AWC_CTL_UINT32:
        case AWC_CTL_BOOL:
            awemgr_type = AWEMGR_VARTYPE_INTEGER;
            break;
        case AWC_CTL_FRACT32:
            awemgr_type = AWEMGR_VARTYPE_FRACT;
            break;
        case AWC_CTL_UNKNOWN:
        case AWC_CTL_ENUM:
        default:
            AWEMGR_API_LOGW("Unhandled variable type in awctype_2_awemgrtype: AWC = %s(%d)", awc_get_typename(awc_type), awc_type);
            break;
    }
    return awemgr_type;
}

void copy_ctl_info(awc_ctl_t* var_p, struct awemgr_ctl_elem_info *info)
{
    info->id.name = var_p->fullname;
    info->id.type = awctype_2_awemgrtype(var_p->type);
    info->id.nr_items = var_p->size;
    info->id.parentName = var_p->module ? var_p->module->name : NULL;
    info->id.alias = var_p->alias;
    switch(info->id.type)
    {
        case AWEMGR_VARTYPE_INTEGER:
        case AWEMGR_VARTYPE_FRACT:
            info->value.integer.default_value = var_p->range.def;
            info->value.integer.min = var_p->range.min;
            info->value.integer.max = var_p->range.max;
            info->value.integer.step = var_p->range.step;
            break;
        case AWEMGR_VARTYPE_FLOAT:
            // todo: currently cast of IUNt32 to float! check this!!!
            info->value.floating.default_value = (float)var_p->range.def;
            info->value.floating.min = (float)var_p->range.min;
            info->value.floating.max = (float)var_p->range.max;
            info->value.floating.step = (float)var_p->range.step;
            break;
        default:
            AWEMGR_API_LOGW("Unhandled variable type in copy_ctl_info: %s", awemgr_vartype_to_string(info->id.type));
            break;
    }
}

void copyInfoFromAwcModule(struct awemgr_module *mod, const awc_module_t* awc_module)
{
    mod->objectId = awc_module->objectid;
    mod->classId = awc_module->classid;
    mod->name = awc_module->name;
    mod->alias = awc_module->alias;
};

void awemgr_copy_usrdata(const awc_dict_element* src, awemgr_userdata* dst)
{
    if((src != NULL) && (dst != NULL))
    {
        dst->key = src->key;
        dst->type = (awemgr_userdata_type)src->type;
        dst->value.str = src->value.str;
    }
}


enum awemgr_rc safe_transact(struct awectrl_data *ctrl_p, struct awecmd_st *buf_p, awecore_error_t *err_p, unsigned int *result_buffer_p, unsigned int result_buffer_size_in_words)
{
    int rc = awectrl_transact(ctrl_p, AWEMGR_CHANNEL_0);
    if(rc != AWECTRL_RC_OK)
    {
        if(rc == AWECTRL_RC_TIMEOUT)
        {
            return awemgr_RC_COMM_TIMEOUT;
        }
        return awemgr_RC_ERR;
    }

    unsigned int rx_words_received;
    rc = awecmd_response_getData(buf_p, result_buffer_p, result_buffer_size_in_words, &rx_words_received);

    err_p->error_code = buf_p->error_code;
    err_p->error_desc = buf_p->error_desc;

    if(rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("Could not parse response.");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(rc == AWECMD_RC_ERR)
    {
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
}

float awemgr_getCpuPercentage(awemgr_cpuinfo* cpu_p)
{
    return 100.0f * (float)(unsigned)(cpu_p->AverageCycles>>8) / (float)(unsigned)(cpu_p->TimePerProcess>>8);
}
