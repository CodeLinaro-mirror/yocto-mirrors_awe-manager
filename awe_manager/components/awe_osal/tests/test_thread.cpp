#include <gtest/gtest.h>
#include <atomic>
#include "awosal_thread.h"
#include "awosal_time.h"

static std::atomic<int> counter{0};

void* increment_counter(void* arg) {
    int* increment = (int*)arg;
    std::cout << "increment for current thread is " << *increment << std::endl;
    for (int i = 0; i < *increment; ++i) {
        int old = counter.fetch_add(1, std::memory_order_relaxed);
    }
    return NULL;
}

void* threadReturnTest(void* arg) {
    aweosal_mssleep(1);
    return (void*)6;
}

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Threads.Basic~1
  covers: dsn~AWEMGR.AWOSAL.Threads~1
  description: Ensures that a thread can be created, started, joined and destroyed
```
*/
TEST(AWOSALThreadTests, ThreadCreationAndJoin) {
    counter.store(0, std::memory_order_relaxed);
    int increment = 5;
    awosal_thread* thread = awosal_create_thread();

    ASSERT_NE(thread, nullptr) << "awosal_thread creation failed.";
    thread->start(thread, increment_counter, &increment);

    void* threadreturn = &increment; // just to asign a valid address
    ASSERT_EQ(thread->join(thread, &threadreturn), AWEOSAL_RC_OK);
    ASSERT_TRUE(threadreturn == NULL);

    ASSERT_EQ(thread->destroy(NULL), AWEOSAL_RC_FAIL_PARAM);
    thread->destroy(&thread);
    ASSERT_EQ(thread, nullptr);
    EXPECT_EQ(counter.load(), increment) << "Counter did not increment as expected.";
}

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Threads.Multithread~1
  covers: dsn~AWEMGR.AWOSAL.Threads~1
  description: Ensures that component can manage multiple threads independently.
```
*/
TEST(AWOSALThreadTests, MultipleThreads) {
    counter.store(0, std::memory_order_relaxed);
    int increment1 = 5;
    int increment2 = 10;

    awosal_thread* thread1 = awosal_create_thread();
    awosal_thread* thread2 = awosal_create_thread();

    ASSERT_NE(thread1, nullptr) << "awosal_thread 1 creation failed.";
    ASSERT_NE(thread2, nullptr) << "awosal_thread 2 creation failed.";

    thread1->start(thread1, increment_counter, &increment1);
    thread2->start(thread2, increment_counter, &increment2);    
    thread1->join(thread1, NULL);
    thread2->join(thread2, NULL);

    thread1->destroy(&thread1);
    thread2->destroy(&thread2);

    EXPECT_EQ(counter.load(), increment1 + increment2) << "Counter did not increment correctly across threads.";
}
