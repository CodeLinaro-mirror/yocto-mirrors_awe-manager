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

#include "awosal_time.h"
#if defined(AWOSAL_WINDOWS)
#include <windows.h>
#else
#include <time.h>
#endif

aweosal_clock_time aweosal_measure_start(void) {
    aweosal_clock_time start;
#if defined(AWOSAL_WINDOWS)
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    start.ticks = (uint64_t)counter.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    start.ticks = (uint64_t)ts.tv_sec * 1000000000ul + (uint64_t)ts.tv_nsec;
#endif
    return start;
}

double aweosal_measure_elapsed(const aweosal_clock_time start) {
#if defined(AWOSAL_WINDOWS)
    LARGE_INTEGER freq, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    return (double)(now.QuadPart - start.ticks) / (double)freq.QuadPart * 1000.0;
#else
    aweosal_clock_time now;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    now.ticks = (uint64_t)ts.tv_sec * 1000000000ul + (uint64_t)ts.tv_nsec;
    return ((double)(now.ticks - start.ticks)) / 1e6f ; // return ms
#endif
}
