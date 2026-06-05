#include <gtest/gtest.h>
#include <stdarg.h>
#include <thread>
#include <atomic>
#include <chrono>
#include "awosal_socket.h"
#include "awosal_time.h"

class SocketServer {
private:
    int server_fd;
    int port;

    int client_fd;
    bool running;

    std::thread server_thread;    // Thread to run the server loop

    public:
    // Constructor
    explicit SocketServer(int port) : port(port), running(false) {}

    void run_server() {
        running = true;
        client_fd = si_create_server(port, &server_fd, -1);
        ASSERT_GT(server_fd, 0);
        ASSERT_GT(client_fd, 0);
        char buffer[1024] = { 0 };
        while (running)
        {
            int numBytes = si_readbuf(client_fd, buffer, 1024 - 1, 0);
            if(!running || strncmp(buffer, "ECHO_STOP", strlen("ECHO_STOP")) == 0)
            {
                // fprintf(stderr, running ? "SocketServer: Found ECHO_STOP!\n" : "Stop flag set");
                break;
            }
            ASSERT_GT(numBytes, 0);
            // echo back!
            ASSERT_EQ(si_write(client_fd, buffer, numBytes), numBytes);
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
        // note: this only works when client has sent ECHO_STOP before!
        si_close_server(server_fd);
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }
};


// this test fixture is supposed to be set up ONCE for all test cases in this file,
// after all tests have been conducted, it is torn down again,
// this is the reason why the methods and the ctx_ are static;
//
class AweOSALTestFixture : public testing::Test
{
protected:
    static void SetUpTestSuite() {
        fprintf(stderr, "SetUpTestSuite: Starting socket server and client...\n");
        const char* port = "9000";
        // ctx_ = new ServerClient_Combo;
        server = new SocketServer(atoi(port));
        server->start();
        aweosal_mssleep(250);  // wait a while until we connect
        clientFd = si_open_connection("127.0.0.1", port, 10);
        ASSERT_GT(clientFd, 0);
    };

    static void TearDownTestSuite()
    {
        fprintf(stderr, "TearDownTestSuite: Sending STOP to socket server and closing...\n");
        ASSERT_EQ(si_writef(clientFd, "%s", "ECHO_STOP"), strlen("ECHO_STOP"));
        server->stop();

        delete server;
    }

    static SocketServer* server;
    static int clientFd;

};

SocketServer* AweOSALTestFixture::server = NULL;
int AweOSALTestFixture::clientFd = 0;

// TODO: should likely become a function in OSAL layer!
int test_si_writef(int fd, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = si_writef_va(0, fmt, ap);  // INVALID file/socket descriptor
    EXPECT_EQ(result, -1);
    result = si_writef_va(fd, fmt, ap);
    va_end(ap);
    return result;
}


TEST_F(AweOSALTestFixture, TestCommunication_NoCR)
{
    char buffer[1024] = {0};
    ASSERT_EQ(si_writef(clientFd, "%s", "Hello World"), strlen("Hello World"));
    ASSERT_EQ(si_readbuf(clientFd, buffer, sizeof(buffer), 0), strlen("Hello World"));
}


TEST_F(AweOSALTestFixture, TestCommunication_CR)
{
    char buffer[1024] = {0};

    ASSERT_EQ(si_writef(clientFd, "%s", "Hello World\n"), strlen("Hello World\n"));
    ASSERT_EQ(si_readln(clientFd, buffer, sizeof(buffer)), strlen("Hello World\n"));
}


TEST_F(AweOSALTestFixture, TestComm_Vargs)
{
    char buffer[1024] = {0};

    ASSERT_EQ(test_si_writef(clientFd, "Hello %s\n", "World"), strlen("Hello World\n"));
    ASSERT_EQ(si_readln(clientFd, buffer, sizeof(buffer)), strlen("Hello World\n"));
}


TEST_F(AweOSALTestFixture, TestTimeout)
{
    char buffer[1024] = {0};

    ASSERT_EQ(si_set_timeout(clientFd, 1000), 0);

    auto start = std::chrono::high_resolution_clock::now();

    ASSERT_EQ(si_readbuf(clientFd, buffer, sizeof(buffer), 0), -1);

    auto end = std::chrono::high_resolution_clock::now();

    int system_error_no;
    ASSERT_EQ(si_timedout(&system_error_no), true);

    double elapsed_seconds = std::chrono::duration<double>(end - start).count();

    ASSERT_NEAR(1.0, elapsed_seconds, 0.1);
}


extern "C"
{
    int os_Socket(int family, int type, int protocol);
}

TEST(AweOSALSocket, ErrorCases)
{
    char buffer[1024] = {0};
    ASSERT_EQ(os_Socket(0,0,0), -1);
    ASSERT_EQ(si_writef(0, "%s", "Hello World\n"), -1);
    ASSERT_EQ(si_write(0, buffer, sizeof(buffer)), -1);
    ASSERT_EQ(si_readln(0, buffer, sizeof(buffer)), -1);
    ASSERT_EQ(si_close_connection(0), SI_RC_ERROR);
    ASSERT_EQ(si_open_connection("127.0.0.1", "1231", 0), SI_RC_ERROR); // Invalid port
    ASSERT_EQ(si_open_connection("192.0.2.1", "9999", 2), SI_RC_ERROR); // Invalid IP is a test net, likely not to respond, will timeout after 2 seconds
}
