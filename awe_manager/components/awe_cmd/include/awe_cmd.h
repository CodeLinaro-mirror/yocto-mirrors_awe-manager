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


#ifndef INCLUSION_GUARD_AWECMD_H
#define INCLUSION_GUARD_AWECMD_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "awe_cmd_types.h"


#define AWECMD_RC_OK        0       /**< return code for ALL good! */
#define AWECMD_RC_ERR       1       /**< an error has occured */
#define AWECMD_RC_AWE_ERR   2       /**< AWECore has returned an errorcode in the response packet */

// todo: keep in sync with enum awemgr_vartype. How? don't like to make this header here public!
enum awecmd_vartype {
    AWECMD_INTEGER, /**< 32bit integer value */
    AWECMD_FLOAT,   /**< 32bit float */
    AWECMD_FRACT    /**< 32bit fractional value Q15 ??? */  // todo: check docs
};


struct awecmd_st
{
    UINT32 *buffer_p;
    UINT32 *wraparound_p;
    bool    wraparound;
    UINT32  tunnelAddress;

    UINT32 *current_p;

    UINT32  words_written;

    UINT32 *response_buffer_p;
    UINT32 *response_buffer_end_p;
    UINT32  response_buffer_size;

    INT32 error_code;               /**< AWE Core Error ID (from Error.h) */
    const char *error_desc;         /**< pointer to error description string */
};

/** helper macro to obtain the size of request buffer in nr of words */
#define AWECMD_REQUESTBUFFER_SZ(ctx_p)  ctx_p->wraparound_p - ctx_p->buffer_p

/** helper macro to return the response buffer size in nr of words */
#define AWECMD_RESPONSEBUFFER_SZ(ctx_p) ctx_p->response_buffer_size

/** helper macro for calculating the coreid out of a control handle */
#define AWE_COREID_FROM_HANDLE(handle)   ((handle & 0x780) >> 7)

/** helper macro for calculating the coreid out of a tunnel Address */
#define AWE_COREID_FROM_TUNNELADDR(tunnelAddr) ((tunnelAddr >> 16)&0xff)

#define SIZE_OF_TUNNEL_HEADER 3

// helper macros to extract from our "coreid-objectid" tunnel address
// the tunnel address is a 32bit value, where the first 16 bits are the core ID and the last 16 bits are the object ID
// actually this should not belong to awe_CMD, it is rather a "protocol" on top of it
#define AWECMD_COREID_FROM_TUNNELADDRESS(tunnel_address)  ((tunnel_address >> 16) & 0xFF)
#define AWECMD_OBJECTID_FROM_TUNNELADDRESS(tunnel_address)  (tunnel_address & 0xffff)

/**
 * allocate a buffer for receiving the binary command data packages, and adjust pointers
 * in the `struct awecmd_st` object
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param nr_words[in] - a command buffer will be allocated with this size (byte size = nr_words * sizeof(UINT32))
 * @param isCircularBuffer[in] - if true, the buffer will be treated as circular, i.e., command data may wrap around
 * @param nr_words_response[in] - a response buffer will be allocated with this size (byte size = nr_words_response * sizeof(UINT32))
 *
 * @returns return value for success or error
 */
int awecmd_init(struct awecmd_st *ctx_p, UINT32 nr_words, bool isCircularBuffer, UINT32 nr_words_response);

/**
 * frees memory allocated before in `awecmd_init`
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 *
 * @returns return value for success or error
 */
int awecmd_exit(struct awecmd_st *ctx_p);

/**
 * re-adjusts the pointers in `struct awecmd_st` object
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 *
 * @returns return value for success or error
 */
int awecmd_reset(struct awecmd_st *ctx_p, UINT32 tunnel_address);

/**
 * reads one AWE command from a stream (file pointer)
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param fp[in] - a handle to an open file (or stream)
 * @param endpointId[in] - selects a specific AWE Instance (0 based index value!);
 *                           will be adjusted to correct bit position in AWE command header internally;
 *                           if negative, the AWB file content will not be modified.
 * @param coreId[in] - CPU core index(0-15), used to address subcanvas module.
 * @param objectId[in] - Object Id of subcanvas Module, 0 for Main Canvas.
 * @returns the number of words obtained from file stream
 */
int awecmd_from_stream(struct awecmd_st *ctx_p, FILE *fp, INT32 endpointId, UINT32 coreId, UINT32 objectId);

