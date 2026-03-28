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


#ifndef INCLUSION_GUARD_AWOSAL_SOCKET_H
#define INCLUSION_GUARD_AWOSAL_SOCKET_H

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef WIN32
#define AWOSAL_WINDOWS
#endif
#ifdef _MSC_VER
#define AWOSAL_COMPILER_MSVC
#define AWOSAL_WINDOWS
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/** defines maximum line length for text writes */
#define MAX_LINE_LENGTH 1024


#define SA  struct sockaddr

/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define LISTENQ     1024    /* 2nd argument to listen() */

/** defines an invalid socket handle value or error */
#define SI_RC_ERROR  -1

/**
 * Opens a socket connection to a TCP server specified by host name and port,
 * the routine tries at least a few times until timeout occurs.
 * 
 * @param pchRemoteHost[in] - host name
 * @param pchRemotePort[in] - socket port (as STRING!!)
 * @param timeout[in] - number of seconds to wait until timeout error 
 * 
 * @returns 0 on success, SI_RC_ERROR otherwise
 */
int si_open_connection(const char *pchRemoteHost, const char *pchRemotePort, int timeout);

/**
 * Closes a socket client connection previously opened with si_open_connection()
 * 
 * @param fd[in] - socket file handle
 * 
 * @returns 0 on success, SI_RC_ERROR otherwise
 */
int si_close_connection(int fd);

/**
 * Reads a (raw) data package of fixed size from the socket
 * 
 * @param fd[in] - socket file handle
 * @param buffer[in] - ptr to buffer to be filled
 * @param buf_sz[in] - size of buffer in bytes
 */
int si_readbuf(int fd, void *buffer, int buf_sz, int cnt);

/**
 * reads from a socket until end of line character is observed,
 * buffer is filled (incl. 0 termination byte)
 * 
 * @param fd[in] - socket file handle
 * @param buffer[in] - ptr to buffer to be filled
 * @param buf_sz[in] - size of buffer in bytes
 * 
 * @returns number of bytes obtained (exl. termination byte!)
 */
int si_readln (int fd, void *buffer, int buf_sz);

/**
 * Writes a (raw) buffer to the socket
 * 
 * @param fd[in] - socket file handle
 * @param buffer[in] - ptr to buffer from where to get data
 * @param buf_sz[in] - size of buffer in bytes
 * 
 * @returns number of bytes written
 */
int si_write (int fd, void *buffer, int buf_sz);

/**
 * Writes a formatted string buffer to the socket
 * 
 * @param fd[in] - socket file handle
 * @param fmt[in] - printf like format specifier
 * @param ...[in] - variable arguments
 * 
 * @returns number of bytes written
 */
int si_writef (int fd, const char *fmt, ...);

/**
 * Similar to si_writef() but already consumes a var arg structure
 * 
 * @param fd[in] - socket file handle
 * @param fmt[in] - printf like format specifier
 * @param ap[in] - var args structure
 * 
 * @returns number of bytes written
 */
int si_writef_va (int fd, const char *fmt, va_list ap);

/**
 * Sets the timeout value on a socket client port
 * 
 * @param fd[in] - socket file handle
 * 
 * @returns 0 if socket timeout was applied, !=0 in error case
 */
int si_set_timeout(int fd, int timeout_ms);

/**
 * When a read function has failed, this routine can be used to
 * obtain the system error code; the routine also directly
 * returns the information if the error was caused due to a 
 * waiting timeout
 * 
 * @param sys_err_code[in/out] - ptr to variable that will receive
 *                               the system error code; it's up to the client
 *                               code to interpret this value!
 * @returns true when last read error was caused by a timeout
 */
bool si_timedout(int *sys_err_code);


int si_create_server(int iListenPort, int* serverFd, int timeout);
int si_close_server(int server_fd);


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // INCLUSION_GUARD_AWOSAL_SOCKET_H
