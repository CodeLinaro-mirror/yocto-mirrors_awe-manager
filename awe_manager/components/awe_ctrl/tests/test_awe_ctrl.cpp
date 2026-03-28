#include <gtest/gtest.h>
#include "awe_ctrl.h"

/* ****************************************************************************
 * FIXTURES
 * ***************************************************************************/

void set_test_traces()
{
	awectrl_set_traces(true);
	bool traces_enabled = awectrl_get_traces();
}

class AweCtrlTestFixture: public testing::Test
{
	public:
		void SetUp() override;
		void TearDown() override;

	protected:
		struct awectrl_data *m_ctrl_ctx;
		awe_config *cfg_p;
};

void AweCtrlTestFixture::SetUp()
{
	set_test_traces();

	const char* awb_file = NULL; 
	//TODO: use c++ 17 filesystem
#ifdef WIN32
	// Windows: Use GetTempPath or a hardcoded path
	awb_file = "C:\\temp\\comm-dump.awb";
#else
	// Unix-like: Use /tmp directory
	awb_file = "/tmp/comm-dump.awb";
#endif
	FILE* fp = fopen(awb_file, "w");
	awectrl_set_trace_file(fp);

	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awectrl_register_configs(cfg_p), AWECFG_RC_OK);
	int rc = awectrl_init(cfg_p, NULL, &m_ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);
}

void AweCtrlTestFixture::TearDown()
{
	FILE *fp = awectrl_get_trace_file();
	fclose(fp);

	int rc = awectrl_exit(&m_ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);
	ASSERT_TRUE(m_ctrl_ctx == NULL);

	ASSERT_TRUE(aweconfig_destroy(&cfg_p) != -1);
}

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.ControlComm.InitExit~1
  covers: dsn~AWEMGR.ControlComm.Initialize~1
  description: |
    Checks that component can be created and safely destructed.
