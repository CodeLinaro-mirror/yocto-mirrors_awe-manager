#include <iostream>
#include <iomanip>
#include <fstream>
#include "awe_ctrl.h"
#include "stressTest.h"

stressTest::stressTest(testConfig &config) : _config(config)
{
    int endpoint = 0;
    static const char *awc_file = TEST_DATA_DIR "/designs/events/target_files/awc_index.txt";

    awemgr_config_create(&cfg_p);	
    awemgr_config_set(cfg_p, "mgr.ctrl.socket.ip", _config.targetIP.c_str());
    awemgr_config_set(cfg_p, "mgr.ctrl.socket.port", _config.tuningPort.c_str());
    awemgr_config_set(cfg_p, "mgr.event.socket.ip", _config.targetIP.c_str());
    awemgr_config_set(cfg_p, "mgr.event.socket.port", _config.eventPort.c_str());

    enum awemgr_rc rc = awemgr_init(&cfg_p, &mgr_p);
    if (!mgr_p)
    {
        std::cout << "Failed to initialize mgr_p" << std::endl;
        return;
    }

    rc = awemgr_load_awc(mgr_p, awc_file, endpoint);
    if (rc != awemgr_RC_OK)
    {
        std::cout << "Failed to load awc" << std::endl;
        return;
    }

    ctx_p = awemgr_get_awc_context(mgr_p, endpoint);
    if (!ctx_p)
    {
        std::cout << "Failed to get awc context" << std::endl;
        return;
    }

    rc = awemgr_load_design(ctx_p, "Main");
    if (rc != awemgr_RC_OK)
    {
        std::cout << "Failed to load design" << std::endl;
        return;
    }
}

stressTest::~stressTest()
{
    awemgr_events_stop(ctx_p);
    awemgr_exit(&mgr_p);
    aweconfig_destroy(&cfg_p);
}

void stressTest::runTests()
{
    auto eventAlert = [](const awemgr_event *ev, void *userdata)
    {
        stressTest *self = static_cast<stressTest *>(userdata);
        float *data = (float *)ev->payload;
        self->results.eventCount++;
        if (!self->_config.quiet)
        {
            if (ev->sizeInBytes == 8) // Event 2
                std::cout << self->results.eventCount << ": " << ev->module.name << ", Payload Size: " << ev->sizeInBytes << ", RMS: " << std::fixed << std::setprecision(6) << data[0] << ", Threshold: " << data[1] << std::endl;
            else // Event 1
                std::cout << self->results.eventCount << ": " << ev->module.name << ", Payload Size: " << ev->sizeInBytes << std::endl;
        }
    };
    auto rc = awemgr_events_start(ctx_p, eventAlert, this);
    if (rc != awemgr_RC_OK)
    {
        std::cout << "Failed to start events" << std::endl;
        return;
    }

    auto processEventsThread = [&]()
    {
        std::cout << "Running thread to process events" << std::endl;
        start_time = std::chrono::high_resolution_clock::now();
        setValue<float>("Scaler1.gain", 15.f); // Event 2 is triggered automatically
        while (running.load())
        {
            if (awemgr_events_process_next(ctx_p, 0) < 0)
            {
                results.eventApiErrors++;
            }
            std::this_thread::yield();
        }
        setValue<float>("Scaler1.gain", 0.f);                      // Event 2 is triggered automatically
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // wait some time to process balance events.
        results.missedEvents = getValue<uint32_t>("Event1.triggerCnt") + getValue<uint32_t>("Event2.triggerCnt") - results.eventCount;
    };

    auto manualTriggerThread = [&]()
    {
        std::cout << "Running thread to Control Writes" << std::endl;
        while (running.load())
        {
            if (setValue<uint32_t>("EventTrigger.value", 1))
                results.writeCount++;
            else
                results.writeApiErrors++;
            std::this_thread::sleep_for(std::chrono::microseconds(200)); // Let the trigger be processed
            if (setValue<uint32_t>("EventTrigger.value", 0))
                results.writeCount++;
            else
                results.writeApiErrors++;
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    };

    auto readValueThread = [&]()
    {
        std::cout << "Running thread to Control Reads" << std::endl;
        while (running.load())
        {
            if (getValue<uint32_t>("Event1.triggerCnt"))
                results.readCount++;
            else
                results.readApiErrors++;
            std::this_thread::sleep_for(std::chrono::microseconds(300));
            if (getValue<uint32_t>("Event2.triggerCnt"))
                results.readCount++;
            else
                results.readApiErrors++;
            std::this_thread::sleep_for(std::chrono::microseconds(300));
        }
    };
    std::thread t1(processEventsThread);
    std::thread t2(manualTriggerThread);
    std::thread t3(readValueThread);

    std::this_thread::sleep_for(std::chrono::seconds(_config.durationSec));
    running.store(false);
    if (t1.joinable())
    {
        t1.join();
    }
    if (t2.joinable())
    {
        t2.join();
    }
    if (t3.joinable())
    {
        t3.join();
    }
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    results.duration = elapsed.count();
}

void stressTest::printTestResults()
{
    std::cout << "\033[33m========================= Test Results =========================\033[0m" << std::endl;
    std::cout << "\033[33m                          AWE_MGR Stress Tests                  \033[0m" << std::endl;
    std::cout << "\033[33m----------------------------------------------------------------\033[0m" << std::endl;

    std::cout << "\033[36m" << std::left << std::setw(30) << "Event Received Count:" << "\033[32m✔ " << results.eventCount << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Event Missed Count:" << "\033[31m✘ " << results.missedEvents << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Event API Errors:" << "\033[31m✘ " << results.eventApiErrors << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Control Write Success Count:" << "\033[32m✔ " << results.writeCount << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Control Write API Error Count:" << "\033[31m✘ " << results.writeApiErrors << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Control Read Count:" << "\033[32m✔ " << results.readCount << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Control Read API Error Count:" << "\033[31m✘ " << results.readApiErrors << "\033[0m" << std::endl;
    std::cout << "\033[36m" << std::left << std::setw(30) << "Test Duration:" << results.duration << " ms" << "\033[0m" << std::endl;

    std::cout << "\033[33m================================================================\033[0m" << std::endl;
}
