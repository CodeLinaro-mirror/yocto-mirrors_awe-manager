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
#include "awosal_thread.h"
#include "awosal_mutex.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void* thread_function(void* arg) {
    int* count = (int*)arg;
    for (int i = 0; i < 5; i++) {
        printf("awosal_thread function running... Count: %d\n", *count + i);
    }
    return (void*)count;
}

int main() {
    int start_count = 100;

    awosal_thread* thread = awosal_create_thread();
    if (!thread) {
        fprintf(stderr, "Failed to create thread.\n");
        return EXIT_FAILURE;
    }
    thread->start(thread, thread_function, &start_count);
    void* threadreturn;
    thread->join(thread, (void**)&threadreturn);
    printf("Thread Returned %d", (int)(intptr_t)threadreturn);
    thread->destroy(&thread);

    awosal_mutex* mutex = awosal_create_mutex();
    if (!mutex) {
        fprintf(stderr, "Failed to create mutex.\n");
        return EXIT_FAILURE;
    }

    mutex->lock(mutex, -1);
    printf("Main thread has locked the mutex.\n");
    
    printf("Performing a critical operation while holding the mutex lock...\n");

    mutex->unlock(mutex);
    printf("Main thread has unlocked the mutex.\n");

    mutex->destroy(&mutex);

    printf("Example program finished successfully.\n");
    return EXIT_SUCCESS;
}
