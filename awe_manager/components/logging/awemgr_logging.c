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

#include <stddef.h>
#include "awosal_string.h"
#include "awemgr_logging.h"

static unsigned s_awemgr_trace_cfg[AWEMGR_LOG_MAX_COMP];

void set_loglevel(unsigned component, unsigned lvl)
{
    if(component < AWEMGR_LOG_MAX_COMP && lvl < AWEMGR_LOG_LEVEL_MAX)
    {
        s_awemgr_trace_cfg[component] = lvl;
    }
}

unsigned get_loglevel(unsigned component)
{
    return s_awemgr_trace_cfg[component];
}

void awemgr_log_buffer(void *data, unsigned int nr_bytes, int var_typ, print_fct_cb cb, void *ctx)
{
    char packet_log[1024];
    char *c_p = packet_log;
    char *end_p = packet_log + sizeof(packet_log);
    unsigned int nr_words_read = nr_bytes / sizeof(unsigned int);
    unsigned int *data_in_words = (unsigned int*) data;
    unsigned int *data_end_in_words = data_in_words + nr_words_read;

    while (data_in_words < data_end_in_words)
    {
        int index = (int)(data_in_words - (unsigned int*) data);

        for (size_t i = 0; i < NR_ITEMS_PER_ROW && (data_in_words < data_end_in_words); i++)
        {
            int n = 0;
            size_t space_left = (size_t)(end_p - c_p);

            if (space_left == 0) {
                break;
            }

            if (var_typ == AWEMGR_LOG_VARTYPE_FLOAT)
            {
                float v = *((float*)data_in_words);
                n = snprintf(c_p, space_left, (i == 0) ? "%.4f" : ", %.4f", v);
            }
            else if (var_typ == AWEMGR_LOG_VARTYPE_FRACT)
            {
                int v = *((int*)data_in_words);
                float normalized_v = v / (float)(1 << 31);
                n = snprintf(c_p, space_left, (i == 0) ? "%.4f" : ", %.4f", normalized_v);
            }
            else if (var_typ == AWEMGR_LOG_VARTYPE_INT)
            {
                n = snprintf(c_p, space_left, (i == 0) ? "0x%08x" : ", 0x%08x", *data_in_words);
            }
            else
            {
                n = snprintf(c_p, space_left, (i == 0) ? "NaN" : ", NaN");
            }

            if (n < 0) {
                break;
            }
            if ((size_t)n >= space_left) {
                c_p = end_p - 1;
                break;
            }
            c_p += n;
            data_in_words++;
        }
        packet_log[sizeof(packet_log) - 1] = '\0';

        if (cb != NULL) {
            cb(packet_log, index, ctx);
        } else {
            printf("%s\n", packet_log);
        }
        c_p = packet_log;
    }
}