```
*/
TEST(AweCtrlTests, InitExit) {

	struct awectrl_data *ctrl_ctx;

	set_test_traces();
	awe_config *cfg_p;
	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awectrl_register_configs(cfg_p), AWECFG_RC_OK);

	int rc = awectrl_init(cfg_p, NULL, &ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);
	ASSERT_TRUE(ctrl_ctx != NULL);

	struct awecmd_st *cmdbuf_p;
	for (int i=0; i<MAX_AWECTRL_COMM_CHANNELS; i++)
	{
		rc = awectrl_get_cmdbuf(ctrl_ctx, i, &cmdbuf_p);
		ASSERT_EQ(rc, AWECTRL_RC_OK);
		ASSERT_EQ(cmdbuf_p->response_buffer_size, 264);
	}

	rc = awectrl_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);
	ASSERT_TRUE(ctrl_ctx == NULL);

	// second exit will only dump message, but returns ok
	rc = awectrl_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);

	ASSERT_EQ(aweconfig_destroy(&cfg_p), AWECFG_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.TuningBufferSize~1
  covers: dsn~AWEMGR.ControlComm.Initialize~1
  description: |
    Checks that the default tuning buffer size can be modified via awe_config 
```
*/
TEST(AweCtrlTests, TuningBufferSize) {

	struct awectrl_data *ctrl_ctx;

	set_test_traces();
	awe_config *cfg_p;
	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awectrl_register_configs(cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.ctrl.buffersize", "4096"), AWECFG_RC_OK);

	int rc = awectrl_init(cfg_p, NULL, &ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);
	ASSERT_TRUE(ctrl_ctx != NULL);

	struct awecmd_st *cmdbuf_p;
	for (int i=0; i<MAX_AWECTRL_COMM_CHANNELS; i++)
	{
		rc = awectrl_get_cmdbuf(ctrl_ctx, i, &cmdbuf_p);
		ASSERT_EQ(rc, AWECTRL_RC_OK);
		ASSERT_EQ(cmdbuf_p->response_buffer_size, 4096);
	}

	rc = awectrl_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);
	ASSERT_TRUE(ctrl_ctx == NULL);

	// second exit will only dump message, but returns ok
	rc = awectrl_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECTRL_RC_OK);

	ASSERT_EQ(aweconfig_destroy(&cfg_p), AWECFG_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.InitExit_Fail~1
  covers: dsn~AWEMGR.ControlComm.Initialize~1
  description: |
    Checks incorrect context handle parameters for init and exit methods.
```
*/
TEST(AweCtrlTests, InitExit_Fail) {
	set_test_traces();
	int rc = awectrl_init(NULL, NULL, NULL);
	ASSERT_EQ(rc, AWECTRL_RC_FAIL_PARAM);

	rc = awectrl_exit(NULL);
	ASSERT_EQ(rc, AWECTRL_RC_FAIL_PARAM);

	ASSERT_EQ(awectrl_notify(NULL, 0), AWECTRL_RC_FAIL_PARAM);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.TransAct_Fail~1
  covers: dsn~AWEMGR.ControlComm.SelectChannel~1
  description: |
    Checks errors when trying to specify incorrect transact parameters.
```
*/
TEST_F(AweCtrlTestFixture, TransAct_Fail) 
{
	ASSERT_EQ(awectrl_transact(m_ctrl_ctx, -1), AWECTRL_RC_FAIL_PARAM);
	ASSERT_EQ(awectrl_transact(NULL, -1), AWECTRL_RC_FAIL_PARAM);
	ASSERT_EQ(awectrl_transact(m_ctrl_ctx, MAX_AWECTRL_COMM_CHANNELS), AWECTRL_RC_FAIL_PARAM);

	ASSERT_EQ(awectrl_transact_explicit(NULL, NULL, 0, NULL, 0, -1), AWECTRL_RC_FAIL_PARAM);
	ASSERT_EQ(awectrl_transact_explicit(m_ctrl_ctx, NULL, 0, NULL, 0, -1), AWECTRL_RC_FAIL_PARAM);
	uint32_t data;
	ASSERT_EQ(awectrl_transact_explicit(m_ctrl_ctx, NULL, 0, &data, 0, -1), AWECTRL_RC_FAIL_PARAM);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.GetBuf_Fail~1
  covers: dsn~AWEMGR.ControlComm.SelectChannel~1
  description: |
    Checks errors when trying to get a CMD buffer object.
```
*/
TEST_F(AweCtrlTestFixture, GetBuf_Fail) {
	struct awecmd_st *cmdbuf_p;

	int rc = awectrl_get_cmdbuf(m_ctrl_ctx, -1, &cmdbuf_p);
	ASSERT_EQ(rc, AWECTRL_RC_OK);

	rc = awectrl_get_cmdbuf(m_ctrl_ctx, 0, NULL);
	ASSERT_EQ(rc, AWECTRL_RC_FAIL_PARAM);

	rc = awectrl_get_cmdbuf(NULL, 0, NULL);
	ASSERT_EQ(rc, AWECTRL_RC_FAIL_PARAM);
}


/**
```yaml
- id: itest~AWEMGR.ControlComm.SendReceive~1
  covers: 
    - dsn~AWEMGR.ControlComm.ReadData~1
    - dsn~AWEMGR.ControlComm.WriteData~1
  description: |
    Transmits a get-target-info message to AWE Core/Server and retrieves the response.
```
*/
TEST_F(AweCtrlTestFixture, SendReceive) 
{
	unsigned int response_buffer[14]; // to provide just enough for the target info
	unsigned int request_buffer[2] = {0x00020029, 0x00020029}; // corresponds to get target info : PFID_GetTargetInfo

	ASSERT_EQ(awectrl_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 14, 0), 0);

	ASSERT_EQ(response_buffer[0], 0xE0000);  // 14 words <<16bit
	ASSERT_EQ(response_buffer[2], 0x473b8000);
}
