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
#include "awosal_thread.h"
#include "awosal_logging.h"
#include <windows.h>
#include <stdlib.h>
#include <stdint.h>

int start_thread(awosal_thread* thread, void* (*function)(void*), void* arg) {
    AWOSAL_FAIL_ON_PTR(thread);
    AWOSAL_FAIL_ON_PTR(function);
    thread->platform_data = (void*)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, arg, 0, NULL);
    if (thread->platform_data == NULL) {
        AWOSAL_LOGE("Failed to create thread");
        return AWEOSAL_RC_FAIL_RESOURCES;
    }
    return AWEOSAL_RC_OK;
}

int join_thread(awosal_thread* thread, void** threadreturn) {
    AWOSAL_FAIL_ON_PTR(thread);
    if (!thread->platform_data) {
        AWOSAL_LOGE("Invalid argument: thread->platform_data == NULL!");
        return AWEOSAL_RC_FAIL;
    }
    DWORD result = WaitForSingleObject((HANDLE)thread->platform_data, INFINITE);
    if (result != WAIT_OBJECT_0) {
        AWOSAL_LOGE("Failed to join thread");
        return AWEOSAL_RC_FAIL;
    }
    if (threadreturn != NULL)
    {
        DWORD exitCode;
        if (GetExitCodeThread((HANDLE)thread->platform_data, &exitCode) != 0) {
            *threadreturn = (void*)(uintptr_t)exitCode;
        } else {
            AWOSAL_LOGE("Failed to get thread exit code.");
            CloseHandle((HANDLE)thread->platform_data);
            thread->platform_data = NULL;
            return AWEOSAL_RC_FAIL;
        }
    }
    CloseHandle((HANDLE)thread->platform_data);
    thread->platform_data = NULL;
    return AWEOSAL_RC_OK;
}

int destroy_thread(awosal_thread** thread) {
    AWOSAL_FAIL_ON_PTR(thread);
    awosal_thread* pThread = *thread;
    AWOSAL_FAIL_ON_PTR(pThread);

    if (pThread->platform_data) {
        CloseHandle((HANDLE)pThread->platform_data);
    }

    free(pThread);
    *thread = NULL;
    return AWEOSAL_RC_OK;
}


awosal_thread* awosal_create_thread() {
    awosal_thread* thread = calloc(1, sizeof(awosal_thread));
    if (thread == NULL) {
        AWOSAL_LOGE("Failed to allocate memory for thread");
        return NULL;
    }

    thread->start = start_thread;
    thread->join = join_thread;
    thread->destroy = destroy_thread;
    return thread;
}
