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
#include "awosal_mutex.h"
#include "awosal_logging.h"
#include <windows.h>
#include <stdlib.h>

struct windata {
    HANDLE mutex_handle;
};

int lock_mutex(awosal_mutex* mutex, int timeout_ms)
{
    AWOSAL_FAIL_ON_PTR(mutex);
    AWOSAL_FAIL_ON_PTR(mutex->platform_data);
    DWORD result = WaitForSingleObject(((struct windata*)mutex->platform_data)->mutex_handle, timeout_ms < 0 ? INFINITE : timeout_ms);
    if (result == WAIT_OBJECT_0) {
        return AWEOSAL_RC_OK;
    } else if (result == WAIT_TIMEOUT) {
        return AWEOSAL_RC_TIMEOUT;
    }
    return AWEOSAL_RC_FAIL;
}

int unlock_mutex(awosal_mutex* mutex) {
    AWOSAL_FAIL_ON_PTR(mutex);
    AWOSAL_FAIL_ON_PTR(mutex->platform_data);
    BOOL result = ReleaseMutex(((struct windata*)mutex->platform_data)->mutex_handle);
    return result ? AWEOSAL_RC_OK : AWEOSAL_RC_FAIL;
}

int destroy_mutex(awosal_mutex** mutex) {
    AWOSAL_FAIL_ON_PTR(mutex);
    awosal_mutex* pMutex = *mutex;
    AWOSAL_FAIL_ON_PTR(pMutex);
    AWOSAL_FAIL_ON_PTR(pMutex->platform_data);

    CloseHandle(((struct windata*)pMutex->platform_data)->mutex_handle);

    free(pMutex);
    *mutex = NULL;
    return AWEOSAL_RC_OK;
}

awosal_mutex* awosal_create_mutex() {
    // Allocate awosal_mutex Structure
    awosal_mutex* mutex = calloc(1, sizeof(awosal_mutex));
    if (!mutex) {
        AWOSAL_LOGE("Failed to allocate memory for mutex");
        return NULL;
    }
    // Initialize windows mutex
    mutex->platform_data = calloc(1, sizeof(struct windata));
    if (!mutex->platform_data) {
        AWOSAL_LOGE("Failed to allocate memory for mutex platform data");
        free(mutex);
        return NULL;
    }
    ((struct windata*)mutex->platform_data)->mutex_handle = CreateMutex(NULL, FALSE, NULL);
    if (!((struct windata*)mutex->platform_data)->mutex_handle) {
        AWOSAL_LOGE("Failed to initialize mutex");
        free(mutex->platform_data);
        free(mutex);
        return NULL;
    }
    // Setup function pointers
    mutex->lock = lock_mutex;
    mutex->unlock = unlock_mutex;
    mutex->destroy = destroy_mutex;
    return mutex;
}
