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
- id: itest~AWEMGR.LoadDesign_Fail~2
  covers: req~AWEMGR.LoadDesign~1
  description: |
    Checks incorrect usage of loading an incorrect design.
    Also assumes a case (with "PresetWithAProblem") in which user has a correct AWC
    file format, but forgot to add the AWB data file to the AWC directory.
    '~2' now checks that the correct error code is returned when the design is not found in the AWC.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, LoadDesign_Fail) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);
	EXPECT_EQ(awemgr_load_design(NULL, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_load_design(awectx_p, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_load_design(awectx_p, "I AM A NON EXISTING DESIGN NAME"), awemgr_RC_ERR_DESIGN_NOTFOUND);
	EXPECT_EQ(awemgr_load_design(awectx_p, "PresetWithAProblem"), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.UnloadDesign_Fail~2
  covers: req~AWEMGR.UnloadDesign~1
  description: |
    Checks failure cases of unloading the design.
    '~2' now checks that the correct error code is returned when the design is not found in the AWC.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, UnloadDesign_Fail) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);
	EXPECT_EQ(awemgr_unload_design(NULL, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_unload_design(awectx_p, "Main"), awemgr_RC_OK); // No design is loaded, return OK
	EXPECT_EQ(awemgr_unload_design(awectx_p, ""), awemgr_RC_ERR_DESIGN_NOTFOUND); // No design is found, return Error
}

extern "C"
{
	// this is a private function not exposed via header
	enum awemgr_rc parse_plugin_string(const char* pluginList, struct awemgr_design_info *design_info);
}

/**
```yaml
- id: itest~AWEMGR.PluginInfo.Count~1
  covers: req~AWEMGR.PluginInfo~1
  description: |
    Ensures that the parser successfully processes up to AWEMGR_MAX_PLUGINS_PER_DESIGN
    and returns an error when the count is exceeded.
```
*/
TEST_F(PluginParserTest, MaxPluginsBoundary) {
    std::string input = "";
	// Generate a string to parse
    for (int i = 0; i < AWEMGR_MAX_PLUGINS_PER_DESIGN; ++i) {
        input += "p" + std::to_string(i) + ".so@0";
        if (i < AWEMGR_MAX_PLUGINS_PER_DESIGN - 1) input += "|";
    }

    // Should pass at AWEMGR_MAX_PLUGINS_PER_DESIGN
    EXPECT_EQ(parse_plugin_string(input.c_str(), &design_info), awemgr_RC_OK);
    EXPECT_EQ(design_info.plugin_count, AWEMGR_MAX_PLUGINS_PER_DESIGN);

    // Should fail at AWEMGR_MAX_PLUGINS_PER_DESIGN + 1
    input += "|extra.so@1";
    EXPECT_EQ(parse_plugin_string(input.c_str(), &design_info), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.PluginInfo.Malformed~1
  covers: req~AWEMGR.PluginInfo~1
  description: |
    Ensures the parser rejects strings with missing mandatory components (ID separator '@'
    or plugin name)
```
*/
TEST_F(PluginParserTest, MalformedStrings) {
    EXPECT_EQ(parse_plugin_string("plugin.so@", &design_info), awemgr_RC_ERR);
    EXPECT_EQ(parse_plugin_string("plugin.so", &design_info), awemgr_RC_ERR);
    EXPECT_EQ(parse_plugin_string("plugin.so|1@plugin2.so|2", &design_info), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.PluginInfo.NameLen~1
  covers: req~AWEMGR.PluginInfo~1
  description: |
    Ensures that plugin names up to (AWEMGR_MAX_PLUGIN_NAME_LEN - 1) are accepted
    and names exceeding the buffer size cause a parse error.
```
*/
TEST_F(PluginParserTest, NameLengthValidation) {
    // Name at 63 characters (OK for 64-byte buffer)
    std::string max_name(AWEMGR_MAX_PLUGIN_NAME_LEN - 1, 'a');
    std::string valid_input = max_name + "@1";
    EXPECT_EQ(parse_plugin_string(valid_input.c_str(), &design_info), awemgr_RC_OK);

    // Name at 64 characters (Error - no room for null terminator)
    std::string long_name(AWEMGR_MAX_PLUGIN_NAME_LEN, 'b');
    std::string invalid_input = long_name + "@1";
    EXPECT_EQ(parse_plugin_string(invalid_input.c_str(), &design_info), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.PluginInfo.CoreID~1
  covers: req~AWEMGR.PluginInfo~1
  description: |
    Validates that the Core ID must be within the range [0, MAX_AWE_ENDPOINTS - 1].
```
*/
TEST_F(PluginParserTest, CoreIDRangeValidation) {
    EXPECT_EQ(parse_plugin_string("p.so@15", &design_info), awemgr_RC_OK);
    // Out of bounds (16)
    EXPECT_EQ(parse_plugin_string("p.so@16", &design_info), awemgr_RC_ERR);
    // Negative ID
    EXPECT_EQ(parse_plugin_string("p.so@-1", &design_info), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.PluginInfo.NullInput~1
  covers: req~AWEMGR.PluginInfo~1
  description: |
    Ensures that the NULL input pointers are handled gracefully.
```
*/
TEST_F(PluginParserTest, NullInputHandling) {
    EXPECT_EQ(parse_plugin_string(NULL, &design_info), awemgr_RC_ERR);
    EXPECT_EQ(parse_plugin_string("p.so@0", NULL), awemgr_RC_ERR);
}