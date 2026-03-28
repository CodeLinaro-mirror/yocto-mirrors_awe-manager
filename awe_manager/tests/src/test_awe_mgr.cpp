#include "test_fixtures.h"
#include "awemgr_logging.h"
#include "awe_awc.h"
#include "awe_ctrl.h"
#include "Errors.h"

static const char* awc_file = TEST_DATA_DIR "/designs/set_get/target_files/awc_index.txt";

static void my_cb(const awemgr_event* ev, void* userdata) {
    printf("Hello from CB - ev: %s(%d)\n", ev->module.name, ev->module.objectId);
}

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.Init_deInit~1
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks that AWEMGR can be initialized and uninitialized
```
*/
TEST_F(AweMgrMinimalTestFixture, BasicTests) {
	enum awemgr_rc rc;
    struct awemgr_data *mgr_p = NULL;
    struct awemgr_ctx *awectx_p;

	ASSERT_EQ(awemgr_init(NULL, NULL), awemgr_RC_ERR);

	aweconfig_set(cfg_p, "mgr.ctrl.socket.ip", "");
	aweconfig_set(cfg_p, "mgr.ctrl.socket.port", "");

	ASSERT_EQ(awemgr_init(&cfg_p,  &mgr_p), awemgr_RC_OK);
	ASSERT_TRUE(mgr_p!=NULL);

	aweconfig_set(cfg_p, "mgr.ctrl.socket.ip", "127.0.0.1");
	aweconfig_set(cfg_p, "mgr.ctrl.socket.port", "15002");

	ASSERT_EQ(awemgr_init(&cfg_p, &mgr_p), awemgr_RC_ERR); // Manager Already Initialized

	// Check with invalid endpoint
	ASSERT_EQ(awemgr_load_awc(mgr_p, awc_file, -2), awemgr_RC_ERR);

	// Check with invalid endpoint, valid values are 0 to 15
	ASSERT_EQ(awemgr_load_awc(mgr_p, awc_file, 18), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_load_awc(NULL, awc_file, 18), awemgr_RC_ERR);

	// Invalid file name
	ASSERT_EQ(awemgr_load_awc(mgr_p, "invalid file", 0), awemgr_RC_ERR);

    rc = awemgr_load_awc(mgr_p, awc_file, 0);
	ASSERT_EQ(rc, awemgr_RC_OK);

	awectx_p = awemgr_get_awc_context(mgr_p, 0);
	ASSERT_TRUE(awectx_p != NULL);

	ASSERT_EQ(awemgr_get_control_info_by_name(awectx_p, NULL, NULL), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_control_info_by_name(NULL, "test", NULL), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_control_info_by_name(awectx_p, "test", NULL), awemgr_RC_ERR);

	rc = awemgr_events_start(awectx_p, my_cb, NULL);
	ASSERT_EQ(rc, awemgr_RC_OK);

	int nr_ctls = awemgr_get_controls_count(awectx_p);
	ASSERT_EQ(nr_ctls, 35);

	ASSERT_EQ(awemgr_get_loaded_awc_count(NULL), -1);

	rc = awemgr_exit(&mgr_p);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_TRUE(mgr_p==NULL);

	rc = awemgr_exit(&mgr_p);
	ASSERT_EQ(rc, awemgr_RC_OK);

	rc = awemgr_exit(NULL);
	ASSERT_EQ(rc, awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.Init.Fail~1
  covers: req~AWEMGR.SoftwareComponent~1
  description: Makes sure init returns error when called with incorrect params.
```
*/
TEST_F(AweMgrMinimalTestFixture, InitFail) {
	enum awemgr_rc rc;
    struct awemgr_data *mgr_p = (struct awemgr_data *) &rc;

	rc = awemgr_init(NULL, NULL);
	ASSERT_EQ(rc, awemgr_RC_ERR);

	rc = awemgr_init(NULL, &mgr_p);
	ASSERT_EQ(rc, awemgr_RC_ERR);
}


