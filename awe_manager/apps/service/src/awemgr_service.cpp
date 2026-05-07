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
 * @file awemgr_service.cpp
 * @brief AWE Manager Linux service
 *
 * Long-running process managed by systemd.  Logs to stderr so that journald
 * captures every message with automatic metadata (PID, unit name, timestamp).
 * Do NOT add timestamps or log-level prefixes in this file — journald provides
 * them via the SD_* severity prefixes.
 *
 * Lifecycle
 * ---------
 *   1. Parse arguments and apply configuration.
 *   2. Initialise AWE Manager (awemgr_init).
 *   3. Optionally load an AWC file and a design.
 *   4. Block until SIGTERM or SIGINT is received.
 *   5. Unload the AWC, call awemgr_exit, and return.
 *
 * Suitable systemd unit snippet
 * ------------------------------
 *   [Service]
 *   Type=simple
 *   ExecStart=/usr/bin/awemgr_service -awc /etc/awe/config.awc -design Main
 *   Restart=on-failure
 *   StandardOutput=journal
 *   StandardError=journal
 */

#include "awe_manager.h"
#include "awe_config.h"

#ifdef AWEMGR_SERVICE_WITH_SHELL
#include "awemgr_shell.h"
#endif

#ifdef AWEMGR_SERVICE_WITH_TUNING_SERVER
#include "awemgr_tuning_server.h"
#endif

#include <chrono>
#include <thread>

#include <csignal>
#ifndef _WIN32
#include <pthread.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* --------------------------------------------------------------------------
 * journald severity prefixes (from <systemd/sd-daemon.h> — reproduced here
 * to avoid a hard build dependency on the systemd development headers).
 * -------------------------------------------------------------------------- */
#define SD_EMERG   "<0>"
#define SD_ALERT   "<1>"
#define SD_CRIT    "<2>"
#define SD_ERR     "<3>"
#define SD_WARNING "<4>"
#define SD_NOTICE  "<5>"
#define SD_INFO    "<6>"
#define SD_DEBUG   "<7>"

/* Convenience wrappers — journald reads the prefix from the first bytes of
 * each line written to stderr; no newline is added by the prefix itself. */
#define LOG_ERR(fmt, ...)  fprintf(stderr, SD_ERR    "awemgr_service: " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) fprintf(stderr, SD_WARNING "awemgr_service: " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) fprintf(stderr, SD_INFO    "awemgr_service: " fmt "\n", ##__VA_ARGS__)
#define LOG_DBG(fmt, ...)  fprintf(stderr, SD_DEBUG   "awemgr_service: " fmt "\n", ##__VA_ARGS__)

/* --------------------------------------------------------------------------
 * Signal handling
 * -------------------------------------------------------------------------- */

static volatile sig_atomic_t g_stop_requested = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_stop_requested = 1;
}

static void install_signal_handlers(void)
{
#ifdef _WIN32
    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);
#else
    struct sigaction sa{};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT,  &sa, nullptr);
#endif
}

/* --------------------------------------------------------------------------
 * Argument helpers
 * -------------------------------------------------------------------------- */

static const char *find_arg(int argc, char *argv[], const char *flag)
{
    for (int i = 1; i < argc - 1; ++i)
        if (strcmp(argv[i], flag) == 0)
            return argv[i + 1];
    return nullptr;
}

static bool has_flag(int argc, char *argv[], const char *flag)
{
    for (int i = 1; i < argc; ++i)
        if (strcmp(argv[i], flag) == 0)
            return true;
    return false;
}

static void print_usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [OPTIONS]\n"
        "\n"
        "Options:\n"
        "  -awc <file>            AWC configuration file to load at startup\n"
        "  -design <name>         Design (AWB) to load after -awc (e.g. 'Main')\n"
        "  -cfg <string>          AWE Manager config overrides, e.g. 'mgr.api.log.level=debug;'\n"
        "  -awe_socket <ip@port>  AWE Server address (e.g. 192.168.1.1@15002); overrides cfg\n"
#ifdef AWEMGR_SERVICE_WITH_SHELL
        "  -shell_socket <port>   Accept shell commands on a TCP socket (localhost only)\n"
#endif
#ifdef AWEMGR_SERVICE_WITH_TUNING_SERVER
        "  -tuning_socket <port>  Accept AWE tuning packets on a TCP socket (all interfaces)\n"
