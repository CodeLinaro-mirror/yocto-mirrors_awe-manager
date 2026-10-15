#include "test_fixtures.h"
#include "awemgr_logging.h"
#include <thread>

/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.Events.DisableEnableAllEvents~1
  covers: req~AWEMGR.Event_Notification~1
  description: Checks if the event modules in the test design can be disabled, then enabled again.
```
*/
TEST_F(AweMgrTestEvents, DisableEnableAllEvents)
{

	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "EventTrigger", &triggerMod), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Event1", &event1Mod), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event1Mod, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);

	EXPECT_EQ(awemgr_disable_event(m_ctx, "Event1"), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event1Mod, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_INACTIVE);

	EXPECT_EQ(awemgr_enable_event(m_ctx, "Event1"), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event1Mod, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);

	ASSERT_EQ(awemgr_get_event_count(m_ctx), 2);
	ASSERT_EQ(awemgr_get_event_count(NULL), -1);

	awemgr_module mod;
	ASSERT_EQ(awemgr_get_event_by_index(m_ctx, 4, &mod), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_get_event_by_index(NULL, 0, &mod), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_get_event_by_index(m_ctx, 0, &mod), awemgr_RC_OK);
	ASSERT_STREQ(mod.name, "Event1");
	ASSERT_EQ(mod.instanceId, 0);
	ASSERT_EQ(mod.objectId, 30003);
	ASSERT_EQ(mod.classId, 3203337638);

	ASSERT_EQ(awemgr_get_event_by_index(m_ctx, 1, &mod), awemgr_RC_OK);
	ASSERT_STREQ(mod.name, "Event2");
	ASSERT_EQ(mod.instanceId, 0);
	ASSERT_EQ(mod.objectId, 31111);
	ASSERT_EQ(mod.classId, 3203337638);
}

/**
```yaml
- id: itest~AWEMGR.Events.ReadEvents~2
  covers: req~AWEMGR.Event_Notification~1
  description: Manually triggers an event, and reads the event data.
