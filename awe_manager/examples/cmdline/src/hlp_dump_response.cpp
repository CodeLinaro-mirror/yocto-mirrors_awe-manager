/* MIT License
**
** Copyright (c) 2025 DSP Concepts, Inc.
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

#include "hlp_dump_response.h"

#include <stdlib.h>
#include <stdio.h>
#include <awosal_string.h>

#define NR_ITEMS_PER_ROW 8

void dump_response(idbg_t* p, const char *prefix, void *data, unsigned int nr_bytes, enum awemgr_vartype typ, bool asTxCmd)
{
    char packet_log[1024];  // todo: fix hardcoded, check for errors below
    char *c_p = packet_log;
    char *end_p = packet_log + sizeof(packet_log);
    unsigned int nr_words_read = nr_bytes / sizeof(unsigned int);
    unsigned int *data_in_words = (unsigned int*) data;
    unsigned int *data_end_in_words = data_in_words + nr_words_read;

    while (data_in_words < data_end_in_words)
    {
        c_p += snprintf(c_p, (end_p - c_p), "%s", prefix);

        for (size_t i = 0; i < NR_ITEMS_PER_ROW && (data_in_words < data_end_in_words); i++)
        {
            // todo: even though the loop is safe in terms of memory corruption,
            //       it could happen that the packet_log memory is not sufficient,
            //       the routine should then not simply overwrite the last data only
            //       but display some "..." or other indication showing that not
            //       all data items were printed
            if (typ == AWEMGR_VARTYPE_FLOAT)
            {
                const char *fmt = (i == 0) ? "%.4f" : ", %.4f";
                float v = *((float*)data_in_words);
                c_p += snprintf(c_p, (end_p - c_p), fmt, v);
            }
            else if (typ == AWEMGR_VARTYPE_FRACT)
            {
                const char *fmt = (i == 0) ? "%.4f" : ", %.4f";
                int v = *((int*)data_in_words);
                float normalized_v = v / (float)(1 << 31);
                c_p += snprintf(c_p, (end_p - c_p), fmt, normalized_v);
            }
            else
            {
                const char *fmt = (i == 0) ? "0x%08x" : ", 0x%08x";
                c_p += snprintf(c_p, (end_p - c_p), fmt, *data_in_words);
            }
            data_in_words++;
        }
        if (asTxCmd)
        {
            c_p += snprintf(c_p, (end_p - c_p), "\"");
        }
        idbg_print(p, "%s,\n", packet_log);
        c_p = packet_log;
    }
}
