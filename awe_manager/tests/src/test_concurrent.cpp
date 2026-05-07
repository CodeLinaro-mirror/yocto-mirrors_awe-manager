#include "test_fixtures.h"
#include <atomic>
#include <vector>
#include <string>
#include <regex>
#include <awosal_time.h>

/** reusing Subcanvas test design */
static const char *awc_file = TEST_DATA_DIR "/designs/subcanvas/target_files_tl_1/awc_index.txt";

// Helper to turn the big blob of text into a vector of event types
std::vector<std::string> getEventSequence(const std::string& output) {
    std::vector<std::string> sequence;
    std::stringstream ss(output);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.find("LOAD_SC:One-Start") != std::string::npos) sequence.push_back("START");
        else if (line.find("LOAD_SC:One-End") != std::string::npos) sequence.push_back("END");
        else if (line.find("SETVAL") != std::string::npos) sequence.push_back("SET");
    }
    return sequence;
};

/**
```yaml
- id: itest~AWEMGR.TwoThreads~1
  covers: req~AWEMGR.ConcurrentLoadingAndControl~1
  description: Loads a design, and while a relatively big subcanvas AWB
                is being loaded, it keeps setting a control value in a loop and checks the value is applied correctly.
                The test checks that the set control calls are not blocked by the loading of the design, and that they are applied correctly.

```
*/
TEST_F(AweMgrTestFixture, TwoThreads) {

    load_awc(awc_file);
    m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
    ASSERT_TRUE(m_ctx != NULL);

    // first load the main design, and check we can read values from the sink
    ASSERT_EQ(awemgr_load_design(m_ctx, "Main"), awemgr_RC_OK);
    delay_ms(10);

    // capture std::cout output to check the interleaving of events

    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    // define a loader thread, start it and loop setting values meanwhile

    std::thread loader([&]() {
        std::cout << "LOAD_SC:One-Start" << std::endl;
        awemgr_load_design(m_ctx, "One");
        std::cout << "LOAD_SC:One-End" << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // loop setting values
    for (int i = 1000; i >= 500; i -= 2)
    {
        std::cout << "SETVAL:DC_In.value:" << i << std::endl;
        ASSERT_EQ(setValue<uint32_t>("DC_In.value", i), true);

        // relax for a while to increase the chances of interleaving with the loading thread,
        // and also to allow the set value to be applied in the target before the next setValue call
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }
    loader.join();

    // restore std::cout
    std::cout.rdbuf(old_cout);

    // convert captured output into a sequence of events
    std::vector<std::string> seq = getEventSequence(buffer.str());

    // count the number of SETVAL events between LOAD-START and LOAD-END
    int nr_interleaved = 0;
    bool insideLoad = false;
    for (const auto& event : seq) {
        std::cout << "SEQ:" << event << ":" << insideLoad << ":" << nr_interleaved << std::endl;
        if (event == "START") {
            insideLoad = true;
        } else if (event == "END") {
            insideLoad = false;
        } else if (event == "SET") {
            if (insideLoad) {
                nr_interleaved++;
            }
        }
    }
    printf("Number of SETVAL events detected between Load-Start and Load-End: %d\n", nr_interleaved);
    EXPECT_GT(nr_interleaved, 20) << "Not enough SETVAL events were detected between Load-Start and Load-End.";

}

/**
```yaml
- id: itest~AWEMGR.ConcurrentTransactAndApi~1
  covers: req~AWEMGR.ConcurrentLoadingAndControl~1
  description: Verifies that awemgr_transact and high-level API calls (awemgr_get_target_info)
               can run concurrently without corrupting each other's responses.  Before the
               fix, awemgr_transact bypassed the channel_protection_mutex, allowing its
               TX→RX round-trip to interleave with shell-command transactions on the shared
               backend socket.  This test reliably triggers that race without the fix and
               must pass with zero errors after it.

```
*/
TEST_F(AweMgrTestFixture, ConcurrentTransactAndApiCall)
{
    /* AWE GetTargetInfo in wire format (same packet used in test_tuning_server.cpp):
     * 2-word packet, bits[31:16] = 2 (len_words), bits[15:0] = 0x0029 (opcode). */
    static const int REQ_WORDS = 2;
    static const int RSP_WORDS = 1024;
    static const int ITERATIONS = 2000;

    std::atomic<int> transact_errors{0};
    std::atomic<int> api_errors{0};

    aweconfig_set(cfg_p, "mgr.comm.timeoutms", "2000");
    aweconfig_set(cfg_p, "mgr.api.log.level", "error");
    aweconfig_set(cfg_p, "mgr.comm.log.level", "error");
    aweconfig_set(cfg_p, "mgr.comm.trace.state", "off");

    /* Thread A: raw awemgr_transact calls — exercises the path that previously
     * lacked the channel_protection_mutex lock. */
    std::thread transact_thread([&]() {
        uint32_t rsp_buf[RSP_WORDS];
        for (int i = 0; i < ITERATIONS; ++i)
        {
            uint32_t req[REQ_WORDS] = {0x00020029u, 0x00020029u};
            enum awemgr_rc rc = awemgr_transact(
                m_mgr_p, req, REQ_WORDS, rsp_buf, RSP_WORDS);
            if (rc != awemgr_RC_OK) {
                ++transact_errors;
                std::cerr << "awemgr_transact error: " << rc << " index: " << i << std::endl;
            }
            aweosal_usleep(100); // small delay to increase chances of interleaving with API calls in main thread
        }
    });

    /* Main thread: high-level API calls that go through awecomm_get_cmdbuf /
     * awecomm_release_lock — the path that already held the mutex. */
    for (int i = 0; i < ITERATIONS; ++i)
    {
        awemgr_targetinfo info{};
        enum awemgr_rc rc = awemgr_get_target_info(m_mgr_p, &info);
        if (rc != awemgr_RC_OK) {
            ++api_errors;
            std::cerr << "awemgr_get_target_info error: " << rc << " index: " << i << std::endl;
        }
    }

    transact_thread.join();

    EXPECT_EQ(transact_errors.load(), 0)
        << "awemgr_transact returned errors during concurrent execution";
    EXPECT_EQ(api_errors.load(), 0)
        << "awemgr_get_target_info returned errors during concurrent execution";
}