```
*/
TEST_F(AweMgrTestEvents, ReadEvents)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEvents *self = static_cast<AweMgrTestEvents *>(userdata);
		ASSERT_EQ(ev->module.instanceId, 0);
		ASSERT_EQ(ev->module.objectId, self->event1Mod.objectId);
		ASSERT_EQ(ev->eventType, 10);
		ASSERT_EQ(ev->sizeInBytes, 20);
		self->cbCount++;
	};
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	// check flag is set
	ASSERT_TRUE(awemgr_events_started(m_ctx));

	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "EventTrigger", &triggerMod), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Event1", &event1Mod), awemgr_RC_OK);

	EXPECT_EQ(awemgr_enable_event(m_ctx, "Event1"), awemgr_RC_OK);

	ASSERT_EQ(awemgr_module_get_state(m_ctx, event1Mod, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);
	ASSERT_EQ(awemgr_module_get_state(m_ctx, event1Mod, &state), awemgr_RC_OK);
	ASSERT_EQ(state, MODULE_ACTIVE);

	ASSERT_EQ(getValue<uint32_t>("Event1.isCallbackRegistered"), 1);
	ASSERT_EQ(setValue<uint32_t>("Event1.triggerBehavior", 0), true); // Rising edge
	ASSERT_EQ(setValue<uint32_t>("Event1.triggerType", 0), true);	  // Instant

	delay_ms(10);

	ASSERT_EQ(getValue<uint32_t>("Event1.failedTriggerCnt"), 0);
	ASSERT_EQ(getValue<uint32_t>("Event1.triggerCnt"), 0);

	// Trigger by making the trigger pin high
	ASSERT_EQ(setValue<uint32_t>("EventTrigger.value", 1), true);
	delay_ms(10);

	ASSERT_EQ(getValue<uint32_t>("Event1.failedTriggerCnt"), 0);
	ASSERT_EQ(getValue<uint32_t>("Event1.triggerCnt"), 1);
	delay_ms(10);

	// Second trigger
	// High to Low
	ASSERT_EQ(setValue<uint32_t>("EventTrigger.value", 0), true);
	delay_ms(10);
	// Low to High
	ASSERT_EQ(setValue<uint32_t>("EventTrigger.value", 1), true);
	delay_ms(10);

	ASSERT_EQ(getValue<uint32_t>("Event1.failedTriggerCnt"), 0);
	ASSERT_EQ(getValue<uint32_t>("Event1.triggerCnt"), 2);
	delay_ms(10);

	ASSERT_EQ(awemgr_events_process_next(m_ctx, 100), awemgr_RC_OK);
	ASSERT_EQ(cbCount, 1);

	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.AsyncEvents~2
  covers: req~AWEMGR.Event_Notification~1
  description: |
    The event module (Event2), triggers an event whenever the RMS of a source signal is greater than the certain threshold.
    The test sets the gain of the signal, causing the RMS to exceed the threshold, thus triggering events Asynchronously.

    It additionally sets the trace state to "on" to check if the data of the event payload
    can be seen in the trace logs. No explicit validation is done on the trace logs,
    but it can be checked manually while the test is running. Look out for the log message
    "awecomm_tracer( 54): [chn:0] EVT:    0 : 0xc11a46d3, 0xc1200000"
```
*/
TEST_F(AweMgrTestEvents, AsyncEvents)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEvents *self = static_cast<AweMgrTestEvents *>(userdata);
		ASSERT_EQ(ev->module.instanceId, self->event2Mod.instanceId);
		ASSERT_EQ(ev->module.objectId, self->event2Mod.objectId);
		ASSERT_EQ(ev->eventType, self->getValue<uint32_t>("Event2.eventType"));
		ASSERT_EQ(ev->sizeInBytes, 8);
		self->cbCount++;
	};
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_module_by_name(m_ctx, "Event2", &event2Mod), awemgr_RC_OK);

	ASSERT_EQ(setValue<float>("Scaler1.gain", 15.f), true); // Changing the gain, would make the RMS > threshold, thus causing event triggers

	ASSERT_EQ(awemgr_config_set(cfg_p, "mgr.event.trace.state", "on"), AWECFG_RC_OK);
	ASSERT_EQ(awemgr_events_process_next(m_ctx, 100), awemgr_RC_OK);
	ASSERT_EQ(cbCount, 1);
	ASSERT_EQ(awemgr_config_set(cfg_p, "mgr.event.trace.state", "off"), AWECFG_RC_OK);

	ASSERT_TRUE(getValue<uint32_t>("Event2.triggerCnt") >= 1);
	delay_ms(50);
	ASSERT_TRUE(getValue<uint32_t>("Event2.triggerCnt") >= 10);

	EXPECT_EQ(awemgr_disable_event(m_ctx, "Event2"), awemgr_RC_OK);
	auto triggercount = getValue<uint32_t>("Event2.triggerCnt");
	delay_ms(10);
	EXPECT_EQ(triggercount, getValue<uint32_t>("Event2.triggerCnt")); // Trigger count remains same after the event is unsubscribed

	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.AsyncTestEventCallbacks~1
  covers: req~AWEMGR.Event_Notification~1
  description: |
    The test counts the number of event callbacks to ensure all the callbacks are recieved.
    This test is configured to run for 2 seconds to get significant number of event callbacks.
