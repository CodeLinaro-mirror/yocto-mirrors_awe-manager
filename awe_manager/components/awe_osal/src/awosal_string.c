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

#include "awosal_string.h"

size_t awosal_strlcpy(char *dst, const char *src, size_t size) {
    if (!src || !dst) {
        return 0;
    }

    const char *s = src;
    while (*s) ++s;
    size_t srclen = (size_t)(s - src);

    if (size != 0) {
        size_t copylen = (srclen >= size) ? size - 1 : srclen;      // ensure copylen is always less than size of dst buffer
        if (dst != src) {
            memmove(dst, src, copylen); // memmove is safer compared to memcpy for overlapping pointers, copylen is precomputed to be less than the size
        }
        dst[copylen] = '\0';
    }
    return srclen;
}