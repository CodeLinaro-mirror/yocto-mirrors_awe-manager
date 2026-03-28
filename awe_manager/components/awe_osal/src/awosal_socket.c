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


#include "awosal_socket.h"
#include "awosal_logging.h"

#if defined (AWOSAL_WINDOWS)

// TODO: for Windows we need to check on socket handles here again:
// see discussion here: https://stackoverflow.com/questions/1953639/is-it-safe-to-cast-socket-to-int-under-win64
// for now it's safe to assume SOCKET is 32 bit, so int should be ok

#define	socklen_t unsigned int

#if defined (AWOSAL_COMPILER_MSVC)
typedef int ssize_t;
//typedef unsigned int size_t;
#endif

#include <windows.h>
#define bzero(buf,len)  memset (buf, 0, len)
#define close           closesocket
#include <winsock.h>

#else

// todo: cleanup here: copied from somewhere to get all deps resolved, list might be shorter

#include    <sys/types.h>   /* basic system data types */
#include    <sys/socket.h>  /* basic socket definitions */
#include    <sys/time.h>    /* timeval{} for select() */
#include    <time.h>        /* timespec{} for pselect() */
#include    <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include    <arpa/inet.h>   /* inet(3) functions */
#include    <errno.h>
#include    <fcntl.h>       /* for nonblocking */
#include    <netdb.h>
#include    <signal.h>
#include    <stdio.h>
#include    <sys/stat.h>    /* for S_xxx file mode constants */
#include    <sys/uio.h>     /* for iovec{} and readv/writev */
#include    <unistd.h>
#include    <sys/wait.h>
#include    <sys/un.h>      /* for Unix domain sockets */

#include    <sys/socket.h>
#include    <sys/select.h>  // for select()

#include    <strings.h>

#endif


// local function to create a socket in a platform independent manner
int os_Socket(int family, int type, int protocol)
{
    int	n;
#if defined (AWOSAL_WINDOWS)
    n = (int) socket(family, type, protocol);
    if (n == INVALID_SOCKET) {
        n = WSAGetLastError ();
        return -1;
    }
#else
    if ( (n = socket(family, type, protocol)) < 0)
        return -1;
#endif
  return n;
}

/**
 * local function to get most of the socket content into an internal buffer at once,
 * avoiding byte wise recv()
 */
static int my_read(int fd, char *ptr)
{
    static int   read_cnt = 0;
    static char *read_ptr, read_buf[MAX_LINE_LENGTH];

    if (read_cnt <= 0)
    {
again:
        if ( (read_cnt = recv(fd, read_buf, sizeof(read_buf), 0)) < 0)
        {
#if defined (AWOSAL_WINDOWS)
            if (read_cnt == SOCKET_ERROR)
#else
            if (errno == EINTR)
#endif
                goto again;

            return -1;
        }
        else if (read_cnt == 0)
            return 0;

        read_ptr = read_buf;
    }

    /* one byte will be been returned from the buffer */
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}


static ssize_t my_read_until(int fd, void *data_ptr, size_t maxlen, char stop_character)
{
    int     n, rc;
    char    c, *ptr;

    ptr = (char*) data_ptr;
    for (n = 1; n < (int) maxlen; n++)
    {
        rc = my_read(fd, &c);
        if (rc == 0)
        {
            if (n == 1)
                return 0;   /* EOF, no data read */
            else
                break;      /* EOF, some data was read */
        }
        else if (rc == 1)
        {
            /* store received byte, even if it the stopping character */
            *ptr++ = c;
            if (c == stop_character)
                break;
        }
        else
            return -1; // general error occured
    }

    // null terminate data, assume it's a string
    *ptr = 0;
    return n;
}

