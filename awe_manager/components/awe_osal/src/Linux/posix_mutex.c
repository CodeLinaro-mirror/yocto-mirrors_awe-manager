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
#include "awosal_mutex.h"
#include "awosal_logging.h"
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

int lock_mutex(awosal_mutex* mutex, int timeout_ms) {
    AWOSAL_FAIL_ON_PTR(mutex);
    AWOSAL_FAIL_ON_PTR(mutex->platform_data);
    if(timeout_ms < 0)
    {
        return pthread_mutex_lock(mutex->platform_data) < 0 ? AWEOSAL_RC_FAIL : AWEOSAL_RC_OK;
    }
    else
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;

        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        int result = pthread_mutex_timedlock(mutex->platform_data, &ts);
        if (result == 0) {
            return AWEOSAL_RC_OK;
        } else if (result == ETIMEDOUT) {
            return AWEOSAL_RC_TIMEOUT;
        }
    }
    return AWEOSAL_RC_FAIL;
}

int unlock_mutex(awosal_mutex* mutex) {
    AWOSAL_FAIL_ON_PTR(mutex);
    AWOSAL_FAIL_ON_PTR(mutex->platform_data); 
    return pthread_mutex_unlock((pthread_mutex_t*)mutex->platform_data)== 0 ? AWEOSAL_RC_OK : AWEOSAL_RC_FAIL;
}

int destroy_mutex(awosal_mutex** mutex) {
    AWOSAL_FAIL_ON_PTR(mutex);
    awosal_mutex* pMutex = *mutex;
    AWOSAL_FAIL_ON_PTR(pMutex);
    AWOSAL_FAIL_ON_PTR(pMutex->platform_data);

    pthread_mutex_destroy((pthread_mutex_t*)pMutex->platform_data);

    free(pMutex->platform_data);
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
    // Allocate pthread_mutex_t Structure
    mutex->platform_data = calloc(1, sizeof(pthread_mutex_t));
    if (!mutex->platform_data) {
        AWOSAL_LOGE("Failed to allocate platform-specific mutex data");
        free(mutex);
        return NULL;
    }
    // Initialize pthread_mutex_t
    if (pthread_mutex_init((pthread_mutex_t*)mutex->platform_data, NULL) != 0) {
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
