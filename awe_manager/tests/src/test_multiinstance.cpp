#include "test_fixtures.h"
#include "awosal_socket.h"


/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/


/**
```yaml
- id: itest~AWEMGR.Multiinstance.subscribe_unsubscribe~1
  covers: 
    - req~AWEMGR.Event_Notification~1
    - req~AWEMGR.Named_Access~1
  description: |
    This test subscribes/unsubscribe the events running on 2 different Instances. 
    Event1 is running on InstanceId 0 and Event2 is running on InstanceID 1.
```
*/
TEST_F(AweMgrTestMultiInstance, subscribe_unsubscribe_different_instances) {

	enum awemgr_module_runtimestate state;
	struct awemgr_module trigger, event1, event2;

	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "EventTrigger", &trigger), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Event1", &event1), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Event2", &event2), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event2, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);

	EXPECT_EQ(awemgr_disable_event(m_ctx, "Event2"), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event2, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_INACTIVE);

	EXPECT_EQ(awemgr_enable_event(m_ctx, "Event2"), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event2, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);
}

/**
```yaml
- id: itest~AWEMGR.Multiinstance.set_get_events~1
  covers:
    - req~AWEMGR.Event_Notification~1
    - req~AWEMGR.Named_Access~1
  description: |
    This test shows that events are propogated from second instance and the
    module parameters can be read and written to in a multiinstance design.
```
*/
TEST_F(AweMgrTestMultiInstance, event_second_instance) {
	enum awemgr_module_runtimestate state;
	struct awemgr_module triggerMod, event2Mod;

	ASSERT_EQ(awemgr_events_start(m_ctx, nullptr, nullptr), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "EventTrigger", &triggerMod), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Event2", &event2Mod), awemgr_RC_OK);

	EXPECT_EQ(awemgr_enable_event(m_ctx, "Event2"), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event2Mod, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);
	//Instance 1
	ASSERT_EQ(getValue<uint32_t>("Event2.isCallbackRegistered"), 1);
	ASSERT_EQ(setValue<uint32_t>("Event2.triggerBehavior", 0), true);  // Rising edge
	ASSERT_EQ(setValue<uint32_t>("Event2.triggerType", 0), true); // Instant

	ASSERT_EQ(getValue<uint32_t>("Event2.failedTriggerCnt"), 0);
	ASSERT_EQ(getValue<uint32_t>("Event2.triggerCnt"), 0);

	// Trigger by making the trigger pin high
	ASSERT_EQ(setValue<uint32_t>("EventTrigger.value", 1), true);
	delay_ms(10);

	ASSERT_EQ(getValue<uint32_t>("Event2.failedTriggerCnt"), 0);
	ASSERT_EQ(getValue<uint32_t>("Event2.triggerCnt"), 1);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}