#endif
        "  -version               Print version and exit\n"
        "  -h, --help             Print this help and exit\n"
        "\n"
        "Environment:\n"
        "  AWEMGR_CFG_OVERRIDE   Semicolon-separated config overrides (same syntax as -cfg)\n",
        prog);
}

/* --------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    if (has_flag(argc, argv, "-h") || has_flag(argc, argv, "--help"))
    {
        print_usage(argv[0]);
        return 0;
    }

    if (has_flag(argc, argv, "-version"))
    {
        fprintf(stdout, "%s\n", awemgr_get_version());
        return 0;
    }

    const char *awc_file        = find_arg(argc, argv, "-awc");
    const char *design_name     = find_arg(argc, argv, "-design");
    const char *cfg_string      = find_arg(argc, argv, "-cfg");
    const char *awe_socket      = find_arg(argc, argv, "-awe_socket");
#ifdef AWEMGR_SERVICE_WITH_SHELL
    const char *shell_sock_port  = find_arg(argc, argv, "-shell_socket");
#endif
#ifdef AWEMGR_SERVICE_WITH_TUNING_SERVER
    const char *tuning_sock_port = find_arg(argc, argv, "-tuning_socket");
#endif

    if (design_name && !awc_file)
    {
        LOG_ERR("-design requires -awc");
        return EXIT_FAILURE;
    }

    install_signal_handlers();

    /* ---- Configuration ---- */
    awe_config *cfg_p = nullptr;
    if (awemgr_config_create(&cfg_p) != awemgr_RC_OK)
    {
        LOG_ERR("Failed to create AWE Manager configuration");
        return EXIT_FAILURE;
    }

    /* Register addon config keys with their defaults before user overrides are
     * applied, so the defaults are in place when the config string / env-var
     * overrides are parsed below. */
#ifdef AWEMGR_SERVICE_WITH_TUNING_SERVER
    awemgr_tuning_server_register_configs(cfg_p);
#endif
#ifdef AWEMGR_SERVICE_WITH_SHELL
    awemgr_shell_register_configs(cfg_p);
#endif

    /* Apply -cfg argument first, then let the environment variable override. */
    if (cfg_string)
    {
        if (aweconfig_from_string(cfg_p, cfg_string) != 0)
            LOG_WARN("Could not apply config string: %s", cfg_string);
    }
    aweconfig_from_envvar(cfg_p, AWEMGR_CFG_OVERRIDE);

    if (awe_socket)
    {
        const char *at = strchr(awe_socket, '@');
        if (!at || at == awe_socket || *(at + 1) == '\0')
        {
            LOG_ERR("Invalid -awe_socket format (expected ip@port, e.g. 192.168.1.1@15002): %s", awe_socket);
            awemgr_config_destroy(&cfg_p);
            return EXIT_FAILURE;
        }
        char ip[64] = {};
        memcpy(ip, awe_socket, (size_t)(at - awe_socket));
        aweconfig_set(cfg_p, "mgr.comm.socket.ip", ip);
        aweconfig_set(cfg_p, "mgr.comm.socket.port", at + 1);
        LOG_INFO("AWE socket: %s port %s", ip, at + 1);
    }

    /* ---- Initialise AWE Manager ---- */
    struct awemgr_data *mgr_p = nullptr;
    LOG_INFO("Initialising AWE Manager %s", awemgr_get_version());

    if (awemgr_init(&cfg_p, &mgr_p) != awemgr_RC_OK)
    {
        LOG_ERR("awemgr_init failed");
        awemgr_config_destroy(&cfg_p);
        return EXIT_FAILURE;
    }

    /* ---- Load AWC ---- */
    /* Use endpoint 0 for a single-canvas setup; multi-canvas support can be
     * added later via additional arguments or a config file. */
    const int endpoint_id = 0;

    if (awc_file)
    {
        LOG_INFO("Loading AWC: %s (endpoint %d)", awc_file, endpoint_id);
        if (awemgr_load_awc(mgr_p, awc_file, endpoint_id) != awemgr_RC_OK)
        {
            LOG_ERR("Failed to load AWC: %s", awc_file);
            awemgr_exit(&mgr_p);
            return EXIT_FAILURE;
        }

        /* ---- Load design ---- */
        if (design_name)
        {
            struct awemgr_ctx *awc_ctx_p = awemgr_get_awc_context(mgr_p, endpoint_id);
            if (!awc_ctx_p)
            {
                LOG_ERR("Failed to get AWC context for endpoint %d", endpoint_id);
                awemgr_exit(&mgr_p);
                return EXIT_FAILURE;
            }

            LOG_INFO("Loading design: %s", design_name);
            if (awemgr_load_design(awc_ctx_p, design_name) != awemgr_RC_OK)
            {
                LOG_ERR("Failed to load design: %s", design_name);
                awemgr_exit(&mgr_p);
                return EXIT_FAILURE;
            }
        }
    }

    LOG_INFO("Running — send SIGTERM or SIGINT to stop");

