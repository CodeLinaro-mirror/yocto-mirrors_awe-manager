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


#ifndef INCLUSION_GUARD_AWEMGR_TARGETINFO_H
#define INCLUSION_GUARD_AWEMGR_TARGETINFO_H

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_AWEMGR_INSTANCES 16

/**
 * A structure holding information about the specific target AWE Core
 * It is similar to an AWE Core internal structure but it is already more
 * application friendly, ie. no packed strings into UINT32s.
 *
 */
typedef struct _awemgr_targetinfo_per_core
{

	float m_sampleRate;   /**< Target sample rate. */
	float m_profileClockSpeed; /**< Target profile clock speed in Hz. */
	unsigned int block_size;  /** Base block size of target */
	unsigned int m_packedData;  /**< Packed field. */


	unsigned int nr_threads;

	/** Target version - high byte is base framework version and must match. */
	unsigned int m_version;					/* 16 */
	char version[15];
	char version_long[30];

	/** Packed field for buffer size, s, input pin count, output pin count. */
	unsigned int m_proxy_buffer_size;			/* 20 */

	char targetName[17];  /** Target name up to 16 characters. */

	float m_coreClockSpeed;  /**< Clock speed of this core. */

	unsigned int m_coreID;  /**< ID of this core. */

	unsigned int m_base_block_size;  /** various information encoded */
	unsigned int m_features;  /**< Feature bits. */

} awemgr_targetinfo_per_core;



typedef struct _awemgr_targetinfo
{
	unsigned int nr_awe_instances;
	awemgr_targetinfo_per_core instance[MAX_AWEMGR_INSTANCES];

} awemgr_targetinfo;

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWEMGR_TARGETINFO_H
