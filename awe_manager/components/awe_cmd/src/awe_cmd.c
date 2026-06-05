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


#include "awe_cmd.h"
#include "awe_cmd_logging.h"
#define DEFINE_ERROR_STRINGS
#include "Errors.h"
// also include the PFID list of AWE Core - note: we extend it further below (PFID_BspCmd) !!!
#include "ProxyIDs.h"

#include <stdlib.h>
#include <string.h>

/*
todo: try to make this leaner than existing COPY/PASTE "structures",

implement ptr increment rather than setting stuff in sendBuffer and do memcpy(),

if using a wraparound_p pointer also circular buffers are supported

*/
/* ****************************************************************************
 * Function Declarations
 * ***************************************************************************/
UINT32 ComputeCRC(UINT32 *pMessage, UINT32 len);

/* ****************************************************************************
 * MACROS
 * ***************************************************************************/
#define AWECMD_FAIL_ON_HANDLE_INCORRECT(c)    if (!c) {            \
         AWE_CMD_LOGE("Invalid argument: handle == NULL!");        \
         return AWECMD_RC_ERR;                                     \
     }
// todo: incorrect param!

#define AWECMD_P_n_S(ctx_p)  UINT32 *p = ctx_p->tunnelAddress == 0 ? ctx_p->buffer_p : ctx_p->buffer_p + SIZE_OF_TUNNEL_HEADER; UINT32 *s = p; UINT32 cnt = 0;

#define AWECMD_MSG_CANVASID(endPointId)    (endPointId << 4)

#define AWECMD_INSTANCEID_0 (0)
#define AWECMD_MSG_CANVAS_AND_INSTANCEID(endPoint, instance)   (AWECMD_MSG_CANVASID(endPoint) | instance)

#define AWECMD_MSG_ADD_HEADER(pfid_code, instance, endPoint)  *p++ = (AWECMD_MSG_CANVAS_AND_INSTANCEID(endPoint, instance)  << 8) | pfid_code; cnt++;

#define AWECMD_MSG_ADD_VALUE(val)  *p++ = val; cnt++;
#define AWECMD_MSG_ADD_HANDLE(val)  AWECMD_MSG_ADD_VALUE(val & 0x3ffff07f)
#define AWECMD_MSG_HEADER_SET_LENGTH(txSize)  *s = (txSize << 16) | *s
#define AWECMD_MSG_HEADER_GET_LENGTH          (*s >> 16) & 0xffff
#define AWECMD_MSG_HEADER_GET_PFID            (*s & 0xff)
#define AWECMD_MSG_ADD_CRC                                                  \
    if (ctx_p->tunnelAddress != 0 && s != ctx_p->buffer_p)                                          \
    {                                                                       \
        UINT32 pkt_size = AWECMD_MSG_HEADER_GET_LENGTH;                     \
        s = ctx_p->buffer_p;                                                \
        AWECMD_MSG_HEADER_SET_LENGTH((pkt_size + SIZE_OF_TUNNEL_HEADER));   \
        cnt += SIZE_OF_TUNNEL_HEADER;                                       \
    }                                                                       \
    *p++ = ComputeCRC(s, AWECMD_MSG_HEADER_GET_LENGTH);                     \
    ++cnt;


// generic command to the BSP, not to AWE-Core
// !!! defined here now and not in official AWE-Core include file
// !!! to be aligned with BSP development!
#define PFID_BSPCmd                             200

// "borrow" from AWECoreUtils.h
#define PACKET_LENGTH_WORDS(x) (x[0]>>16)
#define PACKET_LENGTH_BYTES(x) ((x[0]>>16) * sizeof(x[0]))
#define PACKET_INSTANCEID(x) (x[0] >> 8) & 0xff
#define PACKET_OPCODE(x) ((INT32)x[0] & 0xffU)


/* ****************************************************************************
 * LOCAL FUNCTIONS
 * ***************************************************************************/

static inline UINT32 getMaskFromHandle(UINT32 handle)
{
    UINT32 variable_offset = (handle & 0x7f);   // Ideally this should be (handle & 0x1f);
    if (variable_offset > 31) {
        variable_offset = 31;
    }
    return (1 << variable_offset);
}

static inline int cmd_base(struct awecmd_st *ctx_p, UINT32 pfid, UINT32 endpointId, UINT32 coreId)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    AWECMD_P_n_S(ctx_p);
    AWECMD_MSG_ADD_HEADER(pfid, coreId, endpointId);
    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}

