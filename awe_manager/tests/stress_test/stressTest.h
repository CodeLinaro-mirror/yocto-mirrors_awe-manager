#pragma once
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <chrono>
#include "awe_manager.h"
#include "testConfig.h"

typedef struct testResults
{
    unsigned int eventCount = 0;
    unsigned int eventApiErrors = 0;
    unsigned int missedEvents = 0;
    unsigned int writeCount = 0;
    unsigned int writeApiErrors = 0;
    unsigned int readCount = 0;
    unsigned int readApiErrors = 0;
    double duration;
} testResults_t;

class stressTest
{
public:
    stressTest(testConfig &config);
    ~stressTest();
    void runTests();
    void printTestResults();

private:
    template <typename T>
    bool setValue(const char *name, T value)
    {
        std::lock_guard<std::mutex> lockguard(mtx);
        auto ret = awemgr_control_write(ctx_p, name, 0, (void *)&value, 1) == awemgr_RC_OK;
        return ret;
    }

    template <typename T>
    T getValue(const char *name)
    {
        unsigned int nr_words_in_buf = 0;
        awemgr_vartype type;
        T value;
        std::lock_guard<std::mutex> lockguard(mtx);
        auto ret = awemgr_control_read(ctx_p, name, (void *)&value, 1, &nr_words_in_buf, &type) == awemgr_RC_OK;
        return value;
    }

    std::atomic<bool> running{true};
    testResults_t results;
    struct awemgr_data *mgr_p = nullptr;
    struct awemgr_ctx *ctx_p = nullptr;
    std::chrono::high_resolution_clock::time_point start_time;
    testConfig &_config;
    awe_config* cfg_p;
    std::mutex mtx;
};