```
*/
TEST_F(AweMgrTestEvents, AsyncTestEventCallbacks)
{
	GTEST_SKIP(); // The test is skipped due to frequent failures on Jenkins
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);

	ASSERT_TRUE(m_ctx != NULL);
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEvents *self = static_cast<AweMgrTestEvents *>(userdata);
		self->cbCount++;
	};

	keep_running.store(true);
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	std::thread eventsPollingThread([this]()
									{
        while (keep_running.load()) {
			ASSERT_EQ(awemgr_events_process_next(m_ctx, 0), awemgr_RC_OK) << "Error reading event" << cbCount;
        } });

	ASSERT_EQ(setValue<float>("Scaler1.gain", 20.f), true);
	std::this_thread::sleep_for(std::chrono::seconds(2));
	keep_running.store(false);
	ASSERT_EQ(setValue<float>("Scaler1.gain", 0.f), true);
	if (eventsPollingThread.joinable())
	{
		eventsPollingThread.join();
	}
	// Following GTest Macro will check if the difference between cbCount and "Event2.triggerCnt" is +- 2;
	EXPECT_NEAR(cbCount, getValue<uint32_t>("Event2.triggerCnt"), 2);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.AddRemoveListener~1
  covers: req~AWEMGR.Event_Notification~1
  description: Test API's specific to add/remove/replace category listener callbacks.
```
*/
TEST_F(AweMgrTestEventCategories, cbtest)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);

	ASSERT_TRUE(m_ctx != NULL);
	auto category0Listener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventCategories *self = static_cast<AweMgrTestEventCategories *>(userdata);
		self->category0EventCount++;
	};

	auto category1Listener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventCategories *self = static_cast<AweMgrTestEventCategories *>(userdata);
		self->category1EventCount++;
	};

	auto eventListener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventCategories *self = static_cast<AweMgrTestEventCategories *>(userdata);
		self->eventListenerCount++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), false); // Failed as No callback registered

	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 0, category0Listener, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(category0EventCount, 1);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(category0EventCount, 2);

	// Callback already registered, API replaces the listener
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 0, eventListener, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(eventListenerCount, 1);
	ASSERT_EQ(category0EventCount, 2);

	ASSERT_EQ(awemgr_events_remove_category_listener(m_ctx, 0), awemgr_RC_OK);
	// Callback already removed, API returns no error
	ASSERT_EQ(awemgr_events_remove_category_listener(m_ctx, 0), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), false);
	ASSERT_EQ(eventListenerCount, 1);
	ASSERT_EQ(category0EventCount, 2);

	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 1, category1Listener, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(1), true);
	ASSERT_EQ(category1EventCount, 1);

	ASSERT_EQ(MockEventCategoryTrigger(1), true);
	ASSERT_EQ(category1EventCount, 2);

	ASSERT_EQ(awemgr_events_remove_category_listener(m_ctx, 1), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(1), false);

	ASSERT_EQ(MockEventCategoryTrigger(1), false);
	ASSERT_EQ(category1EventCount, 2);

	// error checks
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 32, eventListener, this), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 0xffffffff, eventListener, this), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_add_category_listener(NULL, 32, eventListener, this), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 31, NULL, this), awemgr_RC_ERR);
}

/**
```yaml
- id: itest~AWEMGR.Events.Errors~2
  covers: req~AWEMGR.SoftwareComponent~1
  description: Checks if the Errors in Events API's are handled correctly
```
*/
TEST_F(AweMgrTestEvents, Errors)
{
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
	};
	ScopedStdoutCapture capture;

	ASSERT_EQ(awemgr_events_start(NULL, eventlistener, this), awemgr_RC_ERR);

	// Set Invalid Port, so events start fails
	aweconfig_set(cfg_p, "mgr.event.socket.port", "13123");
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	// Restore the Events port
	aweconfig_set(cfg_p, "mgr.event.socket.port", "15010");

	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_ERR); // Event already started

	ASSERT_FALSE(awemgr_events_started(NULL));

	ASSERT_EQ(awemgr_events_add_category_listener(NULL, 0, eventlistener, this), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 324324, eventlistener, this), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 0, NULL, this), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_remove_category_listener(NULL, 0), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_remove_category_listener(m_ctx, 34320), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_process_next(NULL, 0), awemgr_RC_ERR);

	EXPECT_EQ(awemgr_enable_event(NULL, "Event1"), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_enable_event(m_ctx, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_enable_event(m_ctx, "InvalidName"), awemgr_RC_ERR);

	EXPECT_EQ(awemgr_disable_event(NULL, "Event1"), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_disable_event(m_ctx, NULL), awemgr_RC_ERR);
	EXPECT_EQ(awemgr_disable_event(m_ctx, "InvalidName"), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
	set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_DEBUG);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK); // Event already stopped is no error
	set_loglevel(AWEMGR_LOG_API, AWEMGR_LOG_LEVEL_INFO);

	ASSERT_EQ(awemgr_events_stop(NULL), awemgr_RC_ERR);

	// additional checks on the output
	std::string output = capture.GetOutput();
	EXPECT_TRUE(output.find("Events already started") != std::string::npos);
	EXPECT_TRUE(output.find("Events already stopped") != std::string::npos);
}

