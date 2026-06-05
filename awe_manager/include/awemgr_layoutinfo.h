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


#ifndef INCLUSION_GUARD_AWEMGR_LAYOUTINFO_H
#define INCLUSION_GUARD_AWEMGR_LAYOUTINFO_H

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_AWEMGR_LAYOUTS 16

/**
 * ...
 *
 */
typedef struct _awemgr_layoutinfo_item
{
	unsigned int timePerProcess;  /**< Averaged time in cycles between calls to the processing function.  Times 256. */
	unsigned int timePerProcessExpected;  /**< Expected averaged time in cycles between calls to the processing function.  Times 256. */
	unsigned int averageCycles;  /**< Average cycles per process. Times 256. */
	unsigned int instCycles;  /**< Instantaneous cycles for the entire layout that executed.  Times 256. */
	unsigned int peakCycles; /**< Peak cycles per process. Times 256. */
	unsigned int overflowCount;  /**< number of times that this layout has not completed in real-time and caused a systemwide reset event. */

} awemgr_layoutinfo_item;


/**
 * ...
 */
typedef struct _awemgr_layoutinfo
{
	unsigned int nr_layouts;

	unsigned int averageCyclesAllCombined;  /**< ... */
	unsigned int overflowCountAllLayouts;  /**< ... */
	unsigned int profilingValuesPerLayout;  /**< ... */

	awemgr_layoutinfo_item layouts[MAX_AWEMGR_LAYOUTS];

} awemgr_layoutinfo;


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWEMGR_LAYOUTINFO_H
