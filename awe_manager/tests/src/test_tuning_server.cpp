/* MIT License
**
** Copyright (c) 2024 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

/**
 * @file test_tuning_server.cpp
 * @brief Tests for awemgr_tuning_server (serversocket addon).
 *
 * Port allocation (each test uses a distinct port to allow parallel execution):
 *   7301 – (reserved)
 *   7302 – ZeroLengthHeader
 *   7303 – OversizeHeader
 *   7304 – TruncatedBody
 *   7305 – GetTargetInfo round-trip
 *   7306 – Respawn
 *   7307 – CleanShutdown / signal test
 */

#include "test_fixtures.h"
#include "awemgr_tuning_server.h"
#include "awosal_socket.h"

#include <csignal>
#include <cstdint>
#include <future>
#include <string>
#include <vector>

#ifndef _WIN32
#include <pthread.h>
#include <signal.h>
using thread_id_t = pthread_t;
#else
using thread_id_t = uintptr_t;
#endif

/* ================================================================
 * Helpers
 * ================================================================ */

/* Build an AWE wire-format header word.
 * AWE wire format: bits[31:16] = total packet length in words. */
static uint32_t make_awe_header(uint16_t len_words, uint16_t opcode_low)
{
    return (static_cast<uint32_t>(len_words) << 16) | opcode_low;
}

/* Read exactly `n` bytes from `fd` using si_readbuf; returns true on success. */
static bool recv_exactly(int fd, void *buf, int n)
{
    int got = 0;
    while (got < n)
    {
        int r = si_readbuf(fd, static_cast<char *>(buf) + got, n - got, 0);
        if (r <= 0)
            return false;
        got += r;
    }
    return true;
}

/* ================================================================
 * Tests: create / destroy — no AWE Manager or network needed
 * ================================================================ */

/**
```yaml
- id: utest~AWEMGR.ADDON.TUNINGSERVER.NullMgrReturnNull~1
  covers: req~AWEMGR.Addon.TuningSocket~1
  description: Checks API behavior when given a null pointer for the manager handle.
```
*/
TEST(TuningServerLifecycle, Create_NullMgr_ReturnsNull)
{
    EXPECT_EQ(awemgr_tuning_server_create(nullptr, nullptr), nullptr);
}

/**
```yaml
- id: utest~AWEMGR.ADDON.TUNINGSERVER.DestroyNullNoCrash~1
  covers: req~AWEMGR.Addon.TuningSocket~1
  description: Ensures that destroying a null server pointer does not cause a crash or abort.
```
*/
TEST(TuningServerLifecycle, Destroy_Null_DoesNotCrash)
{
    /* free(NULL) is defined; verify there is no crash or abort. */
    EXPECT_NO_FATAL_FAILURE(awemgr_tuning_server_destroy(nullptr));
}

/**
```yaml
- id: utest~AWEMGR.ADDON.TUNINGSERVER.NullSrv_ReturnsMinusOne~1
  covers: req~AWEMGR.Addon.TuningSocket~1
  description: Ensures that running a null server pointer returns -1.
```
*/
TEST(TuningServerRun, NullSrv_ReturnsMinusOne)
{
    EXPECT_EQ(awemgr_tuning_server_run(nullptr, "7300"), -1);
}

/* ================================================================
 * Fixture: tests that require a live AWE Manager handle
 *
 * Design notes — avoiding hangs
 * --------------------------------
 * Two failure modes were identified and addressed here:
 *
 * 1. Race on si_listen: si_open_connection() returns ECONNREFUSED immediately
 *    when the server thread hasn't called si_listen() yet (no retry internally).
 *    connect_client() therefore retries in a loop until the port is open.
 *
 * 2. Deadlock on ASSERT failure: a local std::future<int> created by
 *    std::launch::async has a blocking destructor.  If an ASSERT fires before
 *    fut.get() is called, the future goes out of scope, its destructor blocks
 *    waiting for the server thread, and the server thread is blocked in
 *    si_accept() waiting for a connection — deadlock.  To avoid this, the
 *    future is kept as a fixture member (active_fut_) and TearDown connects
 *    a "drain" client to unblock si_accept() before destroying the AWE Manager
 *    handle that the server thread holds.
 * ================================================================ */

