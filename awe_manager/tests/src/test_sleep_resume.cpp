#include "test_fixtures.h"

/**
```yaml
- id: itest~AWEMGR.SleepResume.Success~1
  covers:
    - req~AWEMGR.Sleep~1
    - req~AWEMGR.Resume~1
  description: |
    This test shows:
      - Audio pumping is stopped on calling awemgr_audio_stop by checking that the event count does not increase when pumping is stopped.
      - Audio pumping is resumed again on calling awemgr_audio_start by checking that the event count increase during pumping.
```
*/
TEST_F(AweMgrTestSleepResume, MgrSleepResume)
{
  ASSERT_EQ(awemgr_events_start(m_ctx, nullptr, nullptr), awemgr_RC_OK);
  ASSERT_EQ(getValue<uint32_t>("Event2.triggerCnt"), 0);  // Trigger count starting value = 0
  ASSERT_EQ(setValue<float>("Scaler1.gain", 20.f), true); // Start event trigger
  delay_ms(10);
  ASSERT_TRUE(getValue<uint32_t>("Event2.triggerCnt") > 0); // Ensure Pumping started by checking the event trigger count

  ASSERT_EQ(awemgr_audio_stop(m_ctx), awemgr_RC_OK);
  delay_ms(10);
  auto triggerCount = getValue<uint32_t>("Event2.triggerCnt"); // Take note of trigger count after initiating sleep
  delay_ms(500);

  ASSERT_EQ(getValue<uint32_t>("Event2.triggerCnt"), triggerCount); // Trigger count remain same, means the pumping is stopped

  ASSERT_EQ(awemgr_audio_start(m_ctx), awemgr_RC_OK); // resume Audio
  delay_ms(500);

  ASSERT_TRUE(getValue<uint32_t>("Event2.triggerCnt") > triggerCount); // Expect more event triggers, as the pumping resumes

  ASSERT_EQ(setValue<float>("Scaler1.gain", 0.f), true);
  delay_ms(10);
  ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.SleepResume.Fail~1
  covers:
    - req~AWEMGR.Sleep~1
    - req~AWEMGR.Resume~1
  description: |
    This test shows that awemgr_audio_stop and awemgr_audio_start API's return error code when called with NULL Handle.
```
*/
TEST_F(AweMgrTestSleepResume, SleepResumeFailure)
{
  ASSERT_EQ(awemgr_audio_stop(NULL), awemgr_RC_ERR);
  ASSERT_EQ(awemgr_audio_start(NULL), awemgr_RC_ERR);
}