UINT32 ComputeCRC(UINT32 *pMessage, UINT32 len)
{
    UINT32 crc = 0;
    if((pMessage != NULL) && (len != 0))
    {
        for (UINT32 i = 0; i < (len - 1); ++i)
        {
            crc ^= pMessage[i];
        }
    }
    return crc;
}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int awecmd_init(struct awecmd_st *ctx_p, UINT32 nr_words, bool isCircularBuffer, UINT32 nr_words_response)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    ctx_p->buffer_p = calloc(nr_words + SIZE_OF_TUNNEL_HEADER, sizeof(UINT32));
    if (ctx_p->buffer_p == NULL)
    {
        return AWECMD_RC_ERR;
    }
    ctx_p->wraparound_p = ctx_p->buffer_p + nr_words;
    ctx_p->wraparound = isCircularBuffer;
    ctx_p->tunnelAddress = 0;
    awecmd_reset(ctx_p, 0);

    ctx_p->response_buffer_p = calloc(nr_words_response, sizeof(UINT32));
    ctx_p->response_buffer_end_p = ctx_p->response_buffer_p + nr_words_response;
    ctx_p->response_buffer_size = nr_words_response;

    return AWECMD_RC_OK;
}


int awecmd_exit(struct awecmd_st *ctx_p)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    free(ctx_p->buffer_p);
    ctx_p->buffer_p = NULL;

    free(ctx_p->response_buffer_p);
    ctx_p->response_buffer_p = NULL;

    return AWECMD_RC_OK;
}


int awecmd_reset(struct awecmd_st *ctx_p, UINT32 tunnel_address)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);
    ctx_p->current_p = ctx_p->buffer_p;
    ctx_p->tunnelAddress = tunnel_address;
    if (ctx_p->tunnelAddress != 0)
    {
        uint8_t coreId = AWECMD_COREID_FROM_TUNNELADDRESS(ctx_p->tunnelAddress);
        uint32_t objectId = AWECMD_OBJECTID_FROM_TUNNELADDRESS(ctx_p->tunnelAddress);

        UINT32 *p = ctx_p->buffer_p;
        UINT32 cnt = 0;
        AWECMD_MSG_ADD_HEADER(PFID_BundlePackets, coreId, AWECMD_INSTANCEID_0);  // TODO: check =0 instance???
        AWECMD_MSG_ADD_VALUE(objectId);
        *p++ = 0;  // we do not bundle packets, so set to 0
        ctx_p->current_p = p;  // skip tunnel header for others (+SIZE_OF_TUNNEL_HEADER)
    }
    return AWECMD_RC_OK;
}

int awecmd_AudioStart(struct awecmd_st *ctx_p, UINT32 endpointId)
{
    return cmd_base(ctx_p, PFID_StartAudio, endpointId, AWECMD_INSTANCEID_0);
}

int awecmd_AudioStop(struct awecmd_st *ctx_p, UINT32 endpointId)
{
    return cmd_base(ctx_p, PFID_StopAudio, endpointId, AWECMD_INSTANCEID_0);
}

int awecmd_Destroy(struct awecmd_st *ctx_p, UINT32 endpointId)
{
    return cmd_base(ctx_p, PFID_Destroy, endpointId, AWECMD_INSTANCEID_0);
}

int awecmd_from_stream(struct awecmd_st *ctx_p, FILE *fp, INT32 endpointId, UINT32 coreId, UINT32 objectId)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);
    if(fp == NULL)
        return AWECMD_RC_ERR;  // incorrect param!

    AWECMD_P_n_S(ctx_p);

    ctx_p->words_written = 0;

    // read 1 word which must be a MSG header
    // todo: add some check to make sure we read from AWB and not from something else?!
    size_t nRead = fread(p++, sizeof(UINT32), 1, fp);

    if (nRead > 0) {
        // obtain message size, minimum size is 1 word.
        // beware, this size assumes CRC is included!
        cnt++;

        if (endpointId >= 0) {
            *s = *s | (AWECMD_MSG_CANVASID((UINT32) endpointId) << 8);
        }

        int curCmdSz = AWECMD_MSG_HEADER_GET_LENGTH;
        if (curCmdSz == 0)
        {
            // EOF marker reached
            return 0;
        }
        uint8_t pfid = AWECMD_MSG_HEADER_GET_PFID;
        if(pfid == PFID_BundlePackets && objectId != 0)
        {
            // roll back the s and p pointers to start of the buffer and buffer + 1.
            s = ctx_p->buffer_p;
            p = s + 1;
        }
        // the payload size (additional words to read) is therefore to be reduced by 2
        int payLoadSz = curCmdSz - 2;
        if (payLoadSz > 0) {
            size_t sz = fread(p, sizeof(UINT32), payLoadSz, fp);
            cnt += payLoadSz;
            p += payLoadSz;  // todo: VIOLATES CRC BUFFER!
            if (sz != (size_t) payLoadSz) {
                AWE_CMD_LOGE("Error reading from stream, read %zu words, expected %d words\n", sz, payLoadSz);
                return AWECMD_RC_ERR;
            }
        }

        if(pfid == PFID_BundlePackets)
        {
            if(objectId != 0)
            {
                s[0] = (curCmdSz << 16) | (AWECMD_MSG_CANVAS_AND_INSTANCEID(endpointId, coreId)  << 8) | pfid;
                s[1] = objectId;
            }
        }

        AWECMD_MSG_ADD_CRC;
        ctx_p->words_written = cnt;
    }
    return ctx_p->words_written;
}

