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
#include "awe_cmd.h"
#include "awe_ctrl.h"
#include "awemgr_util.h"    // for eg AWEMGR_FAIL_ON_HANDLE_NULL
#include <string.h>  // for memcpy


/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int  awemgr_get_controls_count(struct awemgr_ctx *ctx_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    UINT32 nr_controls = awc_control_count(ctx_p->awc);
    AWEMGR_API_LOGD("nr of controls: %d ", nr_controls);

    return (int)nr_controls;
}

enum awemgr_rc  awemgr_get_control_info(struct awemgr_ctx *ctx_p, int index, struct awemgr_ctl_elem_info *info)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info);

    awc_ctl_t* ctl_p = awc_get_control_by_index(ctx_p->awc, index);
    if (ctl_p != NULL)
    {
        AWEMGR_API_LOGD(" - [%3d] sz:%5d %8s - %s", index, ctl_p->size, awc_get_typename(ctl_p->type), ctl_p->fullname);
        info->id.instanceId = ctx_p->instanceId;
        copy_ctl_info(ctl_p, info);
    }
    else
    {
        AWEMGR_API_LOGE("Control index outside of range.");
        return awemgr_RC_ERR;
    }

    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_get_control_info_by_name(struct awemgr_ctx *ctx_p, const char *control_name, struct awemgr_ctl_elem_info *info)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(info);

    awc_ctl_t *ctl_p = awc_get_control_from_awc(ctx_p->awc, control_name);
    if (ctl_p == NULL)
    {
        AWEMGR_API_LOGE("Could not find handle for control name: %s", control_name);
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    info->id.instanceId = ctx_p->instanceId;
    copy_ctl_info(ctl_p, info);

    return awemgr_RC_OK;

}

