#include <gtest/gtest.h>
#include <atomic>
#include "awosal_thread.h"
#include "awosal_mutex.h"
#include "awosal_time.h"

static std::atomic<int> shared_counter{0};
static awosal_mutex* mutex = nullptr;

void* increment_with_lock(void* arg) {
    int* increment = (int*)arg;
    if(increment)
    {
        for (int i = 0; i < *increment; i++) {
            mutex->lock(mutex, -1);
            int old = shared_counter.fetch_add(1, std::memory_order_relaxed);
            mutex->unlock(mutex);
            aweosal_mssleep(1);
        }
    }
    return NULL;
}

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Mutex.Basic~1
  covers: dsn~AWEMGR.AWOSAL.Mutex~1
  description: Ensures that a mutex can be created, locked, unlocked and destroyed
```
*/
TEST(AWOSALMutexTests, LockUnlock) {
    mutex = awosal_create_mutex();
    ASSERT_NE(mutex, nullptr) << "Mutex creation failed.";

    mutex->lock(mutex, -1);
    shared_counter.store(1, std::memory_order_relaxed);
    mutex->unlock(mutex);

    EXPECT_EQ(shared_counter.load(), 1) << "Shared counter was not correctly updated.";

    mutex->destroy(&mutex);
    ASSERT_EQ(mutex, nullptr);
}

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Mutex.Multithread~1
  covers: dsn~AWEMGR.AWOSAL.Mutex~1
  description: Ensures that a mutex can effectively work when 2 threads are trying to lock/unlock a mutex
```
*/
TEST(AWOSALMutexTests, MutexMultiThreading) {
    mutex = awosal_create_mutex();
    shared_counter.store(0, std::memory_order_relaxed);
    ASSERT_NE(mutex, nullptr) << "Mutex creation failed.";

    int increments_per_thread = 100;
    awosal_thread* thread1 = awosal_create_thread();
    awosal_thread* thread2 = awosal_create_thread();

    ASSERT_NE(thread1, nullptr) << "awosal_thread 1 creation failed.";
    ASSERT_NE(thread2, nullptr) << "awosal_thread 2 creation failed.";

    thread1->start(thread1, increment_with_lock, &increments_per_thread);
    thread2->start(thread2, increment_with_lock, &increments_per_thread); 
  
    thread1->join(thread1, NULL);
    thread2->join(thread2, NULL);
    thread1->destroy(&thread1);
    ASSERT_EQ(thread1, nullptr);
    thread2->destroy(&thread2);
    ASSERT_EQ(thread2, nullptr);
    EXPECT_EQ(shared_counter.load(), increments_per_thread * 2) << "Mutex did not ensure mutual exclusion correctly.";

    mutex->destroy(&mutex);
    ASSERT_EQ(mutex, nullptr);
}