/**
```yaml
- id: itest~AWEMGR.GetCtx.Fail~1
  covers: req~AWEMGR.MultipleDesignSupport~1
  description: Checks incorrect get context API calls.
```
*/
TEST_F(AweMgrTestFixture, GetCtxFail) {
    struct awemgr_ctx *awectx_p;

	// incorrect handle
	awectx_p = awemgr_get_awc_context(NULL, 1);
	ASSERT_TRUE(awectx_p == NULL);

	// still haven't loaded, so should return error!
	awectx_p = awemgr_get_awc_context(m_mgr_p, 1);
	ASSERT_TRUE(awectx_p == NULL);
}


/**
```yaml
- id: itest~AWEMGR.TargetInfo~2
  covers: req~AWEMGR.AweCoreAdministration~1
  description: Proves that information about target can be retrieved.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, TargetInfo) {

	awemgr_targetinfo info_buffer;
	int rc = awemgr_get_target_info(m_mgr_p, &info_buffer);

	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_EQ(info_buffer.nr_awe_instances, 4);  // for LinuxApp !!!

	ASSERT_EQ(info_buffer.instance[0].m_coreClockSpeed, 1.2e+09);
	ASSERT_EQ(info_buffer.instance[0].nr_threads, 2);
	ASSERT_EQ(info_buffer.instance[0].m_coreID, 0);
	ASSERT_STREQ(info_buffer.instance[0].targetName, "awe#0");

	ASSERT_EQ(info_buffer.instance[1].m_coreClockSpeed, 1.2e+09);
	ASSERT_EQ(info_buffer.instance[1].nr_threads, 2);
	ASSERT_EQ(info_buffer.instance[1].m_coreID, 1);
	ASSERT_STREQ(info_buffer.instance[1].targetName, "awe#1");

	ASSERT_EQ(info_buffer.instance[2].m_coreClockSpeed, 1.2e+09);
	ASSERT_EQ(info_buffer.instance[2].nr_threads, 2);
	ASSERT_EQ(info_buffer.instance[2].m_coreID, 2);
	ASSERT_STREQ(info_buffer.instance[2].targetName, "awe#2");
}


/**
```yaml
- id: itest~AWEMGR.Transact~1
  covers: req~AWEMGR.Raw_Access~1
  description: Check whether the raw-access method transact works correctly and returns the correct target info.
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, TransactTargetInfo) {

	unsigned int request_buffer[2] = {0x00020029, 0x00020029}; // corresponds to get target info : PFID_GetTargetInfo
	unsigned int response_buffer[14]; // to provide just enough for the target info

	int rc = awemgr_transact(m_mgr_p, (void*) request_buffer, sizeof(request_buffer), response_buffer, sizeof(response_buffer));
	ASSERT_EQ(rc, awemgr_RC_OK);

	// todo: for now first word of payload only, later use dedicated data structure
	ASSERT_EQ(response_buffer[0], 0xe0000);    // size (14) in upper 16bits
	ASSERT_EQ(response_buffer[1], 0);          // no error
	ASSERT_EQ(response_buffer[2], 0x473b8000) << "Sample Rate Mismatch, Expected 48000 Actual " << (float)response_buffer[2]; //Sample Rate (48000)
}

/**
```yaml
- id: itest~AWEMGR.Transact_Fail~1
  covers: req~AWEMGR.Raw_Access~1
  description: Checks if transact function now returns an error code of the underlying transact method (QAL-207).
```
*/
TEST_F(AweMgrTestFixtureSetGetAWC, Transact_Fail) {

	int rc = awemgr_transact(m_mgr_p, NULL, 0, NULL, 0);
	ASSERT_EQ(rc, awemgr_RC_ERR);
}