/**
```yaml
- id: itest~AWEMGR.Events.TimeOut~1
  covers: req~AWEMGR.Event_Notification~1
  description: Check that checking events with various timeout values works.
```
*/
TEST_F(AweMgrTestEvents, TimeOut)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEvents *self = static_cast<AweMgrTestEvents *>(userdata);
		self->cbCount++;
	};
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	auto start = std::chrono::high_resolution_clock::now();

	// read any pending event from the target, which can cause the next timeout test to fail.
	awemgr_events_process_next(m_ctx, 20);
	awemgr_events_process_next(m_ctx, 20);
	cbCount = 0;


	ASSERT_EQ(awemgr_events_process_next(m_ctx, 1000), awemgr_RC_COMM_TIMEOUT);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;

	ASSERT_TRUE(elapsed.count() >= 1.0);
	ASSERT_EQ(cbCount, 0);

	start = std::chrono::high_resolution_clock::now();
	ASSERT_EQ(awemgr_events_process_next(m_ctx, 2000), awemgr_RC_COMM_TIMEOUT);
	end = std::chrono::high_resolution_clock::now();
	elapsed = end - start;

	ASSERT_TRUE(elapsed.count() >= 2.0);
	ASSERT_EQ(cbCount, 0);

	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.ErrorProtocol_IncorrectMagicW~1
  covers:
    - req~AWEMGR.Event_Notification~1
    - req~AWEMGR.CommFailErrorCode~1
  description: Reading from a socket, we do not get the right magic word.
```
*/
TEST_F(AweMgrTestEvents, ErrorProtocol_IncorrectMagicW)
{
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
	};
	ScopedStdoutCapture capture;

	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.event.socket.port", "15011"), AWECFG_RC_OK);
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	ASSERT_EQ(awemgr_events_process_next(m_ctx, 2000), awemgr_RC_ERR);

	std::string output = capture.GetOutput();
	EXPECT_TRUE(output.find("Event communication error: Incorrect magic word received") != std::string::npos);
}

/**
```yaml
- id: itest~AWEMGR.Events.ErrorProtocol_IncorrectEventHdr~1
  covers:
    - req~AWEMGR.Event_Notification~1
    - req~AWEMGR.CommFailErrorCode~1
  description: When reading from socket, there is no suitable/enough data for an event header.
```
*/
TEST_F(AweMgrTestEvents, ErrorProtocol_IncorrectEventHdr)
{
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
	};
	ScopedStdoutCapture capture;

	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.event.socket.port", "15012"), AWECFG_RC_OK);
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	ASSERT_EQ(awemgr_events_process_next(m_ctx, 2000), awemgr_RC_ERR);

	std::string output = capture.GetOutput();
	EXPECT_TRUE(output.find("Event communication error: no eventHeader.") != std::string::npos);
}

/**
```yaml
- id: itest~AWEMGR.Events.ErrorProtocol_IncorrectPayload~1
  covers:
    - req~AWEMGR.Event_Notification~1
    - req~AWEMGR.CommFailErrorCode~1
  description: When reading from socket, there is just enough data for the header, but not for the payload.
