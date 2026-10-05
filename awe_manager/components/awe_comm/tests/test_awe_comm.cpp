#include <gtest/gtest.h>

#include "awe_comm.h"
#include "awosal_socket.h"
#include "awosal_time.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <system_error>

#define TEST_MAX_AWE_COMM_CHANNELS  2 // ugly, but awe_ctrl_internal.h is not accessible from test code, and we want to avoid hardcoding this value in test code as well

/* ****************************************************************************
 * HELPER FUNCTIONS
 * ***************************************************************************/
namespace fs = std::filesystem;

static bool same_file_size(const fs::path& p1, const fs::path& p2) {
    std::error_code ec;

    std::uintmax_t size1 = fs::file_size(p1, ec);
    if (ec) return false;

    std::uintmax_t size2 = fs::file_size(p2, ec);
    if (ec) return false;

    return size1 == size2;
}

/* ****************************************************************************
 * FIXTURES
 * ***************************************************************************/

 // helper class to run a simple socket server in the background for testing the socket backend; it will echo back any received data until it receives "ECHO_STOP" or is stopped via stop() method
 class SocketServer {
private:
    int server_fd;
    int port;

	int wait; // time in seconds to wait before closing the connection

    int client_fd;
    bool running;

    std::thread server_thread;    // Thread to run the server loop

    public:
    // Constructor
    explicit SocketServer(int port, int wait) : port(port), wait(wait), running(false) {}

    void run_server() {
        running = true;
        client_fd = si_create_server(port, &server_fd, -1);
        ASSERT_GT(server_fd, 0);
        ASSERT_GT(client_fd, 0);

		std::this_thread::sleep_for(std::chrono::seconds(wait));

        ASSERT_EQ(si_close_connection(client_fd), 0);
    }
    void start()
    {
        if (!running) {
            server_thread = std::thread(&SocketServer::run_server, this);
        }
    }
    void stop()
    {
        si_close_server(server_fd);
        if (server_thread.joinable()) {
            server_thread.join();
        }
		running = false;
    }
};

// *******************************************************************

class AweCommTestFixture: public testing::Test
{
	public:
		void SetUp() override;
		void TearDown() override;

	protected:
		struct awecomm_data *m_ctrl_ctx;
		awe_config *cfg_p;
};

void AweCommTestFixture::SetUp()
{
	const char* awb_file = NULL;
	//TODO: use c++ 17 filesystem
#ifdef WIN32
	// Windows: Use GetTempPath or a hardcoded path
	awb_file = "C:\\temp\\comm-dump.awb";
#else
	// Unix-like: Use /tmp directory
	awb_file = "/tmp/comm-dump.awb";
#endif

	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awecomm_register_configs(cfg_p), AWECFG_RC_OK);

	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.state", "on"), AWECFG_RC_OK);
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.file", awb_file), AWECFG_RC_OK);

	int rc = awecomm_init(cfg_p, NULL, NULL, &m_ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
}

void AweCommTestFixture::TearDown()
{
	int rc = awecomm_exit(&m_ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
	ASSERT_TRUE(m_ctrl_ctx == NULL);

	ASSERT_TRUE(aweconfig_destroy(&cfg_p) != -1);
}

// *******************************************************************

class AweCommTimeOutSocketFixture : public AweCommTestFixture
{
	public:
		void SetUp() override;
		void TearDown() override;
		void StartSilentSocket(const char* port, int wait);
		void InitComm(const char* port, const char* timeout);

		void MeasureTimeOut(double timeout, double tolerance);

	private:
		SocketServer* m_server = nullptr;

};



void AweCommTimeOutSocketFixture::InitComm(const char* port, const char* timeout)
{
	aweconfig_set(cfg_p, "mgr.comm.timeoutms", timeout);
	aweconfig_set(cfg_p, "mgr.comm.socket.port", port);
	int rc = awecomm_init(cfg_p, NULL, NULL, &m_ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
}

void AweCommTimeOutSocketFixture::StartSilentSocket(const char* port, int wait)
{
	// ctx_ = new ServerClient_Combo;
	m_server = new SocketServer(atoi(port), wait);
	m_server->start();
	aweosal_mssleep(100);
}

void AweCommTimeOutSocketFixture::SetUp()
{
	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awecomm_register_configs(cfg_p), AWECFG_RC_OK);

	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.state", "on"), AWECFG_RC_OK);
	// ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.file", awb_file), AWECFG_RC_OK);

}

void AweCommTimeOutSocketFixture::TearDown()
{
	if (m_server)
    {
        m_server->stop();
        delete m_server;
    }
	m_server = nullptr;

	AweCommTestFixture::TearDown();
}

void AweCommTimeOutSocketFixture::MeasureTimeOut(double timeout, double tolerance)
{
	// use some static buffers; re-use the get_target_info request,
	// not that we will not get any response, but just want to trigger the timeout behavior in the comm component
	unsigned int response_buffer[14];
	unsigned int request_buffer[2] = {0x00020029, 0x00020029};

	auto start = std::chrono::high_resolution_clock::now();

	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 14, 0), AWECOMM_RC_TIMEOUT);

	auto end = std::chrono::high_resolution_clock::now();
	double elapsed_seconds = std::chrono::duration<double>(end - start).count();
	ASSERT_NEAR(timeout, elapsed_seconds, tolerance);
}


