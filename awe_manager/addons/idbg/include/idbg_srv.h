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


#ifndef INCLUSION_GUARD_IDBGSRV_H
#define INCLUSION_GUARD_IDBGSRV_H

#include "idbg.h"
#include <stdio.h>
#include <stdarg.h>
#include <atomic>

#define CMDLINE_SIZE 65536

class CIdbgSrv
{

public:
    CIdbgSrv(idbgtableentry_t *startTbl);

    virtual ~CIdbgSrv();

    int runConsole();
    int runFile(char *filename_p);
    int runSocket(char* socket_host, char* socket_port);
    int runCommand(const char* fmt, ...);

    /**
     * Signal a running runSocket() call to stop.
     *
     * Closes the listening and/or client socket so that any blocking
     * si_accept() or read call returns immediately with an error.
     * Safe to call from a different thread.  Has no effect if runSocket()
     * is not currently active.
     */
    void stop();

    void setUserData (void *data_p);

    void  setTimeCommands (bool b) { m_bTimeCommands = b; }
    bool  getTimeCommands ()       { return m_bTimeCommands; }

private:
    int getOneCommand (char *cmdbuffer, int maxcmdbuffersize, FILE *fp);
    int runFromHandle(FILE *fp);
    int runFromSocket(int fd);

    void reportTimedCommand(double elapsed_ms);

    char m_cmdline[CMDLINE_SIZE];
    idbg_t *m_idbg_p;
    std::atomic<int> m_server_fd;
    std::atomic<int> m_client_fd;

    bool m_bTimeCommands;
    bool m_script_mode;
};

#endif // INCLUSION_GUARD_IDBGSRV_H