#include "test_fixtures.h"
#include "awe_ctrl.h"
#include "awemgr_logging.h"

// A custom comparison function
::testing::AssertionResult ArraysEqual(const unsigned int* expected, const unsigned int* actual, int size) {
    for (int i = 0; i < size; ++i) {
        if (expected[i] != actual[i]) {
            return ::testing::AssertionFailure() << "Arrays differ at index " << i
                                                 << ": expected " << expected[i]
                                                 << " but got " << actual[i];
        }
    }
    return ::testing::AssertionSuccess();
}

// #define AWEMGR_GTEST_USE_PC 1

void AweMgrMinimalTestFixture::SetUp()
{
	ASSERT_EQ(awemgr_config_create(&cfg_p), awemgr_RC_OK);
	awectrl_set_traces(true);
}

void AweMgrMinimalTestFixture::TearDown()
{
	ASSERT_EQ(awemgr_config_destroy(&cfg_p), awemgr_RC_OK);
}

void AweMgrTestFixture::SetUp()
{
	AweMgrMinimalTestFixture::SetUp();
	int rc = awemgr_init(&cfg_p, &m_mgr_p);
	ASSERT_EQ(rc, awemgr_RC_OK);
}

void AweMgrTestFixture::TearDown()
{
	int rc = awemgr_exit(&m_mgr_p);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_TRUE(m_mgr_p==NULL);
	AweMgrMinimalTestFixture::TearDown();
}

void AweMgrTestFixture::load_awc(const char* fname)
{
	int rc = awemgr_load_awc(m_mgr_p, fname, 0);
	ASSERT_EQ(rc, awemgr_RC_OK);
	ASSERT_EQ(awemgr_get_loaded_awc_count(m_mgr_p), 1);
}

void AweMgrTestFixture::load_main_design()
{
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);

	ASSERT_EQ(awemgr_load_design(m_ctx, "Main"), awemgr_RC_OK);

	// wait a bit
	delay_ms(20);
}

void AweMgrTestFixture::delay_ms(int msec)
{
	usleep(1000*msec);
}

bool AweMgrTestFixture::writeArray(const char* name, void* data, uint32_t size, uint32_t offset){
	awemgr_vartype type = AWEMGR_VARTYPE_INTEGER;
	auto ret = (awemgr_control_write(m_ctx, name, offset, data, size, type) == awemgr_RC_OK);
	return ret;
}

bool AweMgrTestFixture::readArray(const char* name, void* data, uint32_t size, uint32_t offset){
	UINT32 nr_words_in_buf = 0;
	awemgr_vartype type = AWEMGR_VARTYPE_INTEGER;
	auto ret = (awemgr_control_read(m_ctx, name, data, size, &nr_words_in_buf, &type) == awemgr_RC_OK);
	return ret;
}

// todo: correct m_awc_file. Generate one with an AWB preset!

const char* AweMgrTestFixtureSetGetAWC::m_awc_file = TEST_DATA_DIR "/designs/set_get/target_files/awc_index.txt";

void AweMgrTestFixtureSetGetAWC::SetUp()
{
	AweMgrTestFixture::SetUp();
	load_awc(m_awc_file);
}

void AweMgrTestFixtureSetGetAWCLoaded::SetUp()
{
	AweMgrTestFixtureSetGetAWC::SetUp();
	load_main_design();
}

void AweMgrTestEvents::SetUp()
{
	AweMgrTestFixture::SetUp();
	load_awc(TEST_DATA_DIR "/designs/events/target_files/awc_index.txt");
	awectrl_set_traces(false);
	load_main_design();
	cbCount = 0;
}

void AweMgrTestMultiInstance::SetUp()
{
	AweMgrTestFixture::SetUp();
	load_awc(TEST_DATA_DIR "/designs/events_multiinstance/target_files/awc_index.txt");
	awectrl_set_traces(false);
	load_main_design();
}

void AweMgrTestUserData::SetUp()
{
	AweMgrTestFixture::SetUp();
	load_awc(TEST_DATA_DIR "/designs/minimal/target_files/awc_index.txt");
	awectrl_set_traces(false);
	load_main_design();
}

void AweCTRLTimeoutFixture::SetUp() {
	const char* port = "15006";
	server = new SocketServer(atoi(port));
	server->start();

	AweMgrMinimalTestFixture::SetUp();
	awemgr_config_set(cfg_p, "mgr.ctrl.socket.ip", "127.0.0.1");
	awemgr_config_set(cfg_p, "mgr.ctrl.socket.port", port);
	awemgr_config_set(cfg_p, "mgr.ctrl.socket.timeoutms", "500");
	int rc = awemgr_init(&cfg_p, &m_mgr_p);
	ASSERT_EQ(rc, awemgr_RC_OK);

	load_awc(TEST_DATA_DIR "/designs/minimal/target_files/awc_index.txt");
	m_ctx = awemgr_get_awc_context(m_mgr_p, 0);
	ASSERT_TRUE(m_ctx != NULL);
};

void AweCTRLTimeoutFixture::TearDown()
{
	server->stop();
	delete server;

	AweMgrTestFixture::TearDown();
}

void AweMgrTestFixtureMinimalWithErrors::SetUp()
{
	AweMgrTestFixture::SetUp();
	load_awc(TEST_DATA_DIR "/designs/minimal/target_files/awc_index_with_errors.txt");
	awectrl_set_traces(false);
	load_main_design();
}