int awecmd_getTargetInfo(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId)
{
    return cmd_base(ctx_p, PFID_GetTargetInfo, endpointId, instanceId);
}

int awecmd_setValue(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 handle, const void *value, UINT32 arrayOffset, UINT32 length, bool last)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);
    bool isArray = handle & 0x80000000;

    AWECMD_P_n_S(ctx_p);
    UINT32 mask = getMaskFromHandle(handle);
    if (isArray) {
        int pfid = mask ? PFID_SetValuesSetCall : PFID_SetValues;
        AWECMD_MSG_ADD_HEADER(pfid, AWE_COREID_FROM_HANDLE(handle), endpointId);
        AWECMD_MSG_ADD_HANDLE(handle);
        AWECMD_MSG_ADD_VALUE(arrayOffset);
        if (mask != 0)
        {
            AWECMD_MSG_ADD_VALUE(last ? mask : 0);
        }
        AWECMD_MSG_ADD_VALUE(length);

        UINT32 *value_uint32p = (UINT32*) value;
        for (UINT32 i = 0; i < length; i++)
        {
            AWECMD_MSG_ADD_VALUE(*value_uint32p++);
        }
    }
    else
    {
        int pfid = mask ? PFID_SetValueSetCall : PFID_SetValue;
        AWECMD_MSG_ADD_HEADER(pfid, AWE_COREID_FROM_HANDLE(handle), endpointId);
        AWECMD_MSG_ADD_HANDLE(handle);
        AWECMD_MSG_ADD_VALUE(*((UINT32 *)value));
        if (mask != 0)
        {
            AWECMD_MSG_ADD_VALUE(last ? mask : 0);
        }
        AWECMD_MSG_ADD_VALUE(0);
    }

    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;

}

int awecmd_getValue(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 handle, UINT32 arrayOffset, UINT32 length, bool first)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);
    bool isArray = handle & 0x80000000;
    // todo: more checks regarding length (length + 3 > COMMAND_BUFFER_LENGTH)

    AWECMD_P_n_S(ctx_p);
    int mask = 0;
    if (first)
    {
        mask = getMaskFromHandle(handle);
    }
    if (isArray) {
        AWECMD_MSG_ADD_HEADER(PFID_GetCallFetchValues, AWE_COREID_FROM_HANDLE(handle), endpointId);
        AWECMD_MSG_ADD_HANDLE(handle);
        AWECMD_MSG_ADD_VALUE(arrayOffset);
        AWECMD_MSG_ADD_VALUE(mask);
        AWECMD_MSG_ADD_VALUE(length);
    } else {
        AWECMD_MSG_ADD_HEADER(PFID_FetchValue, AWE_COREID_FROM_HANDLE(handle), endpointId);
        AWECMD_MSG_ADD_HANDLE(handle);
        AWECMD_MSG_ADD_VALUE(0);
    }

    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}

int awecmd_setModuleStatus(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId, UINT32 objectId, UINT32 status)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    AWECMD_P_n_S(ctx_p);
    AWECMD_MSG_ADD_HEADER(PFID_SetModuleState, instanceId, endpointId);
    AWECMD_MSG_ADD_VALUE(objectId);
    AWECMD_MSG_ADD_VALUE(status);
    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}

int awecmd_getModuleStatus(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId, UINT32 objectId)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    AWECMD_P_n_S(ctx_p);
    AWECMD_MSG_ADD_HEADER(PFID_GetModuleState, instanceId, endpointId);
    AWECMD_MSG_ADD_VALUE(objectId);
    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}

int awecmd_getModuleClass(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId, UINT32 objectId)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    AWECMD_P_n_S(ctx_p);
    AWECMD_MSG_ADD_HEADER(PFID_GetObjectByID, instanceId, endpointId);
    AWECMD_MSG_ADD_VALUE(objectId);
    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}