/* ****************************************************************************
 * TEST CASES
 * ***************************************************************************/

/**
```yaml
- id: itest~AWEMGR.ControlComm.InitExit~2
  covers: dsn~AWEMGR.ControlComm.Initialize~1
  description: |
    Checks that component can be created and safely destructed.
    The test now also enables tracing of control communication.
```
*/
TEST(AweCommTests, InitExit) {

	struct awecomm_data *ctrl_ctx;

	awe_config *cfg_p;
	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awecomm_register_configs(cfg_p), AWECFG_RC_OK);
	aweconfig_set(cfg_p, "mgr.comm.trace.state", "on");

	int rc = awecomm_init(cfg_p, NULL, NULL, &ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
	ASSERT_TRUE(ctrl_ctx != NULL);

	struct awecmd_st *cmdbuf_p;
	for (int i=0; i<TEST_MAX_AWE_COMM_CHANNELS; i++)
	{
		ASSERT_EQ(awecomm_get_cmdbuf(ctrl_ctx, i, &cmdbuf_p), AWECOMM_RC_OK);
		ASSERT_EQ(cmdbuf_p->response_buffer_size, 264);

		ASSERT_EQ(awecomm_release_lock(ctrl_ctx), AWECOMM_RC_OK);
	}

	rc = awecomm_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
	ASSERT_TRUE(ctrl_ctx == NULL);

	// second exit will only dump message, but returns ok
	rc = awecomm_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);

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
TEST(AweCommTests, TuningBufferSize) {

	struct awecomm_data *ctrl_ctx;

	awe_config *cfg_p;
	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awecomm_register_configs(cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.buffersize", "4096"), AWECFG_RC_OK);
	aweconfig_set(cfg_p, "mgr.comm.trace.state", "on");

	int rc = awecomm_init(cfg_p, NULL, NULL, &ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
	ASSERT_TRUE(ctrl_ctx != NULL);

	struct awecmd_st *cmdbuf_p;
	for (int i=0; i<TEST_MAX_AWE_COMM_CHANNELS; i++)
	{
		ASSERT_EQ(awecomm_get_cmdbuf(ctrl_ctx, i, &cmdbuf_p), AWECOMM_RC_OK);
		ASSERT_EQ(cmdbuf_p->response_buffer_size, 4096);

		ASSERT_EQ(awecomm_release_lock(ctrl_ctx), AWECOMM_RC_OK);
	}

	rc = awecomm_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);
	ASSERT_TRUE(ctrl_ctx == NULL);

	// second exit will only dump message, but returns ok
	rc = awecomm_exit(&ctrl_ctx);
	ASSERT_EQ(rc, AWECOMM_RC_OK);

	ASSERT_EQ(aweconfig_destroy(&cfg_p), AWECFG_RC_OK);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.TuningBufferSizeUnusable~1
  covers: dsn~AWEMGR.ControlComm.TuningBufferSizeValidation~1
  description: |
    Checks that a tuning buffer size which is too small to hold a command, or not a
    number at all, is rejected and the backend default is used instead. A value below
    the minimum would underflow the payload size computation of the callers.
```
*/
TEST(AweCommTests, TuningBufferSizeUnusable) {

	// the value registered by awecomm_register_configs is the backend default,
	// so the expected fallback can be read from the config instead of hardcoding it
	awe_config *cfg_p;
	ASSERT_EQ(aweconfig_create(&cfg_p), AWECFG_RC_OK);
	ASSERT_EQ(awecomm_register_configs(cfg_p), AWECFG_RC_OK);

	uint32_t default_size = 0;
	ASSERT_EQ(aweconfig_get_as_uint(cfg_p, "mgr.comm.buffersize", &default_size), AWECFG_RC_OK);
	ASSERT_GT(default_size, 6u);

	// "6" leaves no room for payload, "abc" does not convert; both must fall back
	const char *unusable[] = { "0", "6", "abc" };

	for (const char *value : unusable)
	{
		ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.buffersize", value), AWECFG_RC_OK) << value;

		struct awecomm_data *ctrl_ctx;
		ASSERT_EQ(awecomm_init(cfg_p, NULL, NULL, &ctrl_ctx), AWECOMM_RC_OK) << value;
		ASSERT_TRUE(ctrl_ctx != NULL) << value;

		struct awecmd_st *cmdbuf_p;
		for (int i=0; i<TEST_MAX_AWE_COMM_CHANNELS; i++)
		{
			ASSERT_EQ(awecomm_get_cmdbuf(ctrl_ctx, i, &cmdbuf_p), AWECOMM_RC_OK) << value;
			ASSERT_EQ(cmdbuf_p->response_buffer_size, default_size) << value;
			ASSERT_EQ(AWECMD_REQUESTBUFFER_SZ(cmdbuf_p), default_size) << value;

			ASSERT_EQ(awecomm_release_lock(ctrl_ctx), AWECOMM_RC_OK) << value;
		}

		ASSERT_EQ(awecomm_exit(&ctrl_ctx), AWECOMM_RC_OK) << value;
	}

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
TEST(AweCommTests, InitExit_Fail) {

	int rc = awecomm_init(NULL, NULL, NULL, NULL);
	ASSERT_EQ(rc, AWECOMM_RC_FAIL_PARAM);

	rc = awecomm_exit(NULL);
	ASSERT_EQ(rc, AWECOMM_RC_FAIL_PARAM);

	ASSERT_EQ(awecomm_notify(NULL, 0), AWECOMM_RC_FAIL_PARAM);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.TransAct_Fail~1
  covers: dsn~AWEMGR.ControlComm.SelectChannel~1
  description: |
    Checks errors when trying to specify incorrect transact parameters.
```
*/
TEST_F(AweCommTestFixture, TransAct_Fail)
{
	ASSERT_EQ(awecomm_transact(m_ctrl_ctx, -1), AWECOMM_RC_FAIL_PARAM);
	ASSERT_EQ(awecomm_transact(NULL, -1), AWECOMM_RC_FAIL_PARAM);
	ASSERT_EQ(awecomm_transact(m_ctrl_ctx, TEST_MAX_AWE_COMM_CHANNELS), AWECOMM_RC_FAIL_PARAM);

	ASSERT_EQ(awecomm_transact_explicit(NULL, NULL, 0, NULL, 0, -1), AWECOMM_RC_FAIL_PARAM);
	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, NULL, 0, NULL, 0, -1), AWECOMM_RC_FAIL_PARAM);
	uint32_t data;
	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, NULL, 0, &data, 0, -1), AWECOMM_RC_FAIL_PARAM);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.GetBuf_Fail~1
  covers: dsn~AWEMGR.ControlComm.SelectChannel~1
  description: |
    Checks errors when trying to get a CMD buffer object.
