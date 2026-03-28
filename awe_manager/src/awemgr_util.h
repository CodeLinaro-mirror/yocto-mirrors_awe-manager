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


#ifndef INCLUSION_GUARD_AWEMGR_UTIL_H
#define INCLUSION_GUARD_AWEMGR_UTIL_H

#include "awe_awc.h"     // for awc_ctl_t and type enums
#include "awe_cmd.h"     // for awecmd_st
#include "awe_ctrl.h"    // for awectrl_data
#include "awe_manager.h" // for awemgr_ctl_elem_info

#include "awemgr_api_logging.h"

#if defined(__cplusplus)
extern "C" {
#endif


// helper macro to check for a NULL handle parameter,
// bails out and reports error
#define AWEMGR_FAIL_ON_HANDLE_NULL(c)    if (!c) {        \
     AWEMGR_API_LOGE("Invalid argument: handle == NULL!: %s", #c);    \
     return awemgr_RC_ERR;                                \
     }


#define COPY_AWE_ERROR(MGRDATA, AWECMD_ST)  \
     do {                                   \
         (MGRDATA)->awe_error.error_code = (AWECMD_ST)->error_code; \
         (MGRDATA)->awe_error.error_desc = (AWECMD_ST)->error_desc; \
     } while (0)

enum awemgr_vartype awctype_2_awemgrtype(awc_ctl_type_t awc_type);
void copy_ctl_info(awc_ctl_t* var_p, struct awemgr_ctl_elem_info *info);

void copyInfoFromAwcModule(struct awemgr_module *mod, const awc_module_t* awc_module);
void awemgr_copy_usrdata(const awc_dict_element* src, awemgr_userdata* dst);

enum awemgr_rc safe_transact(struct awectrl_data *ctrl_p, struct awecmd_st *buf_p, awecore_error_t *err_p, unsigned int *result_buffer_p, unsigned int result_buffer_size_in_words, unsigned int* rx_words_received_p);

enum awemgr_rc get_awe_instance_ids(struct awemgr_data* mgr_p, unsigned int *nr_awe_instances_p, unsigned int** instance_numbers_pp);

float fract32_to_float(int v);
float fract16_to_float(int v);
int32_t float_to_fract32(float f);
int16_t float_to_fract16(float f);

enum awemgr_rc check_int_range(int64_t val, awc_ctl_t *ctl_p, unsigned int index);
enum awemgr_rc check_float_range(double val, awc_ctl_t *ctl_p, unsigned int index);
enum awemgr_rc check_value_range(void *data, unsigned int index, awc_ctl_t *ctl_p);

#if defined(__cplusplus)
}
#endif

#endif // INCLUSION_GUARD_AWEMGR_UTIL_H