enum awemgr_rc  awemgr_control_write(struct awemgr_ctx *ctx_p,
    const char *control_name,
    unsigned int offset,
    void *data,
    unsigned int data_sz)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_API_LOGI("Write to '%s'", control_name);
    
    awc_ctl_t *ctl_p = awc_get_control_from_awc(ctx_p->awc, control_name);
    if (ctl_p == NULL)
    {
        AWEMGR_API_LOGE("Could not find handle for control name: %s", control_name);
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    // check if offset is in correct range
    if (offset >= ctl_p->size)
    {
        AWEMGR_API_LOGE("Offset too large for control: %s. Only %d words in variable, Offset is at %d",
            control_name, ctl_p->size, offset);
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    // sanity check if amount of data would fit into the variable buffer (starting from offset)
    if (data_sz > (ctl_p->size - offset))
    {
        AWEMGR_API_LOGE("Provided data too big for control: %s. %d words to write, %d available",
            control_name, data_sz, (ctl_p->size - offset));
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    if (ctx_p->parent->range_check_enabled && ctl_p->checkRange)
    {
        for (unsigned int i = 0; i < data_sz; i++)
        {
            enum awemgr_rc rc = check_value_range(data, i, ctl_p);
            if (rc != awemgr_RC_OK)
                return rc;
        }
    }
    int tunnel_address = ctl_p->module->tunnelAddress;
    struct awecmd_st *buf_p;
    (void)awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnel_address, &buf_p);

    unsigned int nr_words_in_buf = (unsigned int) (AWECMD_REQUESTBUFFER_SZ(buf_p) - 6);
    unsigned int nr_words_left = data_sz;

    unsigned int *wData_p = (unsigned int*)data;
    while(nr_words_left > 0)
    {
        unsigned int nr_words_chunk = (nr_words_left > nr_words_in_buf) ? nr_words_in_buf : nr_words_left;
        bool last = nr_words_left <= nr_words_in_buf ? true : false;

        awecmd_setValue(buf_p, (tunnel_address != 0) ? 0 : ctx_p->instanceId, ctl_p->handle, wData_p, offset, nr_words_chunk, last);

        int rc = awectrl_transact(ctx_p->parent->comm_2_awe, (tunnel_address != 0) ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            if(rc == AWECTRL_RC_TIMEOUT)
            {
                return awemgr_RC_COMM_TIMEOUT;
            }
            return awemgr_RC_ERR;
        }

        rc = awecmd_response_getData(buf_p, NULL, 0, NULL);

        COPY_AWE_ERROR(ctx_p->parent, buf_p);
        if(rc == AWECMD_RC_AWE_ERR)
        {
            AWEMGR_API_LOGE("AWE Core returned error when reading from: %s.", control_name);
            return awemgr_RC_AWECORE_ERROR;
        }
        else if(rc == AWECMD_RC_ERR)
        {
            return awemgr_RC_ERR;
        }

        nr_words_left -= nr_words_chunk;
        offset += nr_words_chunk;
        wData_p += nr_words_chunk;
    }
    return awemgr_RC_OK;
}

enum awemgr_rc  awemgr_control_read_partial(struct awemgr_ctx *ctx_p,
    const char *control_name,
    unsigned int  offset,
    unsigned int  nr_words_to_read,
    void *response_buffer,
    unsigned int response_buffer_size,
    unsigned int *response_buffer_filled,
    enum awemgr_vartype *type_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    AWEMGR_API_LOGI("Read from '%s'", control_name);
    awc_ctl_t *ctl_p = awc_get_control_from_awc(ctx_p->awc, control_name);
    if (ctl_p == NULL)
    {
        AWEMGR_API_LOGE("Could not find handle for control name: %s", control_name);
        return awemgr_RC_ERR;  // todo: add correct error code
    }

    // sanity check if response buffer can receive data wanted
    if (response_buffer_size < nr_words_to_read)
    {
        AWEMGR_API_LOGE("Provided response buffer too small for control: %s. %d words wanted, %d provided",
            control_name, nr_words_to_read, response_buffer_size);
        return awemgr_RC_ERR;  // todo: add correct error code
    }
    // check if someone wants to read more words than available in control;
    // be so nice to at least warn about this
    if (nr_words_to_read > (ctl_p->size - offset))
    {
        AWEMGR_API_LOGW("Trying to read more words from control: %s has %d words left at offset %u, %d wanted. Truncating!",
            control_name, (ctl_p->size - offset), offset, response_buffer_size);
        nr_words_to_read = (ctl_p->size - offset);
    }

    int tunnel_address = ctl_p->module->tunnelAddress;
    struct awecmd_st *buf_p;
    // no err check needed, as we can be sure we have correct params here
    awectrl_get_cmdbuf(ctx_p->parent->comm_2_awe, tunnel_address, &buf_p);

    unsigned int nr_words_in_buf = AWECMD_RESPONSEBUFFER_SZ(buf_p) - 4; // todo: check 4 ! sz, errCode, CRC, ...?
    int current_offset = offset;  // CHECK HOW TO HANDLE ctl_p->offset;
    unsigned int words_received = 0;
    int output_offset = 0;
    bool first = true;
    while (nr_words_to_read > 0)
    {
        unsigned int nr_words_chunk = (nr_words_to_read > nr_words_in_buf) ? nr_words_in_buf : nr_words_to_read;

        awecmd_getValue(buf_p, (tunnel_address != 0) ? 0 : ctx_p->instanceId, ctl_p->handle, current_offset, nr_words_chunk, first);

        int rc = awectrl_transact(ctx_p->parent->comm_2_awe, tunnel_address ? AWEMGR_CHANNEL_1 : AWEMGR_CHANNEL_0);
        if(rc != AWECTRL_RC_OK)
        {
            if(rc == AWECTRL_RC_TIMEOUT)
            {
                return awemgr_RC_COMM_TIMEOUT;
            }
            return awemgr_RC_ERR;
        }

        rc = awecmd_response_getData(buf_p, &((unsigned int*)response_buffer)[output_offset], response_buffer_size, response_buffer_filled);

        COPY_AWE_ERROR(ctx_p->parent, buf_p);
        if(rc == AWECMD_RC_AWE_ERR)
        {
            AWEMGR_API_LOGE("AWECore returned error for: %s", control_name);
            return awemgr_RC_AWECORE_ERROR;
        }
        else if(rc == AWECMD_RC_ERR)
        {
            return awemgr_RC_ERR;
        }
        first = false;
        nr_words_to_read -= nr_words_chunk;
        output_offset += nr_words_chunk;
        current_offset += nr_words_chunk;
        words_received += *response_buffer_filled;
    }
    *response_buffer_filled = words_received;

    if (type_p != NULL)
    {
        switch(ctl_p->type)
        {
            case AWC_CTL_FLOAT:
                *type_p = AWEMGR_VARTYPE_FLOAT;
                break;
            case AWC_CTL_FRACT32:
                *type_p = AWEMGR_VARTYPE_FRACT32;
                break;
            case AWC_CTL_FRACT16:
                *type_p = AWEMGR_VARTYPE_FRACT16;
                break;
            case AWC_CTL_UINT32:
                *type_p = AWEMGR_VARTYPE_UNSIGNED_INTEGER;
                break;
            case AWC_CTL_ENUM:
                *type_p = AWEMGR_VARTYPE_ENUM;
                break;
            case AWC_CTL_INT32:
            case AWC_CTL_BOOL:
                *type_p = AWEMGR_VARTYPE_INTEGER;
                break;
            case AWC_CTL_UNKNOWN:
            default:
                AWEMGR_API_LOGE("Unknown ctl_p->type: %d", ctl_p->type);  // we should not be able to get here: how to avoid compiler warning of uused type
                break;
        }
    }
    return awemgr_RC_OK;
}


enum awemgr_rc  awemgr_control_read(struct awemgr_ctx *ctx_p,
    const char *control_name,
    void *response_buffer,
    unsigned int response_buffer_size,
    unsigned int *response_buffer_filled,
    enum awemgr_vartype *type_p)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);

    int sz_of_ctl_variable = 0;
    awc_ctl_t *ctl_p = awc_get_control_from_awc(ctx_p->awc, control_name);
    if (ctl_p != NULL)
    {
        sz_of_ctl_variable = ctl_p->size;
    }
    // note: deliberately deferring the error handling of un-available control_name to partial fct!

    return awemgr_control_read_partial(ctx_p, control_name, 0, sz_of_ctl_variable,
        response_buffer, response_buffer_size, response_buffer_filled, type_p);
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS - USER DATA
 * ***************************************************************************/

