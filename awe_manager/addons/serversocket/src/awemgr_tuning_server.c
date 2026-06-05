/* MIT License
**
** Copyright (c) 2026 DSP Concepts, Inc.
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

#include "awemgr_tuning_server.h"
#include "awosal_socket.h"
#include "awe_config.h"

#include <stdint.h>
/* MSVC 17.6 has a bug in <stdatomic.h>: int_fast16_t (typedef int) conflicts
 * with the already-instantiated atomic_int, causing syntax errors in
 * vcruntime_c11_stdatomic.h.  Work around using compiler intrinsics. */
#if defined(_MSC_VER)
#  include <intrin.h>
   typedef volatile long atomic_int;
#  define atomic_init(obj, val)     (*(obj) = (long)(val))
#  define atomic_load(obj)          ((long)_InterlockedOr((obj), 0))
#  define atomic_store(obj, val)    ((void)_InterlockedExchange((obj), (long)(val)))
#  define atomic_exchange(obj, val) _InterlockedExchange((obj), (long)(val))
#else
#  include <stdatomic.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFG_TUNING_SERVER_BUF_WORDS  "mgr.tuning_server.buf_words"
#define CFG_TUNING_SERVER_TIMEOUTMS  "mgr.tuning_server.timeoutms"
#define DEFAULT_TUNING_SERVER_BUF_WORDS "4096"
#define DEFAULT_TUNING_SERVER_TIMEOUTMS "-1"
#define TUNING_BUF_WORDS_FALLBACK    4096u

/* --------------------------------------------------------------------------
 * Context
 * -------------------------------------------------------------------------- */

struct awemgr_tuning_server
{
    struct awemgr_data *mgr_p;
    unsigned int        buf_words;
    atomic_int          server_fd;      /* listening socket, -1 when not active */
    atomic_int          client_fd;      /* connected client socket, -1 when not active */
    atomic_int          stop_requested; /* set to 1 by stop() before touching fds */
};


/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

int awemgr_tuning_server_register_configs(awe_config *cfg_p)
{
    if (!cfg_p)
        return AWECFG_RC_FAIL;

    aweconfig_init_tuple configs[] = {
        {CFG_TUNING_SERVER_BUF_WORDS, DEFAULT_TUNING_SERVER_BUF_WORDS,
         "Size of AWE tuning packet buffer in 32-bit words", NULL, NULL},
        {CFG_TUNING_SERVER_TIMEOUTMS, DEFAULT_TUNING_SERVER_TIMEOUTMS,
         "Client socket connection timeout in milliseconds (-1 = wait indefinitely)", NULL, NULL},
    };
    return aweconfig_add_multiple(cfg_p, configs, sizeof(configs) / sizeof(configs[0]));
}

awemgr_tuning_server *awemgr_tuning_server_create(struct awemgr_data *mgr_p,
                                                   awe_config *cfg_p)
{
    if (!mgr_p)
        return NULL;

    awemgr_tuning_server *srv = calloc(1, sizeof(*srv));
    if (!srv)
        return NULL;

    srv->mgr_p = mgr_p;
    srv->buf_words = TUNING_BUF_WORDS_FALLBACK;
    atomic_init(&srv->server_fd,      -1);
    atomic_init(&srv->client_fd,      -1);
    atomic_init(&srv->stop_requested,  0);

    if (cfg_p)
    {
        uint32_t val;
        if (aweconfig_get_as_uint(cfg_p, CFG_TUNING_SERVER_BUF_WORDS, &val) == AWECFG_RC_OK
                && val > 0)
            srv->buf_words = val;
    }

    return srv;
}

void awemgr_tuning_server_destroy(awemgr_tuning_server *srv)
{
    free(srv);
}

void awemgr_tuning_server_stop(awemgr_tuning_server *srv)
{
    if (!srv)
        return;

    /* Set the flag FIRST so that run() can detect a stop() that fired in the
     * window between si_accept() returning and atomic_store(client_fd). */
    atomic_store(&srv->stop_requested, 1);

    int sfd = atomic_exchange(&srv->server_fd, -1);
    if (sfd >= 0)
        si_close_server(sfd);

    int cfd = atomic_exchange(&srv->client_fd, -1);
    if (cfd >= 0)
        si_close_connection(cfd);
}


/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/* Read exactly n bytes from fd into buf.
 * Returns n on success, 0 on EOF, -1 on error. */
static int recv_all(int fd, void *buf, int n)
{
    int received = 0;
    while (received < n)
    {
        int r = si_readbuf(fd, (char *)buf + received, n - received, 0);
        if (r <= 0)
            return r;
        received += r;
    }
    return received;
}

/* Service one connected client: read AWE tuning packets, transact, reply.
 * Returns when the client disconnects or a fatal error occurs. */
