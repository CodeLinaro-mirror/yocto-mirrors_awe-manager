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


#ifndef INCLUSION_GUARD_AWECMD_RESPONSES_H
#define INCLUSION_GUARD_AWECMD_RESPONSES_H

#include "awe_cmd_types.h"

// copy of AWE-CORE's /*private*/ TargetInfo and ExtendedInfo structures

typedef struct _TargetInfo
{
	/** Target sample rate. */
	float m_sampleRate;					/* 0 */

	/** Target profile clock speed in Hz. */
	float m_profileClockSpeed;			/* 4 */

	/** Base block size of target (usually 32). */
	UINT32 m_base_block_size;			/* 8 */

	/** Packed field. */
	UINT32 m_packedData;				/* 12 */

	/** Target version - high byte is base framework version and must match. */
	UINT32 m_version;					/* 16 */

	/** Packed field for buffer size, s, input pin count, output pin count. */
	UINT32 m_proxy_buffer_size;			/* 20 */

	/** Target name up to 16 characters. */
	UINT32 m_packedName[2];				/* 24 */

	/** Clock speed of this core. */
	float m_coreClockSpeed;				/* 32 */

	/** ID of this core. */
	UINT32 m_coreID;					/* 36 */

	/** Feature bits. */
	UINT32 m_features;					/* 40 */

}
TargetInfo;

/* private */ typedef struct _ExtendedInfo
{
    /** The user version field. */
    UINT32 userVersion;

    /** Bit packed informational word 1 */
    /**
        bit 0: AWECoreOS product identifier (set in AWECoreOS.c only)
        bit 1: Is the target implementing the shared heap multi-instance model (1 - Yes, otherwise static)
        bit 2-10: Hot fix version
        bit 11-18: Alignment size
        bit 19: Modules reset feature status
        bit 20: Fract16 module/wire support status
        bit 21-31: Unused
    */
    UINT32 infoWord1;

    UINT32 buildNumber;

    /** Currently undefined fields that will report zeros. */
    UINT32 notSpecified[10];

} ExtendedInfo;    // Total length 13 words

// copy of AWE-CORE's defines

#define GET_TARGET_PACKET_BUFFER_LEN(x) (((x)->m_proxy_buffer_size & 0x1fffU) + 1U)
#define SMP_CORE_BIT    0x80000U
#define IS_SMP_CORE(x)                    (((x)->m_proxy_buffer_size & SMP_CORE_BIT) != 0U)
#define GET_TARGET_CORES(x)                ((((x)->m_proxy_buffer_size >> 13) & 0x3fU) + 1U)

#define GET_TARGET_NINPUT_PINS(x)        ((((x)->m_proxy_buffer_size >> 20) & 0x3fU) + 1U)
#define GET_TARGET_NOUTPUT_PINS(x)        ((((x)->m_proxy_buffer_size >> 26) & 0x3fU) + 1U)
#define TARGET_INFO_NUM_INPUTS(targetinfo)    (((targetinfo)->m_packedData >> 8) & 0xffU)
#define TARGET_INFO_NUM_OUTPUTS(targetinfo)   (((targetinfo)->m_packedData >> 16) & 0xffU)

#define MAX_FUNDAMENTAL_BLOCKSIZE            (0xFFFU)
#define MAX_TARGET_THREADS_MASK              (0x3F)
#define GET_TARGET_THREADS(x)            (((x)->m_base_block_size >> 12) & MAX_TARGET_THREADS_MASK)
#define GET_TARGET_BASE_BLOCK_SIZE(x)    ((x)->m_base_block_size & MAX_FUNDAMENTAL_BLOCKSIZE)

#define USING_ALIGN4        (0x200000U)
#define GET_USING_ALIGN4(x)                (((x)->m_base_block_size & USING_ALIGN4) != 0U)

#define GET_EXT_INFO_ALIGN(extendedInfo)        (((extendedInfo).infoWord1 >> ALIGNMENT_WORDS_EXTENDED_INFO_START) & MAX_ALIGNMENT_WORDS)


char* awecmd_target_info_string(TargetInfo *ti_p);

const char* awecmd_target_info_get_name(TargetInfo* ti_p);
const char* awecmd_target_info_get_version(TargetInfo* ti_p);
const char* awecmd_target_info_get_version_long(TargetInfo* ti_p);
const char* awecmd_target_info_get_proc_type(TargetInfo* ti_p);

static inline unsigned int awecmd_extended_info_get_hotfix_version(ExtendedInfo* ext_p) { return (ext_p->infoWord1 >> 2) & 0x1ffU; }
static inline unsigned int awecmd_extended_info_get_alignment_size(TargetInfo* ti_p, ExtendedInfo* ext_p) {
	unsigned int align_words = (ext_p->infoWord1 >> 11) & 0x7fU;
	if (! align_words) {
		// older targets
		if GET_USING_ALIGN4(ti_p) {
			align_words = 4U;
		}
		else {
			align_words = 1U;
		}
	}
	return align_words;
}
static inline unsigned int awecmd_extended_info_has_module_reset(ExtendedInfo* ext_p) { return ext_p->infoWord1 & (1U << 19); }
static inline unsigned int awecmd_extended_info_has_fract16_support(ExtendedInfo* ext_p) { return ext_p->infoWord1 & (1U << 20); }

#endif // INCLUSION_GUARD_AWECMD_RESPONSES_H