/**
```yaml
- id: itest~AWEMGR.Util_VarTypeString~2
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks the string output of the varType conversion.
```
*/
TEST_F(AweMgrMinimalTestFixture, VarTypeString) {

	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_UNSIGNED_INTEGER), "unsigned_integer");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_INTEGER), "integer");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_BOOL), "bool");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_FLOAT), "float");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_FRACT32), "fract32");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_FRACT16), "fract16");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_ENUM), "enum");
	ASSERT_STREQ(awemgr_vartype_to_string(AWEMGR_VARTYPE_UNDEF), "UNDEF");
}

/**
```yaml
- id: itest~AWEMGR.Util_VersionString~1
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks if version string can be retrieved.
```
*/
TEST_F(AweMgrMinimalTestFixture, VersionString) {
	const char *version = awemgr_get_version();
	ASSERT_TRUE(version != NULL);
	// todo: get better check here! Aman: any ideas?
	//  - version string is typically something like Mj.Mi.Pt[-nr-gitrev[-dirty]]
	//  - on server it is 0.0.0
}

/**
```yaml
- id: itest~AWEMGR.Miscelleneous.LogBuffer~3
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks that a buffer can be logged
```
*/
TEST(AweMgrMiscelleneous, LogBuffer) {
	::testing::internal::CaptureStdout();
	float buff = 1.0f;
	awemgr_log_buffer(&buff, sizeof(float), AWEMGR_LOG_VARTYPE_FLOAT, NULL, NULL);

	UINT32 buff_int = 0xdeadbeaf;
	awemgr_log_buffer(&buff_int, sizeof(UINT32), AWEMGR_LOG_VARTYPE_FRACT32, NULL, NULL);
	awemgr_log_buffer(&buff_int, sizeof(UINT32), AWEMGR_LOG_VARTYPE_INT, NULL, NULL);
	awemgr_log_buffer(&buff_int, sizeof(UINT32), -1, NULL, NULL);

	std::string output = ::testing::internal::GetCapturedStdout();
	EXPECT_EQ(output, "1\n-0.260323\n0xdeadbeaf\nNaN\n");

	::testing::internal::CaptureStdout();
	float buff_2[2] = {1.0f, 3.14f};
	awemgr_log_buffer(&buff_2, sizeof(float)*2, AWEMGR_LOG_VARTYPE_FLOAT, NULL, NULL);

	UINT32 buff_int_2[2] = {0xdeadbeaf, 0xdeadbeaf};
	awemgr_log_buffer(&buff_int_2, sizeof(UINT32)*2, AWEMGR_LOG_VARTYPE_FRACT32, NULL, NULL);
	awemgr_log_buffer(&buff_int_2, sizeof(UINT32)*2, AWEMGR_LOG_VARTYPE_INT, NULL, NULL);
	awemgr_log_buffer(&buff_int_2, sizeof(UINT32)*2, -1, NULL, NULL);

	std::string output_2 = ::testing::internal::GetCapturedStdout();
	EXPECT_EQ(output_2, "1, 3.14\n-0.260323, -0.260323\n0xdeadbeaf, 0xdeadbeaf\nNaN, NaN\n");

}

extern "C" {
enum awemgr_vartype awctype_2_awemgrtype(awc_ctl_type_t awc_type);
void copy_ctl_info(awc_ctl_t* var_p, struct awemgr_ctl_elem_info *info);
}

