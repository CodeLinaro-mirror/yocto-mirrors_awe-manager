#include <gtest/gtest.h>
#include <atomic>
#include "awosal_thread.h"
#include "awosal_time.h"

static std::atomic<int> counter{0};

void* increment_counter(void* arg) {
    int* increment = (int*)arg;
    std::cout << "increment for current thread is " << *increment << std::endl;
    for (int i = 0; i < *increment; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed); // int old = ... in case old value is needed for debugging
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

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Threads.JoinInvalidThread~1
  covers: dsn~AWEMGR.AWOSAL.Threads~1
  description: Tests that joining an invalid thread returns an error.
```
*/
TEST(AWOSALThreadTests, JoinInvalidThread) {
    awosal_thread* thread = awosal_create_thread();
    EXPECT_EQ(thread->join(thread, NULL), AWEOSAL_RC_FAIL);
}

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Threads.JoinAlreadyJoined~1
  covers: dsn~AWEMGR.AWOSAL.Threads~1
  description: Tests that joining an already joined thread returns an error.
```
*/
TEST(AWOSALThreadTests, JoinAlreadyJoined) {
    awosal_thread* thread = awosal_create_thread();
    thread->start(thread, threadReturnTest, NULL);
    EXPECT_EQ(thread->join(thread, NULL), AWEOSAL_RC_OK);
    EXPECT_EQ(thread->join(thread, NULL), AWEOSAL_RC_FAIL); // cannot join again
    thread->destroy(&thread);
}


#ifndef WIN32

#include <sys/resource.h>

class ThreadFailureTest : public ::testing::Test {
protected:
    std::vector<pthread_t> resource_hogs;

    void SetUp() override {
        // Reduce limit for this test
        struct rlimit limit;
        getrlimit(RLIMIT_NPROC, &limit);
        original_limit = limit;

        limit.rlim_cur = 20; // Low but reasonable
        setrlimit(RLIMIT_NPROC, &limit);

        // Consume most of the limit
        pthread_t t;
        while (resource_hogs.size() < 18) {
            if (pthread_create(&t, NULL,
                              [](void*)->void*{ pause(); return nullptr; },
                              nullptr) == 0) {
                resource_hogs.push_back(t);
            } else {
                break;
            }
        }
    }

    void TearDown() override {
        // Cleanup
        for (auto& t : resource_hogs) {
            pthread_cancel(t);
            pthread_join(t, nullptr);
        }

        // Restore limit
        setrlimit(RLIMIT_NPROC, &original_limit);
    }

private:
    struct rlimit original_limit;
};

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Threads.CreateThreadFails~1
  covers: dsn~AWEMGR.AWOSAL.Threads~1
  description: Tests that creating and starting a thread fails gracefully.
```
*/
TEST_F(ThreadFailureTest, CreateThreadFails) {
    // The test is skipped due to an unreliable test environment on Jenkins where
    // the thread limit is not properly enforced, leading to inconsistent test results.
    //
    // the functioning of this test is verified by manually setting a low thread limit and running the test locally.

    GTEST_SKIP();

    awosal_thread* thread = awosal_create_thread();

    int result = thread->start(thread, threadReturnTest, NULL);

    EXPECT_EQ(AWEOSAL_RC_FAIL_RESOURCES, result);
}
#endif // WIN32