class TuningServerFixture : public AweMgrTestFixture
{
public:
    void SetUp() override
    {
        AweMgrTestFixture::SetUp();
        awemgr_tuning_server_register_configs(cfg_p);
        srv = awemgr_tuning_server_create(m_mgr_p, cfg_p);
        ASSERT_NE(srv, nullptr);
    }

    void TearDown() override
    {
        /* If a test exited early (ASSERT) the server thread may still be
         * blocked in si_accept().  Connect a drain client to unblock it so
         * the thread finishes before we destroy the AWE Manager handle it
         * holds.  If the future is already done, this block is skipped. */
        if (active_fut_.valid())
        {
            if (active_fut_.wait_for(std::chrono::milliseconds(0)) !=
                std::future_status::ready)
            {
                int drain = si_open_connection(
                    "127.0.0.1", std::to_string(active_port_).c_str(), 2);
                if (drain >= 0)
                    si_close_connection(drain);
            }
            active_fut_.wait(); /* join the thread */
        }

        awemgr_tuning_server_destroy(srv);
        srv = nullptr;
        AweMgrTestFixture::TearDown();
    }

    /* Start awemgr_tuning_server_run in a background thread on `port`.
     * The future is stored in active_fut_ so TearDown can drain it safely.
     * An optional promise can be provided to receive the server thread id
     * (used by the CleanShutdown test). */
    void start_server(int port,
                      std::promise<thread_id_t> *tid_promise_p = nullptr)
    {
        active_port_ = port;
        port_str = std::to_string(port);
        active_fut_ = std::async(std::launch::async,
            [this, tid_promise_p]() -> int
            {
#ifndef _WIN32
                if (tid_promise_p)
                    tid_promise_p->set_value(pthread_self());
#endif
                return awemgr_tuning_server_run(
                    srv, port_str.c_str());
            });
    }

    /* Retrieve the server's return value (blocks until the thread exits). */
    int wait_for_server()
    {
        return active_fut_.get();
    }

    /* Open a client TCP connection to the tuning server.
     * si_open_connection() returns ECONNREFUSED immediately (no internal
     * retry) if the server thread hasn't called si_listen() yet, so this
     * wrapper retries until the port is reachable or the attempt limit is
     * reached. */
    int connect_client(int port,
                       int max_attempts = 40,
                       int delay_ms     = 10)
    {
        const std::string port_s = std::to_string(port);
        for (int i = 0; i < max_attempts; ++i)
        {
            int fd = si_open_connection("127.0.0.1", port_s.c_str(), 1);
            if (fd >= 0)
                return fd;
            std::this_thread::sleep_for(
                std::chrono::milliseconds(delay_ms));
        }
        return SI_RC_ERROR;
    }

    awemgr_tuning_server *srv      = nullptr;
    std::string           port_str;

private:
    std::future<int> active_fut_;
    int              active_port_ = -1;
};

/* ================================================================
 * Tests: lifecycle with a valid awemgr_data handle
 * ================================================================ */
/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.ValidMgr_ReturnsNonNull~1
  covers: req~AWEMGR.Addon.TuningSocket~1
  description: Ensures that running a null server pointer returns -1.
```
*/
TEST_F(TuningServerFixture, Create_ValidMgr_ReturnsNonNull)
{
    /* The fixture SetUp already asserts non-null; create a second independent
     * context to confirm the function is repeatable. */
    awemgr_tuning_server *srv2 = awemgr_tuning_server_create(m_mgr_p, cfg_p);
    ASSERT_NE(srv2, nullptr);
    awemgr_tuning_server_destroy(srv2);
}

/* ================================================================
 * dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
 *
 * The addon shall use the AWE wire format for framing: the high 16 bits of
 * the first 32-bit word encode the total packet length in words.
 * ================================================================ */

/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.ZeroLengthHeader~1
  covers: dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
  description: Server drops the client when the AWE header encodes a packet length of zero words.
```
*/
TEST_F(TuningServerFixture, Run_ZeroLengthHeader_DropsClient)
{
    start_server(7302);
    fprintf(stderr, "Server started on port %s\n", port_str.c_str());

    int client = connect_client(7302);
    ASSERT_GE(client, 0) << "Could not connect to tuning server on port 7302";

    /* len_words = 0 in the high 16 bits — must be rejected by the server. */
    uint32_t hdr = make_awe_header(0u, 0x0029u);
    si_write(client, &hdr, static_cast<int>(sizeof(hdr)));

    /* After the invalid header the server must close the connection;
     * a subsequent read must return ≤ 0 (EOF or error). */
    char dummy;
    int r = si_readbuf(client, &dummy, 1, 0);
    EXPECT_LE(r, 0) << "Expected server to drop client after zero-length header";

    si_close_connection(client);
    EXPECT_EQ(wait_for_server(), 0);
}

