#include <iostream>
#include <thread>
#include "awe_manager.h"
#include "awe_comm.h"
#include <iomanip>

#define DEFAULT_TEST_DURATION_SEC (2)
bool running = true;
uint32_t cbCount = 0;
struct awemgr_ctx *ctx_p = NULL;
struct awemgr_data *mgr_p = NULL;
static const char *awc_file = TEST_DATA_DIR "/designs/events/target_files/awc_index.txt";

int initialize()
{
    int endpoint = 0;

    enum awemgr_rc rc = awemgr_init(NULL, &mgr_p);
    if (!mgr_p)
    {
        std::cout << "Failed to initialize mgr_p" << std::endl;
        return -1;
    }

    rc = awemgr_load_awc(mgr_p, awc_file, endpoint);
    if (rc != awemgr_RC_OK)
    {
        std::cout << "Failed to load awc" << std::endl;
        return -1;
    }

    ctx_p = awemgr_get_awc_context(mgr_p, endpoint);
    if (!ctx_p)
    {
        std::cout << "Failed to get awc context" << std::endl;
        return -1;
    }

    rc = awemgr_load_design(ctx_p, "Main");
    if (rc != awemgr_RC_OK)
    {
        std::cout << "Failed to load design" << std::endl;
        return -1;
    }

    auto eventCallback = [](const awemgr_event *ev, void *userdata)
    {
        float *data = (float *)ev->payload;
        std::cout << ev->module.name << ": Payload Size: " << ev->sizeInBytes << ", RMS: " << std::fixed << std::setprecision(6) << data[0] << ", Threshold: " << data[1] << std::endl;
        cbCount++;
    };

    rc = awemgr_events_start(ctx_p, eventCallback, NULL);
    if (rc != awemgr_RC_OK)
    {
        std::cout << "Failed to start events" << std::endl;
        return -1;
    }
    return 0;
}

int cleanup()
{
    return awemgr_exit(&mgr_p);
}

template <typename T>
bool setValue(const char *name, T value)
{
    auto ret = awemgr_control_write(ctx_p, name, 0, (void *)&value, 1) == awemgr_RC_OK;
    return ret;
}

int runTest(unsigned int durationInSeconds)
{
    auto processEventsThread = [&]()
    {
        std::cout << "Running thread to process events" << std::endl;
        while (running)
        {
            awemgr_events_process_next(ctx_p, durationInSeconds);
        }
    };

    std::thread t(processEventsThread);

    setValue<float>("Scaler1.gain", 15.f); // Set the scaler gain to trigger RMS > threshold events

    std::this_thread::sleep_for(std::chrono::seconds(durationInSeconds));
    running = false;
    if (t.joinable())
    {
        t.join();
    }
    std::cout << "Test Ran for " << durationInSeconds << " seconds, Received " << cbCount << " events!!" << std::endl;
    return 0;
}

int main(int argc, char *argv[])
{
    unsigned durationInSeconds = DEFAULT_TEST_DURATION_SEC;
    if (argc > 1)
    {
        std::string arg = argv[1];
        if (arg.rfind("-time:", 0) == 0)
        {
            durationInSeconds = std::atoi(arg.c_str() + std::string("-time:").length());
            std::cout << std::left << std::setw(30) << "-time:" << durationInSeconds << std::endl;
        }
        else
        {
            std::cout << "Usage: " << argv[0] << " -time:N" << std::endl;
            exit(0);
        }
    }
    initialize();
    runTest(durationInSeconds);
    cleanup();
    return 0;
}
