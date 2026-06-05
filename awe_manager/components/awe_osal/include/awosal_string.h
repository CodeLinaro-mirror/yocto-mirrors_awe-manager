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


#ifndef INCLUSION_GUARD_AWOSAL_STRING_H
#define INCLUSION_GUARD_AWOSAL_STRING_H

#include <string.h>

// !TODO: QC internal (b9645373) - glib.h must be included BEFORE extern "C": glib transitively
// pulls in C++ headers (e.g. <type_traits>) that cannot have C linkage. Placing
// the include inside extern "C" causes "template with C linkage" compile errors.
#ifdef AWEMGR_BUILD_USE_GLIB
    #include <glib.h>
    #define strlcpy  g_strlcpy
    #define strlcat  g_strlcat
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef AWEMGR_BUILD_USE_GLIB
    #include <stddef.h>

    /**
     * @brief Size bounded string copy function (like strlcpy).
     *
     * Copies up to size-1 chars from src to dst, null-terminates dst,
     * and returns the length of src.
     *
     * @param dst Destination buffer.
     * @param src Source string.
     * @param size Size of dst buffer.
     * @return Length of src; if >= size, output was truncated. Returns 0 if dst or src is NULL.
     */
    size_t awosal_strlcpy(char *dst, const char *src, size_t size);

    #define strlcpy  awosal_strlcpy
#endif

#ifdef _MSC_VER
#define strdup _strdup
#define strtok_r strtok_s
#endif

#ifdef __cplusplus
}
#endif

#endif // INCLUSION_GUARD_AWOSAL_STRING_H