```
*/
TEST_F(AweCommTestFixture, GetBuf_Fail) {
	struct awecmd_st *cmdbuf_p;

	int rc = awecomm_get_cmdbuf(m_ctrl_ctx, -1, &cmdbuf_p);
	ASSERT_EQ(rc, AWECOMM_RC_OK);

	rc = awecomm_get_cmdbuf(m_ctrl_ctx, 0, NULL);
	ASSERT_EQ(rc, AWECOMM_RC_FAIL_PARAM);

	rc = awecomm_get_cmdbuf(NULL, 0, NULL);
	ASSERT_EQ(rc, AWECOMM_RC_FAIL_PARAM);
}


/**
```yaml
- id: itest~AWEMGR.ControlComm.GetLockFail~1
  covers: dsn~AWEMGR.ControlComm.Protection~1
  description: |
    Checks errors when trying to get the locks. First if parameters are used correctly.
    Second if trying to get the lock twice without releasing it in between.
```
*/
TEST_F(AweCommTestFixture, GetLockFail) {

	// parameter checks
	ASSERT_EQ(awecomm_acquire_lock(NULL), AWECOMM_RC_FAIL_PARAM);
    ASSERT_EQ(awecomm_release_lock(NULL), AWECOMM_RC_FAIL_PARAM);

	/* Configure a short comm timeout so that awecomm_acquire_lock() (which now
	 * uses the comm timeout as its mutex acquisition timeout) times out
	 * quickly enough to keep this test fast.  The default comm timeout is
	 * 2000 ms; using 500 ms here preserves the original timing assertion. */
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.timeoutms", "500"), AWECFG_RC_OK);

	// double lock
	ASSERT_EQ(awecomm_acquire_lock(m_ctrl_ctx), AWECOMM_RC_OK);

	auto start = std::chrono::high_resolution_clock::now();

	std::thread secondLockerThread([this]()
	{
		ASSERT_EQ(awecomm_acquire_lock(m_ctrl_ctx), AWECOMM_RC_FAIL_RESOURCES);
	});
	secondLockerThread.join();

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed_seconds = std::chrono::duration<double>(end - start).count();
    ASSERT_NEAR(0.5, elapsed_seconds, 0.1);

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
TEST_F(AweCommTestFixture, SendReceive)
{
	unsigned int response_buffer[14]; // to provide just enough for the target info
	unsigned int request_buffer[2] = {0x00020029, 0x00020029}; // corresponds to get target info : PFID_GetTargetInfo

	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 14, 0), 0);

	ASSERT_EQ(response_buffer[0], 0xE0000);  // 14 words <<16bit
	ASSERT_EQ(response_buffer[2], 0x473b8000);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.TraceInfoBigResponseBuffer~1
  covers:
    - dsn~AWEMGR.ControlComm.Tracing~1
  description: |
    Checks that
```
*/
TEST_F(AweCommTestFixture, TraceInfoBigResponseBuffer)
{

	unsigned int response_buffer[1024]; // huge buffer, but tracing should only return 14 words
	unsigned int request_buffer[2] = {0x0002007f, 0x0002007f}; // get nr cores

	::testing::internal::CaptureStdout();
	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 1024, 0), 0);
	std::string output = ::testing::internal::GetCapturedStdout();
	EXPECT_EQ(output, "[DUMP]            trace_callback( 58): [chn:0] TX:    0 : 0x0002007f, 0x0002007f\n"
		"[DUMP]            trace_callback( 58): [chn:0] RX:    0 : 0x00070000, 0x00000004, 0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00070004\n");
}