int awemgr_get_control_userdata_count(struct awemgr_ctx *ctx_p, const char* controlname)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    return awc_get_control_userdata_count(ctx_p->awc, controlname);
}


enum awemgr_rc awemgr_get_control_userdata_by_index(struct awemgr_ctx *ctx_p, const char* controlname, uint32_t index, awemgr_userdata* data)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(data);
    const awc_dict_element* elem_p =  awc_get_control_userdata_by_index(ctx_p->awc, controlname, index);
    if(elem_p != NULL)
    {
        awemgr_copy_usrdata(elem_p, data);
        return awemgr_RC_OK;
    }
    return awemgr_RC_ERR;
}


enum awemgr_rc awemgr_get_control_userdata_by_key(struct awemgr_ctx *ctx_p, const char* controlname, const char* key, awemgr_userdata* data)
{
    AWEMGR_FAIL_ON_HANDLE_NULL(ctx_p);
    AWEMGR_FAIL_ON_HANDLE_NULL(key);
    AWEMGR_FAIL_ON_HANDLE_NULL(data);
    const awc_dict_element* elem_p =  awc_get_control_userdata_by_key(ctx_p->awc, controlname, key);
    if(elem_p != NULL)
    {
        awemgr_copy_usrdata(elem_p, data);
        return awemgr_RC_OK;
    }
    return awemgr_RC_ERR;
}

