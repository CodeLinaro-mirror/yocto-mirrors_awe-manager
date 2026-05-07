#include "test_fixtures.h"

/**
```yaml
- id: itest~AWEMGR.SuppressReturnValues.AudioStartStopSendTwiceOk~1
  covers:
    - req~AWEMGR.Sleep~1
    - req~AWEMGR.Resume~1
    - req~AWEMGR.SupressionAWECoreReturnValues~1
  description: |
    Checks that commands can be sent twice in a row without error, and that the second command has the expected effect (e.g. calling awemgr_audio_stop twice should not cause an error, and the second call should keep the audio stopped).
```
*/
TEST_F(AweMgrTestSuppressReturnValues, AudioStartStopSendTwiceOk)
{
    ASSERT_EQ(awemgr_audio_stop(m_ctx), awemgr_RC_OK);
    ASSERT_EQ(awemgr_audio_stop(m_ctx), awemgr_RC_OK);
    ASSERT_EQ(awemgr_audio_stop(m_ctx), awemgr_RC_OK);
    ASSERT_EQ(awemgr_audio_start(m_ctx), awemgr_RC_OK);
    ASSERT_EQ(awemgr_audio_start(m_ctx), awemgr_RC_OK);
    ASSERT_EQ(awemgr_audio_start(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.SuppressReturnValues.GetProfileNotPumpingOK~1
  covers:
    - req~AWEMGR.SupressionAWECoreReturnValues~1
  description: |
    Checks that when the profile values are requested while an instance is not pumping,
    the command is still processed and returns the expected data,
    instead of returning an error code due to no layouts being active.
```
*/
TEST_F(AweMgrTestSuppressReturnValues, GetProfileNotPumpingOK)
{
    awemgr_cpuinfo info_buffer;
    // first call will definitely be ok; it targets the PAC
    ASSERT_EQ(awemgr_get_cpu_info(m_mgr_p, 0, 0, &info_buffer), awemgr_RC_OK);
    // the signal flow under test has no instance 3 part, so
    // without suppressing the error code, this would cause a test failure;
    // with the suppression, we can check that the command is still processed and returns the expected data
    ASSERT_EQ(awemgr_get_cpu_info(m_mgr_p, 0, 3, &info_buffer), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.SuppressReturnValues.GetAllProfileNotPumpingOK~1
  covers:
    - req~AWEMGR.SupressionAWECoreReturnValues~1
  description: |
    Checks that when the profile layout values are requested while an instance is not pumping,
    the command is still processed and returns the expected data,
    instead of returning an error code due to no layouts being active.
```
*/
TEST_F(AweMgrTestSuppressReturnValues, GetAllProfileNotPumpingOK)
{
    awemgr_layoutinfo info_buffer;

    // first call will definitely be ok; it targets the PAC
    ASSERT_EQ(awemgr_get_layout_info(m_mgr_p, 0, 0, &info_buffer), awemgr_RC_OK);
    // the signal flow under test has no instance 3 part, so
    // without suppressing the error code, this would cause a test failure;
    // with the suppression, we can check that the command is still processed and returns the expected data
    ASSERT_EQ(awemgr_get_layout_info(m_mgr_p, 0, 3, &info_buffer), awemgr_RC_OK);
}
