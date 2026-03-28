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

#include "awe_cmd_responses.h"

#include "awe_cmd_logging.h"
#include "awosal_string.h"

#include <stdio.h>

#define CHECK_NULL(c)    if (!c) { \
    AWE_CMD_LOGE("Invalid argument: handle == NULL!: %s", #c); \
    return 0; \
    }

char* awecmd_target_info_string(TargetInfo *ti_p)
{
    static char full_info[128];

    CHECK_NULL(ti_p);

    UINT32 str[3];
    str[0] = ti_p->m_packedName[0];
    str[1] = ti_p->m_packedName[1];
    str[2] = 0;

    char targetName[16];
    snprintf(targetName, sizeof(targetName), "%.8s(%d)", (char*) str, ti_p->m_coreID);

    int nThreads = ((ti_p->m_base_block_size >> 12) & 0xf);
    int bs = (ti_p->m_base_block_size & 0xfff);

    union
    {
        unsigned long ver;
        unsigned char verb[4];
    } u = {0};
    u.ver = ti_p->m_version;

    char version[15];
    char versionFull[30];
    strlcpy(version, "n/a", sizeof("n/a"));
    if (u.verb[2] >= 'A' && u.verb[2] <= 'Z')
    {
        snprintf(version, sizeof(version), "%c", u.verb[2]);
    }
    snprintf(versionFull, sizeof(versionFull), "AWECore: %d.%s.%d.%d", u.verb[3], version, u.verb[1], u.verb[0]);

    snprintf(full_info, sizeof(full_info), "Target Info: %s at %s : sr=%.2fHz; bs=%d; threads=%d", versionFull, targetName, ti_p->m_sampleRate, bs, nThreads);
    return full_info;
}

unsigned int awecmd_target_info_get_threads(TargetInfo* ti_p)
{
    CHECK_NULL(ti_p);
    return ((ti_p->m_base_block_size >> 12) & 0xf);
}

unsigned int awecmd_target_info_get_blocksize(TargetInfo* ti_p)
{
    CHECK_NULL(ti_p);
    return (ti_p->m_base_block_size & 0xfff);
}

const char* awecmd_target_info_get_name(TargetInfo* ti_p)
{
    static char targetName[16];

    CHECK_NULL(ti_p);

    UINT32 str[3];
    str[0] = ti_p->m_packedName[0];
    str[1] = ti_p->m_packedName[1];
    str[2] = 0;

    snprintf(targetName, sizeof(targetName), "%s", (char*) str);
    return targetName;
}
const char* awecmd_target_info_get_version(TargetInfo* ti_p)
{
    static char version[15];
    strlcpy(version, "n/a", sizeof("n/a"));

    CHECK_NULL(ti_p);

    union
    {
        unsigned long ver;
        unsigned char verb[4];
    } u = {0};
    u.ver = ti_p->m_version;

    if (u.verb[2] >= 'A' && u.verb[2] <= 'Z')
    {
        snprintf(version, sizeof(version), "%c", u.verb[2]);
    }
    return version;
}
const char* awecmd_target_info_get_version_long(TargetInfo* ti_p)
{
    static char versionFull[30];

    CHECK_NULL(ti_p);

    union
    {
        unsigned long ver;
        unsigned char verb[4];
    } u = {0};
    u.ver = ti_p->m_version;

    snprintf(versionFull, sizeof(versionFull), "AWECore: %d.%s.%d.%d",
             u.verb[3],
             awecmd_target_info_get_version(ti_p), u.verb[1], u.verb[0]);
    return versionFull;
}

