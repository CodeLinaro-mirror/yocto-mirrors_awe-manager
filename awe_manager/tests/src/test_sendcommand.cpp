#include "test_fixtures.h"
#include <thread>
#include "Errors.h"

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.SendSpecialCommand_Fail~1
  covers: req~AWEMGR.SendBSPCommand~1
  description: Checks incorrect usage of API is handled correctly.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, SendSpecialCommand_Fail)
{
	int dummy = 0xdead;
	ASSERT_EQ(awemgr_send_command(NULL, 1, 1, &dummy, 4, NULL, 0, NULL), awemgr_RC_ERR);  // no ctx

	ASSERT_EQ(awemgr_send_command(m_ctx, 1, 1, NULL, 1, NULL, 0, NULL), awemgr_RC_ERR);   // no data but sz=1
	ASSERT_EQ(awemgr_send_command(m_ctx, 1, 1, &dummy, 0, NULL, 0, NULL), awemgr_RC_ERR); // data but sz=0

	ASSERT_EQ(awemgr_send_command(m_ctx, 1, 1, &dummy, 4, NULL, 2, NULL), awemgr_RC_ERR); // no respbuf but respsz
	ASSERT_EQ(awemgr_send_command(m_ctx, 1, 1, &dummy, 4, &dummy, 0, NULL), awemgr_RC_ERR); // respbuf but sz=0

	unsigned int buffer_too_large[500];
	ASSERT_EQ(awemgr_send_command(m_ctx, 1, 1, buffer_too_large, 500, NULL, 0, NULL), awemgr_RC_ERR); // req buffer too large
}

/**
```yaml
- id: itest~AWEMGR.SendSpecialCommand_Fail_onAweCore~1
  covers: req~AWEMGR.SendBSPCommand~1
  description: Sends an arbitrary (unknown) command to AWECore and detects that it failed.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, SendSpecialCommand_Fail_onAweCore)
{
	int dummy = 0xdead;
	ASSERT_EQ(awemgr_send_command(m_ctx, 1234, 0, &dummy, 1, NULL, 0, NULL), awemgr_RC_AWECORE_ERROR);
	ASSERT_EQ(awemgr_get_awe_error_code(), E_BADPACKET);
}