```
*/
TEST_F(AweMgrTestEvents, ErrorProtocol_IncorrectPayload)
{
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
	};
	ScopedStdoutCapture capture;
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.event.socket.port", "15013"), AWECFG_RC_OK);
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	ASSERT_EQ(awemgr_events_process_next(m_ctx, 2000), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);

	std::string output = capture.GetOutput();
	EXPECT_TRUE(output.find("Event communication error: payload read failed.") != std::string::npos);
}

/**
```yaml
- id: itest~AWEMGR.Events.ErrorProtocol_PayloadResize~1
  covers:
    - req~AWEMGR.Event_Notification~1
    - req~AWEMGR.CommFailErrorCode~1
  description: When reading from socket, we receive a bigger payload size than expected
               and we need to resize the internal buffer.
```
*/
TEST_F(AweMgrTestEvents, ErrorProtocol_PayloadResize)
{
	auto eventlistener = [](const awemgr_event *ev, void *userdata)
	{
	};
	ScopedStdoutCapture capture;

	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.event.socket.port", "15014"), AWECFG_RC_OK);
	ASSERT_EQ(awemgr_events_start(m_ctx, eventlistener, this), awemgr_RC_OK);

	ASSERT_EQ(awemgr_events_process_next(m_ctx, 2000), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);

	std::string output = capture.GetOutput();
	EXPECT_TRUE(output.find("Payload size(0x1000 Bytes) greater than local buffer size(0x3e8 Bytes). Resizing local buffer.") != std::string::npos);
	EXPECT_TRUE(output.find("Event communication error: payload read failed.") != std::string::npos);
}

/* ****************************************************************************
 * INSPECTOR TESTS
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.FiresAlongsideDefaultListener~1
  covers: req~AWEMGR.Event_Notification~1
  description: Inspector callback is called after the primary default listener for the same event.
               Both must receive the event; neither replaces the other.
```
*/
TEST_F(AweMgrTestEventInspectors, FiresAlongsideDefaultListener)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto defaultListener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->eventListenerCount++;
	};
	auto inspector = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector0Count++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, defaultListener, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(eventListenerCount, 1);
	ASSERT_EQ(inspector0Count, 1);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(eventListenerCount, 2);
	ASSERT_EQ(inspector0Count, 2);

	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.FiresWithNoDefaultListener~1
  covers: req~AWEMGR.Event_Notification~1
  description: Inspector alone is sufficient for an event to be processed (no default listener needed).
               Previously the dispatcher returned ERR when no listener was set; now the inspector satisfies that role.
