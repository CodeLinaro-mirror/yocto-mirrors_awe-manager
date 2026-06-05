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


/*
This is the header file of the awe_COMM component/library.
 */

#ifndef AWE_COMM_BACKEND_H
#define AWE_COMM_BACKEND_H

#include <stddef.h>
#include <stdint.h>

#include "awe_config.h" // for awe_config
#include "awe_cmd.h"  // for awecmd_st and aweevent_header

typedef struct awe_comm_backend {

    int (*init)(struct awe_comm_backend* bkend);
    int (*exit)(struct awe_comm_backend* bkend);
    int (*write)(struct awe_comm_backend* bkend, void* data, int data_sz_words, uint32_t timeoutMs);
    int (*read)(struct awe_comm_backend* bkend, void* target_buffer, int target_buffer_sz_words, int* nr_words_read, uint32_t timeoutMs);

    void* platform_data;
    void* userdata; // for backend to store user data, e.g. (future) callback function pointer, etc.
} awe_comm_backend;


#endif // AWE_COMM_BACKEND_H