/**
```yaml
- id: itest~AWEMGR.ControlComm.TraceSplitFiles~1
  covers:
    - dsn~AWEMGR.ControlComm.TracingSplitFiles~1
  description: |
    Checks that the configured trace file name is treated as a basename and that
    TX and RX traffic is dumped into separate '<basename>.tx' and '<basename>.rx' files.
```
*/
TEST_F(AweCommTestFixture, TraceSplitFiles)
{
	// SetUp() configured mgr.comm.trace.file with the basename below
#ifdef WIN32
	const char* tx_path = "C:\\temp\\comm-dump.awb.tx";
	const char* rx_path = "C:\\temp\\comm-dump.awb.rx";
#else
	const char* tx_path = "/tmp/comm-dump.awb.tx";
	const char* rx_path = "/tmp/comm-dump.awb.rx";
#endif

	unsigned int response_buffer[1024];
	unsigned int request_buffer[2] = {0x0002007f, 0x0002007f}; // get nr cores (2 words, 1 word without CRC)

	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 1024, 0), 0);

	// disabling file tracing closes (and thereby flushes) the TX and RX dump files
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.file", "~"), AWECFG_RC_OK);

	// TX dump holds the request without CRC: exactly 1 word
	FILE* tx = fopen(tx_path, "rb");
	ASSERT_NE(tx, nullptr);
	unsigned int tx_words[8] = {0};
	size_t tx_read = fread(tx_words, sizeof(unsigned int), 8, tx);
	fclose(tx);
	EXPECT_EQ(tx_read, 1u);
	EXPECT_EQ(tx_words[0], 0x0002007fu);

	// RX dump holds the response without CRC: 6 words (7th word is the CRC and is excluded)
	FILE* rx = fopen(rx_path, "rb");
	ASSERT_NE(rx, nullptr);
	unsigned int rx_words[8] = {0};
	size_t rx_read = fread(rx_words, sizeof(unsigned int), 8, rx);
	fclose(rx);
	EXPECT_EQ(rx_read, 6u);
	EXPECT_EQ(rx_words[0], 0x00070000u);
	EXPECT_EQ(rx_words[5], 0x00000003u);
}