/**
```yaml
- id: itest~AWEMGR.Utils~2
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks the utility functions of AweMgr
```
*/
TEST(AweMgrMiscelleneous, Utils) {
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_FLOAT), AWEMGR_VARTYPE_FLOAT);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_INT32), AWEMGR_VARTYPE_INTEGER);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_UINT32), AWEMGR_VARTYPE_UNSIGNED_INTEGER);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_BOOL), AWEMGR_VARTYPE_BOOL);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_FRACT32), AWEMGR_VARTYPE_FRACT32);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_FRACT16), AWEMGR_VARTYPE_FRACT16);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_UNKNOWN), AWEMGR_VARTYPE_UNDEF);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_ENUM), AWEMGR_VARTYPE_ENUM);
	ASSERT_EQ(awctype_2_awemgrtype(AWC_CTL_TYPE_MAX), AWEMGR_VARTYPE_UNDEF);

	ASSERT_EQ(awctype_2_awemgrtype((awc_ctl_type_t)20), AWEMGR_VARTYPE_UNDEF);
	awc_ctl_t awcCtl;
	awcCtl.type = AWC_CTL_TYPE_MAX;
	awcCtl.range.def = 10;
	awcCtl.module = NULL;
	awcCtl.alias = (char*) "test-alias";
	struct awemgr_ctl_elem_info mgrCtl;
	copy_ctl_info(&awcCtl, &mgrCtl);
	ASSERT_TRUE(mgrCtl.id.parentName == NULL);
	ASSERT_STREQ(mgrCtl.id.alias, "test-alias");

	awc_module_t awcMod = { 0 };
	awcMod.name = (char*) "test";
	awcCtl.module = &awcMod;
	awcMod.alias = (char*) "test-alias";
	copy_ctl_info(&awcCtl, &mgrCtl);
	ASSERT_STREQ(mgrCtl.id.parentName, "test");
	ASSERT_STREQ(mgrCtl.id.alias, "test-alias");
}

/**
```yaml
- id: itest~AWEMGR.CommTimeout~1
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks that the communication timeout errors are propagated correctly
```
*/
TEST_F(AweCTRLTimeoutFixture, timeout)
{
	unsigned int nr_words_in_buf = 0;
	awemgr_vartype type = AWEMGR_VARTYPE_FLOAT;   // must be initialized for _write() method to avoid valgrind errors!
	UINT32 value = 0;
	ASSERT_EQ(awemgr_control_read(m_ctx, "Scaler1.gain", (void *)&value, 1, &nr_words_in_buf, &type), awemgr_RC_COMM_TIMEOUT);

	ASSERT_EQ(awemgr_control_write(m_ctx, "Scaler1.gain", 0, (void *)&value, 1), awemgr_RC_COMM_TIMEOUT);
}

/**
```yaml
- id: itest~AWEMGR.AWECoreErrors~1
  covers: req~AWEMGR.AWECoreErrors~1
  description: When AWE Manager returns error code awemgr_RC_AWECORE_ERROR, API awemgr_get_awe_error can be used to
               get the structure containing error code and description string.
```
*/
TEST_F(AweMgrTestFixtureMinimalWithErrors, AWECoreErrors)
{
	struct awemgr_module module;
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Scaler1", &module), awemgr_RC_OK);

	// This test fixture uses an awc file with invalid object id for scaler1 module
	// This forces awecore to return an error in the response packet
	ASSERT_EQ(awemgr_module_set_state(m_ctx, module, MODULE_BYPASS), awemgr_RC_AWECORE_ERROR);
	awecore_error_t* pErr = awemgr_get_awe_error(m_mgr_p);
	ASSERT_TRUE(pErr != NULL);
	ASSERT_EQ(pErr->error_code, E_NO_MORE_OBJECTS);
	ASSERT_STREQ(pErr->error_desc, "no more objects found");

	// The awc file contains the valid handle for gain, so awemgr_control_write should succeed
	unsigned int new_value = 10.f;
	ASSERT_EQ(awemgr_control_write(m_ctx, "Scaler1.gain", 0, &new_value, 1), awemgr_RC_OK);
	pErr = awemgr_get_awe_error(m_mgr_p);
	ASSERT_TRUE(pErr != NULL);
	ASSERT_EQ(pErr->error_code, E_SUCCESS);
	ASSERT_STREQ(pErr->error_desc, "success");

	pErr = awemgr_get_awe_error(NULL);
	ASSERT_TRUE(pErr == NULL);
}