/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.OversizeHeader~1
  covers: dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
  description: Server drops the client when the AWE header encodes a packet length exceeding the 4096-word tuning buffer.
```
*/
TEST_F(TuningServerFixture, Run_OversizeHeader_DropsClient)
{
    start_server(7303);

    int client = connect_client(7303);
    ASSERT_GE(client, 0) << "Could not connect to tuning server on port 7303";

    /* Default mgr.tuning_server.buf_words = 4096; 4097 must be rejected. */
    uint32_t hdr = make_awe_header(4097u, 0x0029u);
    si_write(client, &hdr, static_cast<int>(sizeof(hdr)));

    char dummy;
    int r = si_readbuf(client, &dummy, 1, 0);
    EXPECT_LE(r, 0) << "Expected server to drop client after oversize header";

    si_close_connection(client);
    EXPECT_EQ(wait_for_server(), 0);
}

/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.TruncatedBody~1
  covers: dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
  description: Server handles EOF mid-packet gracefully when the client closes after sending only the header word of a multi-word packet.
```
*/
TEST_F(TuningServerFixture, Run_TruncatedBody_DropsClient)
{
    start_server(7304);

    int client = connect_client(7304);
    ASSERT_GE(client, 0) << "Could not connect to tuning server on port 7304";

    /* Announce a 3-word packet but close after sending only the header.
     * The body never arrives → recv_all in serve_client returns 0 (EOF). */
    uint32_t hdr = make_awe_header(3u, 0x0029u);
    si_write(client, &hdr, static_cast<int>(sizeof(hdr)));
    si_close_connection(client);

    EXPECT_EQ(wait_for_server(), 0);
}

/* ================================================================
 * dsn~AWEMGR.ADDON.TUNINGSERVER.PacketForwarding~1
 * dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
 *
 * The addon shall accept AWE tuning packets over TCP, forward them to
 * awemgr_transact(), and return the AWE Core response to the client.
 * ================================================================ */

/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.RoundTrip~1
  covers:
    - dsn~AWEMGR.ADDON.TUNINGSERVER.PacketForwarding~1
    - dsn~AWEMGR.ADDON.TUNINGSERVER.AWEWireFormat~1
  description: A valid AWE GetTargetInfo packet sent over TCP is forwarded to AWECore via awemgr_transact and the response is relayed back to the client with a valid wire-format header.
