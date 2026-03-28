#include "test_fixtures.h"


/**
```yaml
- id: itest~AWEMGR.DetachFromSystem~1
  covers: req~AWEMGR.DetachFromRunningTarget~1
  description: |
    Ensures that AWE Manager detaches from a system without stopping the system.
    This includes the steps: a) Init and Run system, b) Modify something on system,
    c) Detach from system, d) Init AWE Manager only,
    e) Check if modifications are still present.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, DetachFromTarget) {

	// fixture has initialized and started the system
	// SourceInt_1.value contains 0xdead; see TEST_F(AweMgrTestFixtureSetGetAWCLoaded, VariableReadWrite)

	// update a variable with new value
	unsigned int new_value = 0xdeadaffe;
	ASSERT_EQ(awemgr_control_write(m_ctx, "SourceInt_1.value", 0, &new_value, 1, AWEMGR_VARTYPE_INTEGER), awemgr_RC_OK);

	// stop AWE Manager in "detach mode"
	ASSERT_EQ(awemgr_skip_unload_design_on_exit(m_ctx), awemgr_RC_OK);

	ASSERT_EQ(awemgr_exit(&m_mgr_p), awemgr_RC_OK);

	// sleep a while
	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	// re-init and load AWC - do not load AWB !!!
	ASSERT_EQ(awemgr_init(&cfg_p, &m_mgr_p), awemgr_RC_OK);
	ASSERT_EQ(awemgr_load_awc(m_mgr_p, AweMgrTestFixtureSetGetAWC::m_awc_file, 0), awemgr_RC_OK);
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	// read back the modified value from the system
	unsigned int response_buffer[10];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(m_ctx, "SourceInt_1.value", response_buffer, 10, &nr_words_in_buf, &typ), awemgr_RC_OK);
	EXPECT_EQ(response_buffer[0], 0xdeadaffe);
};

/**
```yaml
- id: itest~AWEMGR.DetachFromSystemFail~1
  covers: req~AWEMGR.DetachFromRunningTarget~1
  description: Checks incorrect usage of API. Call with incorrect parameter.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, DetachCallFail) {

	ASSERT_EQ(awemgr_skip_unload_design_on_exit(NULL), awemgr_RC_ERR);

}

/**
```yaml
- id: itest~AWEMGR.SkipUnloadTwice~1
  covers: req~AWEMGR.DetachFromRunningTarget~1
  description: Checks that double skip-marking is not returned as error.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, SkipUnloadTwice) {

	ASSERT_EQ(awemgr_skip_unload_design_on_exit(m_ctx), awemgr_RC_OK);
	ASSERT_EQ(awemgr_skip_unload_design_on_exit(m_ctx), awemgr_RC_OK);
}