```
*/
TEST_F(AweMgrTestEventInspectors, FiresWithNoDefaultListener)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto inspector = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector0Count++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);

	// No listener, no inspector: must fail
	ASSERT_EQ(MockEventCategoryTrigger(0), false);

	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector, this), awemgr_RC_OK);

	// Inspector alone: must succeed
	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(inspector0Count, 1);

	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.FiresAlongsideCategoryListener~1
  covers: req~AWEMGR.Event_Notification~1
  description: Inspector fires after a category-specific listener, not instead of it.
               The category listener must not be displaced by the inspector.
```
*/
TEST_F(AweMgrTestEventInspectors, FiresAlongsideCategoryListener)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto categoryListener = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->category0EventCount++;
	};
	auto inspector = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector0Count++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_add_category_listener(m_ctx, 0, categoryListener, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(category0EventCount, 1);
	ASSERT_EQ(inspector0Count, 1);

	// Inspector also fires for a category with no specific listener (NULL default)
	ASSERT_EQ(MockEventCategoryTrigger(5), true);
	ASSERT_EQ(category0EventCount, 1); // unchanged
	ASSERT_EQ(inspector0Count, 2);

	ASSERT_EQ(awemgr_events_remove_category_listener(m_ctx, 0), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.MultipleInspectors~1
  covers: req~AWEMGR.Event_Notification~1
  description: Two independent inspectors (e.g. shell and tuning socket) both receive every event.
```
*/
TEST_F(AweMgrTestEventInspectors, MultipleInspectors)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto inspector0 = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector0Count++;
	};
	auto inspector1 = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector1Count++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector0, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector1, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(inspector0Count, 1);
	ASSERT_EQ(inspector1Count, 1);

	ASSERT_EQ(MockEventCategoryTrigger(7), true);
	ASSERT_EQ(inspector0Count, 2);
	ASSERT_EQ(inspector1Count, 2);

	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector0), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector1), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.ClearByCallback~1
  covers: req~AWEMGR.Event_Notification~1
  description: Clearing one inspector by its callback pointer leaves other inspectors intact.
```
*/
TEST_F(AweMgrTestEventInspectors, ClearByCallback)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto inspector0 = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector0Count++;
	};
	auto inspector1 = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector1Count++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector0, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector1, this), awemgr_RC_OK);

	// Remove inspector0 only
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector0), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(inspector0Count, 0); // was cleared
	ASSERT_EQ(inspector1Count, 1); // still active

	// Clearing an already-cleared callback returns ERR
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector0), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector1), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.SetIdempotent~1
  covers: req~AWEMGR.Event_Notification~1
  description: Re-registering the same callback updates userdata in-place rather than consuming a new slot.
```
*/
TEST_F(AweMgrTestEventInspectors, SetIdempotent)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto inspector0 = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector0Count++;
	};
	auto inspector1 = [](const awemgr_event *ev, void *userdata)
	{
		AweMgrTestEventInspectors *self = static_cast<AweMgrTestEventInspectors *>(userdata);
		self->inspector1Count++;
	};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);

	// Register inspector0 twice; should occupy only one slot, leaving room for inspector1
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector0, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector0, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, inspector1, this), awemgr_RC_OK);

	ASSERT_EQ(MockEventCategoryTrigger(0), true);
	ASSERT_EQ(inspector0Count, 1);
	ASSERT_EQ(inspector1Count, 1);

	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector0), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, inspector1), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.SlotsFull~1
  covers: req~AWEMGR.Event_Notification~1
  description: Registering more than MAX_SUPPORTED_GENERIC_LISTENERS distinct callbacks returns an error.
```
*/
TEST_F(AweMgrTestEventInspectors, SlotsFull)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto cb0 = [](const awemgr_event*, void*){};
	auto cb1 = [](const awemgr_event*, void*){};
	auto cb2 = [](const awemgr_event*, void*){};
	auto cb3 = [](const awemgr_event*, void*){};
	auto cb4 = [](const awemgr_event*, void*){};

	ASSERT_EQ(awemgr_events_start(m_ctx, NULL, this), awemgr_RC_OK);

	ASSERT_EQ(awemgr_events_set_listener(m_ctx, cb0, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, cb1, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, cb2, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, cb3, this), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, cb4, this), awemgr_RC_ERR); // slots full

	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, cb0), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, cb1), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, cb2), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, cb3), awemgr_RC_OK);
	ASSERT_EQ(awemgr_events_stop(m_ctx), awemgr_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.Events.Inspector.Errors~1
  covers: req~AWEMGR.Event_Notification~1
  description: Null-pointer guard checks for set_inspector and clear_inspector.
```
*/
TEST_F(AweMgrTestEventInspectors, Errors)
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	auto inspector = [](const awemgr_event*, void*){};

	ASSERT_EQ(awemgr_events_set_listener(NULL, inspector, this), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_set_listener(m_ctx, NULL,     this), awemgr_RC_ERR);

	ASSERT_EQ(awemgr_events_clear_inspector(NULL,  inspector), awemgr_RC_ERR);
	ASSERT_EQ(awemgr_events_clear_inspector(m_ctx, NULL),      awemgr_RC_ERR);
}