static void serve_client(struct awemgr_tuning_server *srv)
{
    unsigned int *req_buf = malloc(srv->buf_words * sizeof(unsigned int));
    unsigned int *rsp_buf = malloc(srv->buf_words * sizeof(unsigned int));
    if (!req_buf || !rsp_buf)
    {
        fprintf(stderr, "awemgr_tuning_server: buffer allocation failed (%u words)\n",
                srv->buf_words);
        free(req_buf);
        free(rsp_buf);
        return;
    }

    for (;;)
    {
        /* Read the first word to determine packet length.
         * AWE wire format: bits[31:16] = total packet length in words. */
        uint32_t header_word;
        int client_fd = atomic_load(&srv->client_fd);
        int n = recv_all(client_fd, &header_word, (int)sizeof(header_word));
        if (n == 0)
            break;  /* clean EOF — client closed the connection */
        if (n < 0)
        {
            fprintf(stderr, "awemgr_tuning_server: recv error\n");
            break;
        }

        uint16_t len_words = (uint16_t)(header_word >> 16);
        if (len_words == 0 || len_words > srv->buf_words)
        {
            fprintf(stderr, "awemgr_tuning_server: invalid packet length %u words\n",
                    (unsigned)len_words);
            break;
        }

        req_buf[0] = header_word;

        /* Read the remainder of the packet (len_words - 1 words). */
        if (len_words > 1)
        {
            int remaining = (int)((len_words - 1) * sizeof(unsigned int));
            n = recv_all(client_fd, &req_buf[1], remaining);
            if (n <= 0)
            {
                if (n < 0)
                    fprintf(stderr, "awemgr_tuning_server: recv error (body)\n");
                break;
            }
        }

        /* Forward packet to AWECore via AWE Manager. */
        enum awemgr_rc rc = awemgr_transact(
            srv->mgr_p,
            req_buf, (int)len_words,
            rsp_buf, (int)srv->buf_words);

        if (rc == awemgr_RC_COMM_TIMEOUT || rc == awemgr_RC_ERR)
        {
            /* Communication failure — no usable response to forward. */
            fprintf(stderr, "awemgr_tuning_server: awemgr_transact failed (%d), "
                    "dropping client\n", rc);
            break;
        }

        /* Read response packet length from the response buffer header word.
         * AWECore fills rsp_buf[0] using the same AWE wire format. */
        uint16_t rsp_len_words = (uint16_t)(rsp_buf[0] >> 16);
        if (rsp_len_words == 0 || rsp_len_words > srv->buf_words)
        {
            fprintf(stderr, "awemgr_tuning_server: invalid response length %u words\n",
                    (unsigned)rsp_len_words);
            break;
        }

        if (si_write(client_fd, rsp_buf,
                     (int)(rsp_len_words * sizeof(unsigned int))) < 0)
        {
            fprintf(stderr, "awemgr_tuning_server: send error\n");
            break;
        }
    }

    free(req_buf);
    free(rsp_buf);
}


/* --------------------------------------------------------------------------
 * Public: run one accept-service-disconnect cycle
 * -------------------------------------------------------------------------- */

int awemgr_tuning_server_run(awemgr_tuning_server *srv,
                             const char *port)
{
    if (!srv)
        return -1;

    int server_fd;
    if (si_listen(atoi(port), &server_fd) != 0)
    {
        fprintf(stderr, "awemgr_tuning_server: listening on port %s failed\n", port);
        return -1;
    }

    atomic_store(&srv->server_fd, server_fd);
    fprintf(stderr, "awemgr_tuning_server: listening on port %s\n", port);

    int client_fd = si_accept(server_fd);

    /* Close the listening socket.  Use atomic_exchange so that a concurrent
     * stop() cannot double-close the same fd via its own exchange — only one
     * side will get the non-(-1) value and actually call si_close_server. */
    int sfd = atomic_exchange(&srv->server_fd, -1);
    if (sfd >= 0)
        si_close_server(sfd);

    if (client_fd < 0)
    {
        fprintf(stderr, "awemgr_tuning_server: accept interrupted by stop() or signal\n");
        return -1;
    }

    /* Store the client fd so stop() can close it if needed. */
    atomic_store(&srv->client_fd, client_fd);

    /* Guard against the race window where stop() fired between si_accept()
     * returning and the store above: stop() would have found client_fd == -1
     * and done nothing.  Now that the fd is visible, one side wins the
     * exchange and closes it; the other side gets -1 and skips the close. */
    if (atomic_load(&srv->stop_requested))
    {
        int cfd = atomic_exchange(&srv->client_fd, -1);
        if (cfd >= 0)
            si_close_connection(cfd);
        return -1;
    }

    serve_client(srv);

    int cfd = atomic_exchange(&srv->client_fd, -1);
    if (cfd >= 0)
        si_close_connection(cfd);

    fprintf(stderr, "awemgr_tuning_server: client disconnected\n");

    return 0;
}
