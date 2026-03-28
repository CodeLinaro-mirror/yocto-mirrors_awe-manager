#include <gtest/gtest.h>
#include "awe_event.h" 
#include "awe_ctrl.h"

/* ****************************************************************************
 * FIXTURES
 * ***************************************************************************/
int listenerCb(const aweevent_hdr* hdr, const char* payload, const void* userdata)
{
	EXPECT_TRUE(hdr != NULL);
	return 0;
}

class AweCtrlEventAbstractionTestFixture: public testing::Test
{
	public:
		void SetUp() override
		{
			ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
			ASSERT_EQ(aweevent_register_configs(cfg_p), AWECFG_RC_OK);
			ASSERT_TRUE(create_evt_backend(NULL, NULL, NULL) == NULL);
			pBkend = create_evt_backend(cfg_p, listenerCb, NULL);
			ASSERT_TRUE(pBkend != NULL);
		}

		void TearDown() override
		{
			ASSERT_EQ(pBkend->exit(NULL), AWECTRL_RC_OK);
			aweevent_data* evt_p = NULL;
			ASSERT_EQ(aweevent_exit(&evt_p), AWECTRL_RC_OK);
			ASSERT_EQ(pBkend->exit(pBkend), AWECTRL_RC_OK);	
			free(evt_p);
			ASSERT_TRUE(aweconfig_destroy(&cfg_p) != -1);
		}
	protected:
		awe_evt_backend *pBkend;
		awe_config *cfg_p;
};

/**
```yaml
- id: itest~AWEMGR.EventsAbstraction.Api~1
  covers: dsn~AWEMGR.ControlComm.Abstraction~1
  description: Tests the event abstraction interface API's
```
*/
TEST_F(AweCtrlEventAbstractionTestFixture, API) {
	ASSERT_EQ(pBkend->init(NULL), AWE_EVT_RC_INVALID_ARG);
	ASSERT_EQ(pBkend->read_event(NULL, 0), AWE_EVT_RC_INVALID_ARG);

	// take a backup of platform data, make it NULL, execute API and restore
	auto bkup = pBkend->platform_data;
	pBkend->platform_data = NULL;
	ASSERT_EQ(pBkend->init(pBkend), AWE_EVT_RC_INVALID_ARG);
	pBkend->platform_data = bkup;

	aweconfig_set(cfg_p, "mgr.event.socket.ip", "");
	aweconfig_set(cfg_p, "mgr.event.socket.port", "");
	ASSERT_EQ(pBkend->init(pBkend), AWE_EVT_RC_OK);

	aweconfig_set(cfg_p, "mgr.event.socket.ip", "127.0.0.1");
	aweconfig_set(cfg_p, "mgr.event.socket.port", "15010");

	ASSERT_EQ(pBkend->read_event(NULL, 0), AWE_EVT_RC_INVALID_ARG);
	// take a backup of platform data, make it NULL, execute API and restore
	bkup = pBkend->platform_data;
	pBkend->platform_data = NULL;
	ASSERT_EQ(pBkend->read_event(pBkend, 0), AWE_EVT_RC_INVALID_ARG);
	pBkend->platform_data = bkup;
}

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/