static ssize_t my_written(int fd, const void *data_ptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char  *ptr;

    if (!fd)
        return -1;

    ptr = (char*) data_ptr;
    nleft = n;
    while (nleft > 0)
    {
        if ( (nwritten = send(fd, ptr, (int) nleft,0)) <= 0)
        {
#if defined (AWOSAL_WINDOWS)
            if (nwritten == SOCKET_ERROR)
                return(-1);         /* error */
#else
            if (errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
#endif
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return((ssize_t) n);
}

static int set_socket_mode_blocking(int sockfd, bool blocking_mode_on)
{
#if ! defined (AWOSAL_WINDOWS)

    int arg;
    if( (arg = fcntl(sockfd, F_GETFL, NULL)) < 0) {
        AWOSAL_LOGE("Error fcntl(..., F_GETFL) (%s)", strerror(errno));
        return -1;
    }
    if (blocking_mode_on) {
        arg &= (~O_NONBLOCK);
    } else {
        arg |= O_NONBLOCK;
    }
    if( fcntl(sockfd, F_SETFL, arg) < 0) {
        AWOSAL_LOGE("Error fcntl(..., F_SETFL) (%s)", strerror(errno));
        return -1;
    }
#endif
    return 0;
}

int si_close_connection(int fd)
{
    if (!fd)
        return SI_RC_ERROR;
    close(fd);
    return 0;
}


int si_open_connection (const char *pchRemoteHost, const char *pchRemotePort, int timeout)
{
    int                sockfd = 0;
    struct sockaddr_in servaddr;
    int                cnt = 1;
    struct hostent*    hp;

#if defined (AWOSAL_WINDOWS)
    {
        WSADATA    sockDat;
        /* request a socket API with a correct version */
        WSAStartup (MAKEWORD (1, 1), &sockDat);
        if ( LOBYTE (sockDat.wVersion) != 1 ||
             HIBYTE (sockDat.wVersion) != 1) {
          WSACleanup();
          return SI_RC_ERROR;
        }
    }
#endif

    /* create a socket object */
    sockfd = os_Socket (AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return SI_RC_ERROR;

    /* set the address struct of the server */
    bzero ((void*) &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(atoi(pchRemotePort));

    /* get the hostname by its name */
    if ((hp = gethostbyname(pchRemoteHost)) == NULL)
    {
        close(sockfd);
        return SI_RC_ERROR;
    }

    /* copy the received information */
    memcpy(&(servaddr.sin_addr), hp->h_addr_list[0], hp->h_length);

    if (set_socket_mode_blocking(sockfd, false))
    {
        close(sockfd);
        return SI_RC_ERROR;
    }

    int res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (res < 0)
    {
        if (errno == EINPROGRESS) {
            /* socket connection not yet finalized and still in progress
               wait (with timeout via select()) until that specific handle
               becomes usable
             */

            struct timeval tv;
            fd_set fd_set_to_waitfor;

            do {
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                FD_ZERO(&fd_set_to_waitfor);
                FD_SET(sockfd, &fd_set_to_waitfor);

                res = select(sockfd+1, NULL, &fd_set_to_waitfor, NULL, &tv);

                if (res < 0 && errno != EINTR) {
                    // general error with that socket handle; bail out
                    AWOSAL_LOGE("Error connecting %d - %s", errno, strerror(errno));
                    return SI_RC_ERROR;
                }
                else if (res > 0)
                {
                    int value;
                    socklen_t length = sizeof(int);

                    /* check if the socket can be written to finally,
                       if getsockopt still returns a value != 0, then continue to wait
                       (unless we have timed out)
                     */
                    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&value), &length) < 0) {
                        AWOSAL_LOGE("Error with getsockopt() on '%s@%s': %d - %s", pchRemoteHost, pchRemotePort, errno, strerror(errno));
                        return SI_RC_ERROR;
                    }
                    if (value) {
                        AWOSAL_LOGE("Error in delayed connection() on '%s@%s': %d - %s", pchRemoteHost, pchRemotePort, value, strerror(value));
                        return SI_RC_ERROR;
                    }
                    break;
                }
                else {
                    /* timed out in select() after 1 second;
                       check a counter against provided timeout value
                     */
                    AWOSAL_LOGW("Could not connect to server '%s@%s'. %d/%d ... ", pchRemoteHost, pchRemotePort, cnt, timeout);
                    if (cnt == timeout)
                    {
                        AWOSAL_LOGE("waited for %d seconds and still no answer of host. bailing out", timeout);
                        return SI_RC_ERROR;
                    }
                    cnt ++;
                }
            } while (1);
        }
        else
        {
            AWOSAL_LOGE("Socket not connected, and not in EINPROGRESS state: errno()=%d", errno);
            return SI_RC_ERROR;
        }
    }

    if (set_socket_mode_blocking(sockfd, true))
    {
        close(sockfd);
        return SI_RC_ERROR;
    }

    return sockfd;
}

int si_readln (int fd, void *buffer, int buf_sz)
{
    int  liBytesRead = 0;

    if (!fd)
        return -1;

    /* clear buffer first */
    bzero (buffer, buf_sz);

    liBytesRead = (int) my_read_until(fd, buffer, buf_sz, '\n');

    return liBytesRead;
}

int si_write (int fd, void *buffer, int buf_sz)
{
    return my_written(fd, buffer, buf_sz);
}

int si_writef (int fd, const char *fmt, ...)
{
    char buf[MAX_LINE_LENGTH];
    va_list ap;

    if (!fd)
        return -1;

    va_start (ap, fmt);
    vsnprintf(buf, MAX_LINE_LENGTH, fmt, ap);

    va_end   (ap);

    return (si_write (fd, buf, (int) strlen (buf)));
}

int si_writef_va (int fd, const char *fmt, va_list ap)
{
    char buf[MAX_LINE_LENGTH];

    if (!fd)
        return -1;

    vsnprintf (buf, MAX_LINE_LENGTH, fmt, ap);
    return (si_write (fd, buf, (int) strlen (buf)));
}

int si_readbuf(int fd, void *buffer, int buf_sz, int cnt)
{
    int read_cnt = recv(fd, buffer, (size_t)buf_sz, 0);
    return read_cnt;
}

bool si_timedout(int *sys_err_code)
{
    int ret_val = 0;
    int error_code = 0;
#if defined (AWOSAL_WINDOWS)
    ret_val = WSAGetLastError();
    error_code = WSAETIMEDOUT;
#else
    ret_val = errno;
    error_code = EAGAIN;
#endif
    if (sys_err_code)
        *sys_err_code = ret_val;
    return (ret_val == error_code);
}

int si_set_timeout(int fd, int timeout_ms)
{
    int rc;
#if defined (AWOSAL_WINDOWS)
    DWORD timeout = timeout_ms;
    rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
#else
    struct timeval tv;
    tv.tv_sec  = (timeout_ms > 0 ? timeout_ms / 1000 : 0);
    tv.tv_usec = (timeout_ms > 0 ? 1000*(timeout_ms % 1000) : 0);
    rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
#endif
    return rc;
}

int si_create_server(int iListenPort, int* serverFd, int timeout)
{
    // todo: timeout not used, how could we use that?!

    unsigned int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

#if defined (AWOSAL_WINDOWS)
    {
        WSADATA    sockDat;
        /* request a socket API with a correct version */
        WSAStartup (MAKEWORD (1, 1), &sockDat);
        if ( LOBYTE (sockDat.wVersion) != 1 ||
             HIBYTE (sockDat.wVersion) != 1) {
          WSACleanup();
          return SI_RC_ERROR;
        }
    }
#endif

    // Creating socket file descriptor
    server_fd = os_Socket (AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket failed");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR, (const char*)&opt,
                   sizeof(opt))) {
        return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(iListenPort);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return -1;
    }
#if defined (AWOSAL_WINDOWS)
    if ((new_socket = (int) accept((SOCKET) server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
#else
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
#endif
        perror("accept");
        return -1;
    }

    *serverFd = server_fd;
    return new_socket;
}

int si_close_server(int server_fd)
{
    if (server_fd)
    {
        // closing the listening socket
        close(server_fd);
    }
    return 0;
}