static bool ignore_error_code_for_pfid(INT32 error_code, UINT32 pfid)
{
    static const struct { INT32 error_code; UINT32 pfid; } suppress[] = {
        { E_AUDIO_ALREADY_STARTED, PFID_StartAudio },
        { E_AUDIO_ALREADY_STOPPED, PFID_StopAudio  },
        { E_NO_LAYOUTS, PFID_GetProfileValues  },
        { E_NO_LAYOUTS, PFID_GetAllProfiling  }
    };
    for (size_t i = 0; i < sizeof(suppress) / sizeof(suppress[0]); i++)
    {
        if (suppress[i].error_code == error_code && suppress[i].pfid == pfid)
            return true;
    }
    return false;
}

/* method to parse the returned AWE-Core response buffer
 * it copies out the payload data of the returned data but does a central error checking before
 */
int awecmd_response_getData(struct awecmd_st *ctx_p, void *values, unsigned int value_size_words, unsigned int *words_copied_out)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    // retrieve the error code as an integer value:
    // as the protocol foresees the error code in various positions, depending on the PFID used
    // for the last command, this ugly switch/case construction is required
    //

    // just make sure that in error cases no one thinks he got data
    if (words_copied_out)
        *words_copied_out = 0;

    // first find out which offset into the buffer contains the error code
    int idx_2_tuning_header = (ctx_p->tunnelAddress != 0) ? SIZE_OF_TUNNEL_HEADER : 0;
    UINT32  last_pfid = ctx_p->buffer_p[idx_2_tuning_header] & 0xff;
    unsigned int  packet_size = PACKET_LENGTH_WORDS(ctx_p->response_buffer_p);
    if (packet_size < 3 || packet_size > ctx_p->response_buffer_size) {
        AWE_CMD_LOGE("ResponseBuffer yields strange packet size. packet_size=%u for PFID=%u", packet_size, last_pfid);
        return AWECMD_RC_ERR;
    }
    UINT32* data_ptr = &ctx_p->response_buffer_p[2];  // where does the "real" data start
    UINT32* error_code_ptr = &ctx_p->response_buffer_p[1]; // at which position is the AWE-Core error code
    UINT32  payload_sz_in_words = packet_size - 3;  // minus: 1. length, 2. err_code, 3. CRC;

    switch (last_pfid)
    {
        case PFID_ClassModule_Constructor:
        case PFID_ClassWire_Constructor:
        case PFID_ClassLayout_Constructor:
            error_code_ptr = &ctx_p->response_buffer_p[2];
            data_ptr = &ctx_p->response_buffer_p[1];
            payload_sz_in_words = 1;
            break;
        case PFID_GetCores2:
            error_code_ptr = &ctx_p->response_buffer_p[1];
            data_ptr = &ctx_p->response_buffer_p[1];
            payload_sz_in_words = ctx_p->response_buffer_p[1] + 1;  // copy nr instances + itself
            break;
        case PFID_GetCIModuleCount:
            error_code_ptr = &ctx_p->response_buffer_p[1];
            data_ptr = &ctx_p->response_buffer_p[1];
            payload_sz_in_words = 1;
            break;
        case PFID_GetCIModuleInfo:
            error_code_ptr = &ctx_p->response_buffer_p[1];
            data_ptr = &ctx_p->response_buffer_p[2];
            if (*error_code_ptr == *data_ptr)
                error_code_ptr = NULL;
            payload_sz_in_words = 2;
            break;
        case PFID_GetHeapCount:
            data_ptr = &ctx_p->response_buffer_p[1];
            payload_sz_in_words = 1;
            break;
        case PFID_GetModuleState:
            error_code_ptr = &ctx_p->response_buffer_p[1];
            data_ptr = &ctx_p->response_buffer_p[1];
            payload_sz_in_words = 1;
            break;
        default:
            break;
    }

    // then check for error condition on the returned response
    // all AWE-Core error codes are negative numbers! Hence the comparison < E_SUCCESS
    ctx_p->error_code = error_code_ptr ? (INT32)*error_code_ptr : E_SUCCESS;
    if(ctx_p->error_code > E_SUCCESS)
    {
        // likely the error code and data is stored in the same word. e.g PFID_GetCores2 / PFID_GetModuleState
        ctx_p->error_code = E_SUCCESS;
    }
    UINT32 error_index = (UINT32)(-(INT32)ctx_p->error_code);
    if(error_index < sizeof(s_error_strings)/sizeof(s_error_strings[0]))
    {
        ctx_p->error_desc = s_error_strings[error_index];
    }
    else
    {
        ctx_p->error_desc = "Unknown Error";
    }
    if (ctx_p->error_code < E_SUCCESS)
    {
        if (! ignore_error_code_for_pfid(ctx_p->error_code, last_pfid))
        {
            AWE_CMD_LOGE("ResponseBuffer contains error! errcode=%d for PFID=%u : %s", ctx_p->error_code, last_pfid, ctx_p->error_desc);
            if (words_copied_out)
                *words_copied_out = 0;
            return AWECMD_RC_AWE_ERR;
        }
        else
        {
            AWE_CMD_LOGW("ResponseBuffer contains return value %d for PFID=%u (%s) but it is ignored due to PFID! Returning ok.", ctx_p->error_code, last_pfid, ctx_p->error_desc);
            ctx_p->error_code = E_SUCCESS;  // reset error code to success, as we ignore this error code for this PFID
            ctx_p->error_desc = s_error_strings[E_SUCCESS];
        }
    }

    // finally copy out the received data to the recipient
    if (values != NULL)
    {
        int nr_bytes_out = payload_sz_in_words * sizeof(UINT32);

        if (payload_sz_in_words > value_size_words)
        {
            AWE_CMD_LOGW("Response buffer size too small for received data. "
                "AWE response payload: %d, Target buffer: %d", payload_sz_in_words, value_size_words);
            nr_bytes_out = value_size_words * sizeof(UINT32);
            payload_sz_in_words = value_size_words;
        }
        memcpy(values, data_ptr, nr_bytes_out);
    }

    if (words_copied_out != NULL)
        // when there is a recipient for the amount of words copied out, update it
        // but check first if we have copied something
        *words_copied_out = values ? payload_sz_in_words : 0;

    // reset buffer to sure we do not ("accidentally") read it a second time :)
    ctx_p->response_buffer_p[0] = 0;

    return AWECMD_RC_OK;
}

