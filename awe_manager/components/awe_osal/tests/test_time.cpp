#include <gtest/gtest.h>
#include <thread>
#include "awosal_time.h"

/**
```yaml
- id: utest~AWEMGR.AWOSAL.Time1Second~1
  covers: dsn~AWEMGR.AWOSAL.TimeMeasure~1
  description: Ensures the time measurement functions correctly measure approximately 1 second.
```
*/
TEST(AWEOSALTimeTests, Wait1Second) {
    aweosal_clock_time start = aweosal_measure_start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    double elapsed_ms = aweosal_measure_elapsed(start);
    EXPECT_GE(elapsed_ms, 1000.0);
}

