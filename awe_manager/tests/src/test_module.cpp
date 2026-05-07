#include "test_fixtures.h"


/**
```yaml
- id: itest~AWEMGR.GetModuleHandle~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: Ensures that a module handle is returned correctly.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, ModuleHandle) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	struct awemgr_module module_info;
	ASSERT_EQ(awemgr_get_module_by_name(awectx_p, "SinkInt_10", &module_info), awemgr_RC_OK);

	ASSERT_EQ(module_info.instanceId, 0);
	ASSERT_EQ(module_info.objectId, 30005);
	ASSERT_EQ(module_info.classId, 3203336302);
	ASSERT_STREQ(module_info.name, "SinkInt_10");
};


/**
```yaml
- id: itest~AWEMGR.GetModuleHandleByIndex~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: Ensures that a module handle is returned correctly at a given index.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, ModuleHandleByIndex) {

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	int nr_mods = awemgr_get_modules_count(awectx_p);

	ASSERT_EQ(nr_mods, 12);

	struct awemgr_module mod_info;
    ASSERT_EQ(awemgr_get_module_by_index(awectx_p, 0, &mod_info), awemgr_RC_OK);

	ASSERT_EQ(mod_info.objectId, 30000);
	ASSERT_STREQ(mod_info.name, "SourceFloat_10");

};

/**
```yaml
- id: itest~AWEMGR.GetModuleHandleByIndex_Fail~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: Ensures that incorrect parameters are handled ok.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, ModuleHandleByIndex_Fail) {
    enum awemgr_rc rc;

	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	int nr_mods = awemgr_get_modules_count(NULL);
	ASSERT_EQ(nr_mods, -1);

	struct awemgr_module mod_info;
    rc = awemgr_get_module_by_index(NULL, 0, &mod_info);
	ASSERT_EQ(rc, awemgr_RC_ERR);

    rc = awemgr_get_module_by_index(awectx_p, -1, &mod_info);
	ASSERT_EQ(rc, awemgr_RC_ERR);

    rc = awemgr_get_module_by_index(awectx_p, 0, NULL);
	ASSERT_EQ(rc, awemgr_RC_ERR);
};

/**
```yaml
- id: itest~AWEMGR.GetModuleHandle_Fail~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: Checks incorrect usage of module handle retrival.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, ModuleHandle_Fail) {
	struct awemgr_ctx *awectx_p = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	struct awemgr_module module_info;

	EXPECT_EQ(awemgr_get_module_by_name(NULL, "SinkInt_10", &module_info), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_module_by_name(awectx_p, "SinkInt_10", NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_module_by_name(awectx_p, "SinkInt_10_DOES_NOT_EXIST", &module_info), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_get_module_by_name(awectx_p, NULL, &module_info), awemgr_RC_ERR);
};

/**
```yaml
- id: itest~AWEMGR.GetModuleClass~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: Ensures a module's class ID can be retrieved.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, GetModuleClass) {
	struct awemgr_module module_info;
	EXPECT_EQ(awemgr_get_module_by_name(m_ctx, "SinkInt_10", &module_info), awemgr_RC_OK);

	unsigned int class_id;
	EXPECT_EQ(awemgr_module_get_class(m_ctx, module_info, &class_id), awemgr_RC_OK);
	EXPECT_EQ(class_id, 0xBEEF086E); // 3203336302
};

/**
```yaml
- id: itest~AWEMGR.ModuleOperationState~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: Checks if runtime state can be set and queried again.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, ModuleOperationState) {
	// get handle to source module
	struct awemgr_module source_module;
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "SourceInt_1", &source_module), awemgr_RC_OK);

	// check the OP state
	enum awemgr_module_runtimestate state;
	ASSERT_EQ(awemgr_module_get_state(m_ctx, source_module, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);

	// mute the module
	ASSERT_EQ(awemgr_module_set_state(m_ctx, source_module, MODULE_MUTED), awemgr_RC_OK);
	ASSERT_EQ(awemgr_module_get_state(m_ctx, source_module, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_MUTED);

	// bypass the module
	ASSERT_EQ(awemgr_module_set_state(m_ctx, source_module, MODULE_BYPASS), awemgr_RC_OK);
	ASSERT_EQ(awemgr_module_get_state(m_ctx, source_module, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_BYPASS);

	// inactivate the module
	ASSERT_EQ(awemgr_module_set_state(m_ctx, source_module, MODULE_INACTIVE), awemgr_RC_OK);
	ASSERT_EQ(awemgr_module_get_state(m_ctx, source_module, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_INACTIVE);
};


/**
```yaml
- id: itest~AWEMGR.ModuleOperationStateDownStream~1
  covers: req~AWEMGR.ModuleAdministration~1
  description: |
    Checks if operational state change really has influence on downstream SINK module
```
*/
TEST_F(AweMgrTestFixtureSetGetAWCLoaded, ModuleOperationStateDownStream) {
	// get handle to source module
	struct awemgr_module source_module;
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "SourceInt_1", &source_module), awemgr_RC_OK);

	// check the OP state
	enum awemgr_module_runtimestate state;
	ASSERT_EQ(awemgr_module_get_state(m_ctx, source_module, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);

	// read from "downstream" module, value should be the default as from design
	unsigned int response_buffer[1];
	unsigned int nr_words_in_buf;
	enum awemgr_vartype typ;
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", response_buffer, sizeof(response_buffer), &nr_words_in_buf, &typ), awemgr_RC_OK);
	ASSERT_EQ(response_buffer[0], 0xdead);

	// mute the source module
	ASSERT_EQ(awemgr_module_set_state(m_ctx, source_module, MODULE_MUTED), awemgr_RC_OK);

	usleep(1000*10); // give AWE pumping a little bit

	// repeat reading downstream module, should be 0 now
	ASSERT_EQ(awemgr_control_read(m_ctx, "SinkInt_1.value", response_buffer, sizeof(response_buffer), &nr_words_in_buf, &typ), awemgr_RC_OK);
	ASSERT_EQ(response_buffer[0], 0);

};