#ifndef _WIN32
    /* Block stop signals in the main thread before spawning addon threads.
     * Worker threads inherit the blocked mask, so SIGINT/SIGTERM are only
     * ever delivered to the main thread — keeping pause() reliable. */
    sigset_t stop_signals;
    sigemptyset(&stop_signals);
    sigaddset(&stop_signals, SIGINT);
    sigaddset(&stop_signals, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &stop_signals, nullptr);
#endif

#ifdef AWEMGR_SERVICE_WITH_SHELL
    awemgr_shell_ctx *shell = awemgr_shell_create(mgr_p, cfg_p);
    std::thread shell_thread;
    if (!shell)
    {
        LOG_WARN("Failed to create shell context — runtime command support unavailable");
    }
    else if (shell_sock_port)
    {
        LOG_INFO("Shell socket listening on localhost:%s", shell_sock_port);
        shell_thread = std::thread([shell, shell_sock_port]() {
            while (!g_stop_requested)
                awemgr_shell_run_socket(shell, "localhost", shell_sock_port);
        });
    }
#endif

#ifdef AWEMGR_SERVICE_WITH_TUNING_SERVER
    awemgr_tuning_server *tuning_srv = nullptr;
    std::thread tuning_thread;
    if (tuning_sock_port)
    {
        tuning_srv = awemgr_tuning_server_create(mgr_p, cfg_p);
        if (!tuning_srv)
        {
            LOG_WARN("Failed to create tuning server — AWE Designer connectivity unavailable");
        }
        else
        {
            LOG_INFO("Tuning socket listening on %s", tuning_sock_port);
            tuning_thread = std::thread([tuning_srv, tuning_sock_port]() {
                while (!g_stop_requested)
                    awemgr_tuning_server_run(tuning_srv, tuning_sock_port);
            });
        }
    }
#endif

    /* ---- Main loop — block until a stop signal is received ---- */
#ifndef _WIN32
    /* Unblock stop signals now that all threads have been created.
     * From this point on SIGINT/SIGTERM are guaranteed to reach this thread,
     * which is the only one that doesn't have them blocked. */
    pthread_sigmask(SIG_UNBLOCK, &stop_signals, nullptr);
#endif
    while (!g_stop_requested)
    {
#ifdef _WIN32
        /* Windows has no pause(); poll at low frequency instead. */
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#else
        pause(); /* sleep until any signal wakes us */
#endif
    }

    LOG_INFO("Stop requested, shutting down");

    /* ---- Stop addon threads and wait for them to finish before freeing
     *      the manager handle they may still be using. ---- */
#ifdef AWEMGR_SERVICE_WITH_SHELL
    if (shell)
    {
        awemgr_shell_execute(shell, "/event -stop");
        awemgr_shell_stop(shell);
        if (shell_thread.joinable())
            shell_thread.join();
        awemgr_shell_destroy(shell);
    }
#endif

#ifdef AWEMGR_SERVICE_WITH_TUNING_SERVER
    if (tuning_srv)
    {
        awemgr_tuning_server_stop(tuning_srv);
        if (tuning_thread.joinable())
            tuning_thread.join();
        awemgr_tuning_server_destroy(tuning_srv);
    }
#endif

    /* ---- Cleanup ---- */
    if (awemgr_exit(&mgr_p) != awemgr_RC_OK)
        LOG_WARN("awemgr_exit reported an error");

    LOG_INFO("Shutdown complete");
    return EXIT_SUCCESS;
}