```
*/
TEST_F(TuningServerFixture, Run_GetTargetInfo_ResponseReceived)
{
    start_server(7305);

    int client = connect_client(7305);
    ASSERT_GE(client, 0) << "Could not connect to tuning server on port 7305; "
                            "is the AWE server reachable?";

    /* AWE GetTargetInfo in wire format: 2-word packet.
     * req[0] bits[31:16] = 2 (len_words); verified in test_awe_mgr.cpp:TransactTargetInfo. */
    uint32_t req[2] = {0x00020029u, 0x00020029u};
    ASSERT_GT(si_write(client, req, static_cast<int>(sizeof(req))), 0)
        << "Failed to send GetTargetInfo request";

    /* Read and validate the response header word. */
    uint32_t rsp_hdr = 0;
    ASSERT_TRUE(recv_exactly(client, &rsp_hdr, static_cast<int>(sizeof(rsp_hdr))))
        << "No response received from tuning server";

    uint16_t rsp_len = static_cast<uint16_t>(rsp_hdr >> 16);
    EXPECT_GT(rsp_len, 0u)   << "Response header must encode a non-zero word count";
    EXPECT_LE(rsp_len, 512u) << "Response must fit within the 512-word tuning buffer";

    /* Drain remaining response words before closing so the server observes a
     * clean disconnect rather than a broken-pipe error. */
    if (rsp_len > 1)
    {
        std::vector<uint32_t> rest(rsp_len - 1);
        recv_exactly(client, rest.data(),
                     static_cast<int>(rest.size() * sizeof(uint32_t)));
    }

    si_close_connection(client);
    EXPECT_EQ(wait_for_server(), 0);
}

/* ================================================================
 * dsn~AWEMGR.ADDON.TUNINGSERVER.Respawn~1
 *
 * After a client disconnects, the server shall be able to accept a new
 * client connection without requiring a restart.
 * ================================================================ */

/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.Respawn~1
  covers: dsn~AWEMGR.ADDON.TUNINGSERVER.Respawn~1
  description: awemgr_tuning_server_run can be called again after a client disconnects, successfully accepting a second client on the same port.
```
*/
TEST_F(TuningServerFixture, Run_Respawn_AcceptsSecondClient)
{
    const int port = 7306;

    /* --- First client connects and disconnects cleanly. --- */
    start_server(port);

    int client1 = connect_client(port);
    ASSERT_GE(client1, 0) << "First client could not connect on port " << port;
    si_close_connection(client1);

    ASSERT_EQ(wait_for_server(), 0) << "First run() did not return 0";

    /* --- Second client connects and disconnects without a server restart. --- */
    start_server(port);

    int client2 = connect_client(port);
    ASSERT_GE(client2, 0) << "Second client could not connect on port " << port
                           << " — server failed to respawn";
    si_close_connection(client2);

    EXPECT_EQ(wait_for_server(), 0) << "Second run() did not return 0";
}

/* ================================================================
 * dsn~AWEMGR.ADDON.TUNINGSERVER.CleanShutdown~1
 *
 * A SIGTERM or SIGINT received while waiting for a client connection shall
 * cause the server to return cleanly so the calling service can shut down.
 * ================================================================ */

/**
```yaml
- id: itest~AWEMGR.ADDON.TUNINGSERVER.CleanShutdown~1
  covers: dsn~AWEMGR.ADDON.TUNINGSERVER.CleanShutdown~1
  description: SIGINT delivered to the thread blocked in accept() causes awemgr_tuning_server_run to unblock and return without crashing.
```
*/
TEST_F(TuningServerFixture, Run_SignalInterruptsAccept_ReturnsCleanly)
{
#ifdef _WIN32
    GTEST_SKIP() << "Signal-based accept interruption is not tested on Windows";
#else
    /* Install a no-op SIGINT handler without SA_RESTART so that the blocked
     * accept() syscall returns EINTR instead of being automatically restarted
     * or the default disposition (process termination) taking effect. */
    struct sigaction sa{}, old_sa{};
    sa.sa_handler = [](int) {};
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* no SA_RESTART */
    ASSERT_EQ(sigaction(SIGINT, &sa, &old_sa), 0);

    /* Communicate the server thread id so we can target pthread_kill precisely,
     * avoiding accidental delivery to the gtest main thread. */
    std::promise<thread_id_t> tid_promise;
    auto tid_future = tid_promise.get_future();

    start_server(7307, &tid_promise);

    /* Wait until the server thread has published its tid (i.e., it is past
     * the set_value() call and is approaching si_accept()). */
    thread_id_t server_tid = tid_future.get();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* Deliver SIGINT to the blocked server thread.  With the handler above,
     * accept() returns EINTR, si_accept() returns SI_RC_ERROR, and
     * awemgr_tuning_server_run() returns -1. */
    pthread_kill(server_tid, SIGINT);

    /* The server must unblock and return; a hang here indicates the
     * requirement is not met.  TearDown will drain the server via a dummy
     * client if the signal had no effect, preventing a fixture deadlock. */
    int rc = wait_for_server();

    /* Restore the original SIGINT disposition before asserting so a test
     * failure does not leave the process in a broken signal state. */
    sigaction(SIGINT, &old_sa, nullptr);

    /* accept() interrupted by a signal causes awemgr_tuning_server_run() to
     * return -1 (client_fd < 0 path).  The function must not crash. */
    EXPECT_EQ(rc, -1) << "Expected -1 (EINTR path) after SIGINT";
#endif
}
