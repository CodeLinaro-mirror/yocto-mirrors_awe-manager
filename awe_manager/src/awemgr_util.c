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

#include "awemgr_util.h"
#include "awemgr_api_logging.h"
#include "types/awemgr_limits.h"
#include "types/awemgr_data.h"
#include "awe_comm.h"       // for awecomm_transact
#include "awe_cmd.h"        // for awecmd_response_getData
#include <math.h>           // for fmod and fabs
#include <inttypes.h>       // for PRId64

#if defined(_MSC_VER)
static __declspec(thread) awecore_error_t awemgr_tls_error = {0, "No Error"};
#else
static _Thread_local awecore_error_t awemgr_tls_error = {0, "No Error"};
#endif


int awemgr_get_awe_error_code(void)
{
    return awemgr_tls_error.error_code;
}

const char* awemgr_get_awe_error_string(void)
{
    return awemgr_tls_error.error_desc;
}


enum awemgr_vartype awctype_2_awemgrtype(awc_ctl_type_t awc_type)
{
    enum awemgr_vartype awemgr_type = AWEMGR_VARTYPE_UNDEF;

    switch(awc_type)
    {
        case AWC_CTL_FLOAT:
            awemgr_type = AWEMGR_VARTYPE_FLOAT;
            break;
        case AWC_CTL_UINT32:
            awemgr_type = AWEMGR_VARTYPE_UNSIGNED_INTEGER;
            break;
        case AWC_CTL_INT32:
            awemgr_type = AWEMGR_VARTYPE_INTEGER;
            break;
        case AWC_CTL_ENUM:
            awemgr_type = AWEMGR_VARTYPE_ENUM;
            break;
        case AWC_CTL_BOOL:
            awemgr_type = AWEMGR_VARTYPE_BOOL;
            break;
        case AWC_CTL_FRACT32:
            awemgr_type = AWEMGR_VARTYPE_FRACT32;
            break;
        case AWC_CTL_FRACT16:
            awemgr_type = AWEMGR_VARTYPE_FRACT16;
            break;
        case AWC_CTL_UNKNOWN:
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
    info->range_enabled = var_p->checkRange;
    switch(info->id.type)
    {
        case AWEMGR_VARTYPE_UNSIGNED_INTEGER:
            info->range.u32.min = (uint32_t)var_p->range.min;
            info->range.u32.max = (uint32_t)var_p->range.max;
            info->range.u32.step = (uint32_t)var_p->range.step;
            break;
        case AWEMGR_VARTYPE_INTEGER:
        case AWEMGR_VARTYPE_BOOL:
            info->range.i32.min = (int32_t)var_p->range.min;
            info->range.i32.max = (int32_t)var_p->range.max;
            info->range.i32.step = (int32_t)var_p->range.step;
            break;
        case AWEMGR_VARTYPE_FRACT32:
        case AWEMGR_VARTYPE_FRACT16:
        case AWEMGR_VARTYPE_FLOAT:
            info->range.f32.min = (float)var_p->range.min;
            info->range.f32.max = (float)var_p->range.max;
            info->range.f32.step = (float)var_p->range.step;
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


enum awemgr_rc awemgr_comm_rc_to_mgr_rc(int comm_rc)
{
    enum awemgr_rc rc;

    switch (comm_rc)
    {
        case AWECOMM_RC_OK:
            rc = awemgr_RC_OK;
            break;
        case AWECOMM_RC_TIMEOUT:
            rc = awemgr_RC_COMM_TIMEOUT;
            break;
        case AWECOMM_RC_FAIL_COMM:
            rc = awemgr_RC_COMM_FAIL;
            break;
        default:
            rc = awemgr_RC_ERR;
            break;
    }
    return rc;
}

enum awemgr_rc safe_transact(struct awecomm_data *comm_p, struct awecmd_st *buf_p, unsigned int *result_buffer_p, unsigned int result_buffer_size_in_words, unsigned int* rx_words_received_p)
{
    int comm_rc = awecomm_transact_on_cmdbuf(comm_p, buf_p);
    if(comm_rc != AWECOMM_RC_OK)
    {
        return awemgr_comm_rc_to_mgr_rc(comm_rc);
    }

    int cmd_rc = awecmd_response_getData(buf_p, result_buffer_p, result_buffer_size_in_words, rx_words_received_p);

    awemgr_tls_error.error_code = buf_p->error_code;
    awemgr_tls_error.error_desc = buf_p->error_desc;

    if(cmd_rc == AWECMD_RC_AWE_ERR)
    {
        AWEMGR_API_LOGE("Could not parse response.");
        return awemgr_RC_AWECORE_ERROR;
    }
    else if(cmd_rc == AWECMD_RC_ERR)
    {
        return awemgr_RC_ERR;
    }
    return awemgr_RC_OK;
}

float awemgr_getCpuPercentage(awemgr_cpuinfo* cpu_p)
{
    return 100.0f * (float)(unsigned)(cpu_p->AverageCycles>>8) / (float)(unsigned)(cpu_p->TimePerProcess>>8);
}

enum awemgr_rc get_awe_instance_ids(struct awemgr_data* mgr_p, unsigned int *nr_awe_instances_p, unsigned int** instance_numbers_pp)
{
    static unsigned int rx_buffer[16];  // this is also the result buffer

    *nr_awe_instances_p = 0;

    struct awecmd_st *buf_p = NULL;

    AWEMGR_FAIL_ON_ACQUIRE_BUFFER(awecomm_get_cmdbuf(mgr_p->comm_2_awe, AWEMGR_CHANNEL_0, &buf_p));

    // first... get number of CPU cores the system supports
    awecmd_getNrCores(buf_p, 0, 0);

    // the response is encoded into rx_buffer (1: nr of cores, 2++: instance nr )
    unsigned int rx_words_received;
    enum awemgr_rc rc = safe_transact(mgr_p->comm_2_awe, buf_p, rx_buffer, sizeof(rx_buffer) / sizeof(rx_buffer[0]), &rx_words_received);

    (void) awecomm_release_lock(mgr_p->comm_2_awe);

    if(rc != awemgr_RC_OK)
    {
        AWEMGR_API_LOGE("Could not request number of cores.");
        return rc;
    }

    *nr_awe_instances_p = rx_buffer[0];
    *instance_numbers_pp = &rx_buffer[1];

    return awemgr_RC_OK;
}

float fract32_to_float(int v)
{
    // 4.6566128731e-10f  ==  1 / 2^31  ==  1 / 2147483648.0f
    return (float)v * 4.6566128731e-10f;
}

float fract16_to_float(int v)
{
    // 3.0517578125e-05f  ==  1 / 2^15  ==  1 / 32768.0f
    return (float)v * 3.0517578125e-05f;
}

int32_t float_to_fract32(float f)
{
    if (f >= 1.0f) f = 0.999999999f;
    if (f < -1.0f) f = -1.0f;
    return (int32_t)(f * 2147483648.0f); // 2^31
}

int16_t float_to_fract16(float f)
{
    if (f >= 1.0f) f = 0.999969482f;
    if (f < -1.0f) f = -1.0f;

    int32_t raw = (int32_t)roundf(f * 32768.0f); // 2^15

    if (raw > 32767) raw = 32767;
    if (raw < -32768) raw = -32768;

    return raw;
}

// Helper: check integer range and step
enum awemgr_rc check_int_range(int64_t val, awc_ctl_t *ctl_p, unsigned int index)
{
    int64_t min  = (int64_t)ctl_p->range.min;
    int64_t max  = (int64_t)ctl_p->range.max;
    int64_t step = (int64_t)ctl_p->range.step;

    if (val < min || val > max)
    {
        AWEMGR_API_LOGE("Value %" PRId64 " at index %u is outside allowed range [%" PRId64 " .. %" PRId64 "] for control: %s",
                        val, index, min, max, ctl_p->fullname);
        return awemgr_RC_ERR_INVALID_VAL;
    }

    if (step != 0 && ((val - min) % step != 0))
    {
        AWEMGR_API_LOGE("Value %" PRId64 " at index %u is not aligned to step %" PRId64 " for control: %s",
                        val, index, step, ctl_p->fullname);
        return awemgr_RC_ERR_INVALID_VAL;
    }

    return awemgr_RC_OK;
}

// Helper: check float range and step
enum awemgr_rc check_float_range(double val, awc_ctl_t *ctl_p, unsigned int index)
{
    double min  = ctl_p->range.min;
    double max  = ctl_p->range.max;
    double step = ctl_p->range.step;

    if (val < min || val > max)
    {
        AWEMGR_API_LOGE("Value %f at index %u is outside allowed range [%f .. %f] for control: %s",
                        val, index, min, max, ctl_p->fullname);
        return awemgr_RC_ERR_INVALID_VAL;
    }

    if (step != 0.0)
    {
        double step_abs = fabs(step);
        double rel = val - min;
        double rem = fmod(rel, step);

        if (rem < 0) rem += step_abs;

        double eps = step_abs * 1e-6;
        switch (ctl_p->type) {
            case AWC_CTL_FRACT16:
                eps = fmax(1.0 / 32768.0, step_abs * 1e-5); // covers rounding error for Q1.15
                break;
            case AWC_CTL_FRACT32:
                eps = fmax(1.0 / 2147483648.0, step_abs * 1e-6); // covers rounding error for Q1.31
                break;
            default:
                break;
        }

        if (rem > eps && fabs(rem - step_abs) > eps)
        {
            AWEMGR_API_LOGE("Value %f at index %u is not aligned to step %f for control: %s",
                            val, index, step, ctl_p->fullname);
            return awemgr_RC_ERR_INVALID_VAL;
        }
    }

    return awemgr_RC_OK;
}

// --- Helper: Perform range check for a single value ---
enum awemgr_rc check_value_range(void *data, unsigned int index, awc_ctl_t *ctl_p)
{
    switch (ctl_p->type)
    {
        case AWC_CTL_BOOL:
        case AWC_CTL_INT32:
        case AWC_CTL_UINT32:
        {
            int64_t val = (ctl_p->type == AWC_CTL_UINT32) ?
                          (int64_t)((uint32_t*)data)[index] :
                          (int64_t)((int32_t*)data)[index]; // BOOL stored as int32_t
            return check_int_range(val, ctl_p, index);
        }

        case AWC_CTL_FLOAT:
        {
            double val = (double)((float*)data)[index];
            return check_float_range(val, ctl_p, index);
        }

        case AWC_CTL_FRACT32:
        {
            int raw_val = ((int32_t*)data)[index];
            double val = (double)fract32_to_float(raw_val);
            return check_float_range(val, ctl_p, index);
        }

        case AWC_CTL_FRACT16:
        {
            int raw_val = ((int32_t*)data)[index];
            double val = (double)fract16_to_float(raw_val);
            return check_float_range(val, ctl_p, index);
        }

        case AWC_CTL_ENUM:
        default:
            return awemgr_RC_OK; // No range check
    }
}