#ifndef _TEST_FIXTURES_
#define _TEST_FIXTURES_

#include <gtest/gtest.h>
#include "awe_manager.h"
#include "awe_event_backend.h"
#include "awosal_socket.h"
#include <thread>
#include <chrono>
#ifdef WIN32
#include <windows.h>
#define usleep(x) Sleep(x / 1000)
#endif

// helper routine
::testing::AssertionResult ArraysEqual(const unsigned int *expected, const unsigned int *actual, int size);

class ScopedStdoutCapture  {
	public:
		ScopedStdoutCapture () { ::testing::internal::CaptureStdout(); }
		~ScopedStdoutCapture () {
			if (!captured) {
				GetOutput();
			}
		}

		std::string GetOutput() {
			if (!captured) {
				captured = true;
				return ::testing::internal::GetCapturedStdout();
			}
			return "";
		}

	private:
		bool captured = false;
};

class AweMgrMinimalTestFixture : public testing::Test
{
public:
	void SetUp() override;
	void TearDown() override;
protected:
	awe_config* cfg_p;
};

class AweMgrTestFixture : public AweMgrMinimalTestFixture
{
public:
	void SetUp() override;
	void TearDown() override;
	template<typename T>
	bool setValue(const char* name, T value);
	template<typename T>
	T getValue(const char* name);
	bool writeArray(const char *name, void *data, uint32_t size, uint32_t offset = 0);
	bool readArray(const char *name, void *data, uint32_t size, uint32_t offset = 0);
public:
	struct awemgr_data *m_mgr_p = NULL;
	struct awemgr_ctx *m_ctx = NULL;
	void load_awc(const char *fname);
	void load_main_design();
	void delay_ms(int msec);
};

template <typename T>
bool AweMgrTestFixture::setValue(const char *name, T value)
{
	auto ret = awemgr_control_write(m_ctx, name, 0, (void *)&value, 1) == awemgr_RC_OK;
	return ret;
}
template <typename T>
T AweMgrTestFixture::getValue(const char *name)
{
	unsigned int nr_words_in_buf = 0;
	awemgr_vartype type;
	T value;
	awemgr_control_read(m_ctx, name, (void *)&value, 1, &nr_words_in_buf, &type);
	return value;
}

class AweMgrTestFixtureSetGetAWC : public AweMgrTestFixture
{
public:
	void SetUp() override;

public:
	static const char *m_awc_file;
};

class AweMgrTestFixtureSetGetAWCLoaded : public AweMgrTestFixtureSetGetAWC
{
public:
	void SetUp() override;
};

class AweMgrTestEvents : public AweMgrTestFixture
{
public:
	void SetUp() override;

public:
	enum awemgr_module_runtimestate state;
	struct awemgr_module triggerMod, scalerMod, event1Mod, event2Mod;
	uint32_t cbCount = 0;
	std::atomic<bool> keep_running;
};

extern "C"
{
	int mgrevent_dispatcher(const aweevent_header *pHdr, const char *pPayload, const void *pUsrData);
}

class AweMgrTestEventCategories : public AweMgrTestEvents
{
public:
	bool MockEventCategoryTrigger(unsigned category)
	{
		std::string payload = "category" + std::to_string(category);
		hdr.eventCategory = category;
		hdr.dataSize = sizeof(payload);
		return mgrevent_dispatcher(&hdr, payload.c_str(), m_ctx) == awemgr_RC_OK;
	}
	aweevent_header hdr;
	std::string payload;
	uint32_t category0EventCount = 0, category1EventCount = 0, eventListenerCount = 0;
};

class AweMgrTestEventInspectors : public AweMgrTestEventCategories
{
public:
    uint32_t inspector0Count = 0;
    uint32_t inspector1Count = 0;
};

class AweMgrTestMultiInstance: public AweMgrTestFixture
{
	public:
		void SetUp() override;
	public:
};

class AweMgrTestSleepResume : public AweMgrTestEvents
{
	// Derived class only for the Text Fixture name, no implementation difference
};

class AweMgrTestSuppressReturnValues : public AweMgrTestEvents
{
	// Derived class only for the Text Fixture name, no implementation difference
};

class AweMgrTestUserData: public AweMgrTestFixture
{
	public:
		void SetUp() override;
	public:
};


class SocketServer {
	private:
		int server_fd;
		int port;

		int client_fd;
		bool running;

		std::thread server_thread;

		public:
		explicit SocketServer(int port) : port(port), running(false) {}

		void run_server() {
			running = true;
			client_fd = si_create_server(port, &server_fd, -1);
			ASSERT_GT(server_fd, 0);
			ASSERT_GT(client_fd, 0);
			while (running)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
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
			running = false;
			si_close_server(server_fd);
			if (server_thread.joinable()) {
				server_thread.join();
			}
		}
	};

	class AweCOMMTimeoutFixture : public AweMgrTestFixture
	{
	public:
		void SetUp() override;
		void TearDown() override;
	protected:
		SocketServer* server;
		int clientFd;

	};

	class AweMgrTestFixtureMinimalWithErrors : public AweMgrTestFixture
	{
	public:
		void SetUp() override;

	public:
		static const char *m_awc_file;
	};

class PluginParserTest : public ::testing::Test {
protected:
    struct awemgr_design_info design_info;

    void SetUp() override {
        memset(&design_info, 0, sizeof(design_info));
    }

    // Helper to generate a long string for overflow tests
    std::string generate_long_string(size_t length, char fill) {
        return std::string(length, fill);
    }
};

#endif // _TEST_FIXTURES_
