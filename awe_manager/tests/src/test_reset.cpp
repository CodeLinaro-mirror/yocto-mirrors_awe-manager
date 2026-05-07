#include "test_fixtures.h"


/**
```yaml
- id: itest~AWEMGR.ResetAllStates~1
  covers: req~AWEMGR.Resetting~1
  description: Ensures that all module states are cleared.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, StateReset) {

	ASSERT_EQ(awemgr_reset_state_all(m_mgr_p), awemgr_RC_OK);

};
