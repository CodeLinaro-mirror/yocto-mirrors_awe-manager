#include "test_fixtures.h"


/**
```yaml
- id: itest~AWEMGR.Design_And_Preset_Load~1
  covers: 
    - req~AWEMGR.LoadDesign~1
    - req~AWEMGR.Preset_Selection~1
  description: |
    Loads the SetGet AWB; then it reads the values of the integer SINK modules
    and compares them to the know values (from the design);
    a first presetAWB is applied which will change the values of the corresponding
    integer SOURCE modules, and the SINK modules are queried again,
    this time for a different set of values; finally, a presetAWB with original
    values is restored.

```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, PresetLoad) {
    enum awemgr_rc rc;
	unsigned int response_buffer[10], response_value;
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;

	unsigned int expected_orig_1 = 0xdead;
	unsigned int expected_modified_1 = 0xaffedead;
	unsigned int expected_orig_10[10] = {10, 170, 57005, 0, 0, 0, 0, 10, 170, 57005};
	unsigned int expected_modified_10[10] = {0xa00aa00a, 0xaa0000aa, 0xdeaddead, 0xF0000000, 0xF0000000, 0xF0000000, 0xF0000000, 0xa00aa00a, 0xaa0000aa, 0xdeaddead};

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", &response_value, sizeof(response_value), &nr_words_in_buf, &typ), awemgr_RC_OK);
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, sizeof(response_buffer), &nr_words_in_buf, &typ), awemgr_RC_OK);

	EXPECT_EQ(response_value, expected_orig_1);
	EXPECT_TRUE(ArraysEqual(response_buffer, expected_orig_10, 10));

	// load a preset for the integer Source module
	rc = awemgr_load_design(m_ctx, "Modified");
	ASSERT_EQ(rc, awemgr_RC_OK);

	delay_ms(20); // give AWE pumping a little bit

	// repeat the reading
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", &response_value, sizeof(response_value), &nr_words_in_buf, &typ), awemgr_RC_OK);
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, sizeof(response_buffer), &nr_words_in_buf, &typ), awemgr_RC_OK);

	EXPECT_EQ(response_value, expected_modified_1);
	EXPECT_TRUE(ArraysEqual(response_buffer, expected_modified_10, 10));

	// revert to original settings
	rc = awemgr_load_design(m_ctx, "Original");
	ASSERT_EQ(rc, awemgr_RC_OK);

	delay_ms(20); // give AWE pumping a little bit

	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", &response_value, sizeof(response_value), &nr_words_in_buf, &typ), awemgr_RC_OK);
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_10.value", response_buffer, sizeof(response_buffer), &nr_words_in_buf, &typ), awemgr_RC_OK);

	EXPECT_EQ(response_value, expected_orig_1);
	EXPECT_TRUE(ArraysEqual(response_buffer, expected_orig_10, 10));

};


/**
```yaml
- id: itest~AWEMGR.LoadDesign_Fail~1
  covers: req~AWEMGR.LoadDesign~1
  description: |
    Checks incorrect usage of loading an incorrect design. 
    Also assumes a case (with "PresetWithAProblem") in which user has a correct AWC
    file format, but forgot to add the AWB data file to the AWC directory.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, LoadDesign_Fail) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);
	EXPECT_EQ(awemgr_load_design(NULL, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_load_design(awectx_p, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_load_design(awectx_p, "I AM A NON EXISTING DESIGN NAME"), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_load_design(awectx_p, "PresetWithAProblem"), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.UnloadDesign_Fail~1
  covers: req~AWEMGR.UnloadDesign~1
  description: |
    Checks failure cases of unloading the design. 
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, UnloadDesign_Fail) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);
	EXPECT_EQ(awemgr_unload_design(NULL, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_unload_design(awectx_p, "Main"), awemgr_RC_OK); // No design is loaded, return OK
	EXPECT_EQ(awemgr_unload_design(awectx_p, ""), awemgr_RC_ERR); // No design is found, return Error
}