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


#ifndef INCLUSION_GUARD_AWEMGR_CLASSINFO_H
#define INCLUSION_GUARD_AWEMGR_CLASSINFO_H

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * A structure holding information about the specific target AWE Core
 * It is similar to an AWE Core internal structure but it is already more
 * application friendly, ie. no packed strings into UINT32s.
 *
 */
typedef struct _awemgr_classinfo
{
	unsigned int classId;
	unsigned int nrParameters;

} awemgr_classinfo;

// note: in order to avoid dyn mem allocation here we define a reasonable sized max buffer
#define MAX_NR_CLASSINFOS 1000

typedef struct _awemgr_classlistinfo
{
	unsigned int            nr_classes;
	awemgr_classinfo        classes[MAX_NR_CLASSINFOS];

} awemgr_modulelist_info;


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWEMGR_CLASSINFO_H
