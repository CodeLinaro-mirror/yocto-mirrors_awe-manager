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

    int nThreads = GET_TARGET_THREADS(ti_p);
    int bs = GET_TARGET_BASE_BLOCK_SIZE(ti_p);

    union
    {
        unsigned long ver;
        unsigned char verb[4];
    } u = {0};
    u.ver = ti_p->m_version;

    char version[15];
    char versionFull[30+15];  // extra space
    strlcpy(version, "n/a", sizeof("n/a"));
    if (u.verb[2] >= 'A' && u.verb[2] <= 'Z')
    {
        snprintf(version, sizeof(version), "%c", u.verb[2]);
    }
    snprintf(versionFull, sizeof(versionFull), "AWECore: %d.%s.%d.%d", u.verb[3], version, u.verb[1], u.verb[0]);

    snprintf(full_info, sizeof(full_info), "Target Info: %s at %s : sr=%.2fHz; bs=%d; threads=%d", versionFull, targetName, ti_p->m_sampleRate, bs, nThreads);
    return full_info;
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
    static char versionFull[30+15];  // extra space

    CHECK_NULL(ti_p);

    union
    {
        unsigned long ver;
        unsigned char verb[4];
    } u = {0};
    u.ver = ti_p->m_version;

    snprintf(versionFull, sizeof(versionFull), "%d.%s.%d.%d",
             u.verb[3],
             awecmd_target_info_get_version(ti_p), u.verb[1], u.verb[0]);
    return versionFull;
}

typedef struct {
    unsigned int  id;
    const char   *name;
} proc_type_name_entry_t;

static const proc_type_name_entry_t proc_type_name_lookup[] =
{
    {  1U, "Native"      },
    {  2U, "SHARC"       },  /* SHARC 214xx */
    {  3U, "Blackfin"    },
    {  4U, "CortexM4"    },
    {  5U, "OMAP"        },
    {  6U, "DSK6713"     },
    {  7U, "HiFi2DSP"    },  /* Hifi 24 bit (also HiFiMiniDSP) */
    {  8U, "CortexM3"    },
    {  9U, "iMX25"       },
    { 10U, "CortexM7"    },
    { 11U, "C674x"       },
    { 12U, "CortexA5"    },
    { 13U, "CortexA7"    },
    { 14U, "CortexA8"    },
    { 15U, "CortexA9"    },
    { 16U, "CortexA12"   },
    { 17U, "CortexA15"   },
    { 18U, "CortexA53"   },
    { 19U, "CortexA57"   },
    { 20U, "ARM9"        },
    { 21U, "ARM11"       },
    { 22U, "Hexagon"     },  /* Qualcomm Hexagon */
    { 23U, "HiFi3DSP"    },  /* Generic HiFi 3 */
    { 24U, "S1000"       },  /* Intel S1000 / Sue Creek */
    { 25U, "HemiLite"    },  /* Knowles processor in IA-610 */
    { 26U, "CortexA72"   },
    { 27U, "CortexA35"   },
    { 28U, "C66xx"       },
    { 29U, "SHARC215xx"  },  /* SHARC 215xx family */
    { 30U, "HiFi4DSP"    },  /* Generic HiFi 4 */
    { 31U, "DeltaMax"    },  /* Knowles deltaMax processor */
    { 32U, "HemiDelta"   },  /* Knowles hemiDelta processor */
    { 33U, "CEVA"        },  /* CEVA processor */
    { 34U, "FordHiFi4"   },  /* Ford iMX8 HiFi4 processor */
    { 35U, "CEVAX2"      },  /* CEVA X2 processor */
    { 36U, "Unassigned"  },  /* Placeholder type */
    { 37U, "CortexM33"   },
    { 38U, "SC589"       },  /* ADI SC589 */
    { 39U, "HiFi5DSP"   },  /* Generic HiFi 5 */
    { 40U, "CortexM55"   },
    { 41U, "SHARC-FX"    },  /* ADI SHARC-FX processor */
    { 42U, "C7x"         },  /* TI C7X processor */
    { 43U, "Kalimba"     },  /* Kalimba processor */
    { 44U, "CortexA"     },  /* Generic CortexA processor */
    { 45U, "HiFi5sDSP"  },  /* Cadence HiFi5s DSP */
    { 46U, "HIFIiQDSP"  },  /* Cadence HiFi iQ DSP */
};

const char* awecmd_target_info_get_proc_type(TargetInfo* ti_p)
{
    CHECK_NULL(ti_p);
    unsigned int procID = ((ti_p->m_packedData >> 24) & 0x7fU);
    for (size_t i = 0; i < sizeof(proc_type_name_lookup) / sizeof(proc_type_name_entry_t); i++)
    {
        if (proc_type_name_lookup[i].id == procID)
        {
            return proc_type_name_lookup[i].name;
        }
    }
    return "Unknown";
}