/**
```yaml
- id: itest~AWEMGR.ControlComm.TraceSplitFilesChange~1
  covers:
    - dsn~AWEMGR.ControlComm.TracingSplitFiles~1
  description: |
    Checks that a configured trace file set up can be changed at runtime and
    that the new trace file name creates a new set of TX and RX dump files,
    while the old ones are closed.
```
*/
TEST_F(AweCommTestFixture, TraceSplitFilesChange)
{
	// SetUp() configured mgr.comm.trace.file with the basename below,
	// defining a second set of TX and RX dump files to be created when the config is changed
#ifdef WIN32
	const char* tx_path = "C:\\temp\\comm-dump.awb.tx";
	const char* rx_path = "C:\\temp\\comm-dump.awb.rx";
	const char* awb_file_changed = "C:\\temp\\comm-dump-changed.awb";
	const char* tx_path_changed = "C:\\temp\\comm-dump-changed.awb.tx";
	const char* rx_path_changed = "C:\\temp\\comm-dump-changed.awb.rx";
#else
	const char* tx_path = "/tmp/comm-dump.awb.tx";
	const char* rx_path = "/tmp/comm-dump.awb.rx";
	const char* awb_file_changed = "/tmp/comm-dump-changed.awb";
	const char* tx_path_changed = "/tmp/comm-dump-changed.awb.tx";
	const char* rx_path_changed = "/tmp/comm-dump-changed.awb.rx";
#endif

	unsigned int response_buffer[1024];
	unsigned int request_buffer[2] = {0x0002007f, 0x0002007f}; // get nr cores (2 words, 1 word without CRC)

	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 1024, 0), 0);

	// change file tracing base name: this should result in closing the old TX and RX dump files and
	// opening new ones with the new base name
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.file", awb_file_changed), AWECFG_RC_OK);

	// send command again
	ASSERT_EQ(awecomm_transact_explicit(m_ctrl_ctx, request_buffer, 2, response_buffer, 1024, 0), 0);

	// disabling file tracing closes (and thereby flushes) the new TX and RX dump files
	ASSERT_EQ(aweconfig_set(cfg_p, "mgr.comm.trace.file", "~"), AWECFG_RC_OK);

	// compare files, check size is the same should be enough in this test case.
	ASSERT_TRUE(same_file_size(tx_path, tx_path_changed));
	ASSERT_TRUE(same_file_size(rx_path, rx_path_changed));
}


/**
```yaml
- id: itest~AWEMGR.ControlComm.TimeOutSettings~1
  covers: dsn~AWEMGR.ControlComm.TimeOut~1
  description: |
    Checks various time out settings on a non-responsive socket port.
```
*/
TEST_F(AweCommTimeOutSocketFixture, TimeOutSettings) {

	// start an "empty socket server" that will accept the connection, but not send any data;
	// it will close the connection after 5 seconds
	StartSilentSocket("9000", 5);

	// start communication with a timeout of 2 seconds
	InitComm("9000", "1000");

	// check that transact returns with timeout after around 1 second
	MeasureTimeOut(1.0, 0.5);

	// change the timeout value to 2 seconds, and check that transact now returns with timeout after around 2 seconds
	aweconfig_set(cfg_p, "mgr.comm.timeoutms", "2000");
	MeasureTimeOut(2.0, 0.5);
}

