#include "test_fixtures.h"

static const char* awc_file = TEST_DATA_DIR "/designs/set_get/target_files/awc_index.txt";

/**
```yaml
- id: itest~AWEMGR.Init_MultiInstance.Auto~1
  covers: req~AWEMGR.MultipleDesignSupport~1
  description: Loads AWC files for multiple instances; assignes instanceIds sequentially.
```
*/
TEST_F(AweMgrTestFixture, MultiInstanceAutoSuggest) {
    enum awemgr_rc rc;
	
	rc = awemgr_load_awc(m_mgr_p, awc_file, 0);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), 1);
	
    rc = awemgr_load_awc(m_mgr_p, awc_file, 1);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), 2);
};


/**
```yaml
- id: itest~AWEMGR.Init_MultiInstance.Specific~1
  covers: req~AWEMGR.MultipleDesignSupport~1
  description: Loads AWC files for multiple instances given specific endpointId
```
*/
TEST_F(AweMgrTestFixture, MultiInstanceSpecific) {
    enum awemgr_rc rc;
	
	rc = awemgr_load_awc(m_mgr_p, awc_file, 2);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), 1);

    rc = awemgr_load_awc(m_mgr_p, awc_file, 0);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), 2);
};

/**
```yaml
- id: itest~AWEMGR.Init_MultiInstance.Fail~1
  covers: req~AWEMGR.MultipleDesignSupport~1
  description: Checks various incorrect API usages specific for setting up multi-instance AWC
```
*/
TEST_F(AweMgrTestFixture, MultiInstanceFailure) {
    enum awemgr_rc rc;
	
	// check filling up max + too much;
	// after loop there are endpointIds 0, 1, 2 and 3
	for (int x=0; x < MAX_AWE_ENDPOINTS; x++)
	{
		rc = awemgr_load_awc(m_mgr_p, awc_file, x);
		ASSERT_EQ(rc, awemgr_RC_OK);
		ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), x+1);
	}

	ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), MAX_AWE_ENDPOINTS);

	// there should be no AWC with this endpointId
	ASSERT_TRUE(awemgr_get_awc_context(m_mgr_p, 55) == NULL);

	rc = awemgr_load_awc(m_mgr_p, awc_file, -1);
	ASSERT_EQ(rc, awemgr_RC_ERR);

	rc = awemgr_unload_awc(NULL, -1);
	ASSERT_EQ(rc, awemgr_RC_ERR);

	rc = awemgr_unload_awc(m_mgr_p, -1);
	ASSERT_EQ(rc, awemgr_RC_ERR);
};