/**
 * constructs an AWE tuning buffer to Start the Audio
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_AudioStart(struct awecmd_st *ctx_p, UINT32 endpointId);


/**
 * constructs an AWE tuning buffer to stop the Audio
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_AudioStop(struct awecmd_st *ctx_p, UINT32 endpointId);

/**
 * constructs an AWE tuning buffer to destroy the current layout
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_Destroy(struct awecmd_st *ctx_p, UINT32 endpointId);

/**
 * constructs an AWE tuning buffer to query AWE Core for the information about its instance
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 * @param instanceId[in] - index of the AWE instance (aka CPU core index) (0-15)
 *
 * @returns return value for success or error
 */
int awecmd_getTargetInfo(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId);

// todo: check if enum awecmd_vartype is really needed; would possibly beneficial to know what is behind data?!
int awecmd_setValue(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 handle, const void *value, UINT32 arrayOffset, UINT32 length, enum awecmd_vartype type, bool last);

int awecmd_getValue(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 handle, UINT32 arrayOffset, UINT32 length, bool first);

int awecmd_setModuleStatus(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId, UINT32 objectId, UINT32 status);

int awecmd_getModuleStatus(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId, UINT32 objectId);

int awecmd_getModuleClass(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId, UINT32 objectId);

int awecmd_getNrCores(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId);

int awecmd_getExtendedInfo(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 instanceId);

/**
 * a command to the BSP, and not to AWE-Core
 *
 * This command constructs a tuning message that is directed to the BSP, not to AWECore itself.
 * The semantic (meaning) of the content is to be agreed upon by the integration (authors of BSP
 * and control code, like AWE-Manager). A sub_CMD key is used to define this semantic.
 * Additional sub_CMD specific information is added in a payload, which also can be NULL.
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 * @param coreId[in] - specific core to address (in a multi-processor signal flow) (0 based)
 * @param sub_CMD[in] - integration specific command, defines what the payload looks like or if there is a payload at all
 * @param payload[in] - pointer to a buffer which contains the "parameters" of sub_CMD;
 *                      if the command needs to transfer the data in chunks (eg a huge data is transferred in chunks)
 *                      the flags (eg isLast) are to be encoded in the payload
 * @param length_in_words[int] - size of the payload in number of words
 *
 * @returns return value for success or error
 */
int awecmd_BSPCmd(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId, int sub_CMD, const void *payload, UINT32 length_in_words);

/**
 * Creates command to retrieve the CPU profiling info
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 * @param coreId[in] - specific core to address (in a multi-processor signal flow) (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_getCpuLoad(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId);

/**
 * Creates command to retrieve the number of classes available to AWE-Core
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 * @param coreId[in] - specific core to address (in a multi-processor signal flow) (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_getClassCount(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId);


/**
 * Creates command to retrieve information about a specific class
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 * @param coreId[in] - specific core to address (in a multi-processor signal flow) (0 based)
 * @param index[in] - index of the class information; must be smaller than number returned
 *                    when awecmd_getClassCount() was used
 *
 * @returns return value for success or error
 */
int awecmd_getClassInfo(struct awecmd_st *ctx_p, UINT32 endpointId, UINT32 coreId, UINT32 index);

/**
 * Creates command to retrieve the number of heaps available to AWE-Core
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_getHeapCount(struct awecmd_st *ctx_p, UINT32 endpointId);

/**
 * Creates command to get (standard) heap information
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_getHeapSize(struct awecmd_st *ctx_p, UINT32 endpointId);

/**
 * Creates command to get (shared memory) heap information - used in AWE-Q and other multi-core designs
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param endpointId[in] - index of the AWE design on the system (0 based)
 *
 * @returns return value for success or error
 */
int awecmd_getSharedHeapSize(struct awecmd_st *ctx_p, UINT32 endpointId);

/**
 * generic routine to parse the data buffer returned from AWE Core
 *
 * It is assumed/required that the originally returned buffer from AWE Core is stored internally
 * in the response_buffer_p parameter of ctx_p.
 *
 * @param ctx_p[in] - a pointer to an externally allocated/located `struct awecmd_st` object
 * @param values[in] - points to a data buffer to be filled with the returned data
 * @param value_size_words[in] - size of the `values` buffer (in words)
 * @param words_copied_out[out] - ptr to a scalar integer; will be updated with the number of words written into `values`
 *
 * @returns return value for success or error
 */
int awecmd_response_getData(struct awecmd_st *ctx_p, void *values, unsigned int value_size_words, unsigned int *words_copied_out);


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWECMD_H
