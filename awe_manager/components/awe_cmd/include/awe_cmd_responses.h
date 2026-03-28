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


#ifndef INCLUSION_GUARD_AWECMD_RESPONSES_H
#define INCLUSION_GUARD_AWECMD_RESPONSES_H

#include "awe_cmd_types.h"

// copy of AWE Core's TargetInfo structure
// todo: check how this could be included smarter; or kept in sync

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

char* awecmd_target_info_string(TargetInfo *ti_p);

unsigned int awecmd_target_info_get_threads(TargetInfo* ti_p);
unsigned int awecmd_target_info_get_blocksize(TargetInfo* ti_p);
const char* awecmd_target_info_get_name(TargetInfo* ti_p);
const char* awecmd_target_info_get_version(TargetInfo* ti_p);
const char* awecmd_target_info_get_version_long(TargetInfo* ti_p);


#endif // INCLUSION_GUARD_AWECMD_RESPONSES_H