int awecmd_getNrCores(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId)
{
    return cmd_base(ctx_p, PFID_GetCores2, endpointId, instanceId);
}

int awecmd_getExtendedInfo(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId)
{
    return cmd_base(ctx_p, PFID_GetExtendedInfo, endpointId, instanceId);
}

int awecmd_BSPCmd(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId, int sub_CMD, const void *payload, UINT32 length_in_words)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);
    AWECMD_P_n_S(ctx_p);

    // this command is currently only implemented for multi-core systems (single canvas)
    AWECMD_MSG_ADD_HEADER(PFID_BSPCmd, coreId, endpointId);

    AWECMD_MSG_ADD_VALUE(sub_CMD);
    AWECMD_MSG_ADD_VALUE(length_in_words);
    UINT32 *value_uint32p = (UINT32*) payload;
    for (UINT32 i = 0; i < length_in_words; i++)
    {
        AWECMD_MSG_ADD_VALUE(*value_uint32p++);
    }

    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;

}

int awecmd_getClassCount(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId)
{
    return cmd_base(ctx_p, PFID_GetCIModuleCount, endpointId, coreId);
}

int awecmd_getClassInfo(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId, UINT32 index)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    AWECMD_P_n_S(ctx_p);
    AWECMD_MSG_ADD_HEADER(PFID_GetCIModuleInfo, coreId, endpointId);
    AWECMD_MSG_ADD_VALUE(index);
    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}

int awecmd_getHeapCount(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId)
{
    return cmd_base(ctx_p, PFID_GetHeapCount, endpointId, coreId);
}

int awecmd_getHeapSize(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId)
{
    return cmd_base(ctx_p, PFID_GetHeapSize, endpointId, coreId);
}

int awecmd_getSharedHeapSize(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId)
{
    return cmd_base(ctx_p, PFID_GetSharedHeapSize, endpointId, coreId);
}

int awecmd_getCpuLoad(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId)
{
    return cmd_base(ctx_p, PFID_GetProfileValues, endpointId, coreId);
}

int awecmd_getAllProfilingData(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId, UINT32 layoutIdx, UINT32 maxLayouts)
{
    AWECMD_FAIL_ON_HANDLE_INCORRECT(ctx_p);

    AWECMD_P_n_S(ctx_p);
    AWECMD_MSG_ADD_HEADER(PFID_GetAllProfiling, coreId, endpointId);
    AWECMD_MSG_ADD_VALUE(layoutIdx);
    AWECMD_MSG_ADD_VALUE(maxLayouts);
    AWECMD_MSG_HEADER_SET_LENGTH((cnt + 1));  // double () to have cnt+1 evaluated first!
    AWECMD_MSG_ADD_CRC;
    ctx_p->words_written = cnt;
    return AWECMD_RC_OK;
}


int awecmd_resetState(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId)
{
    return cmd_base(ctx_p, PFID_ResetState, endpointId, coreId);
}
