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


#ifndef INCLUSION_GUARD_IDBGSRV_H
#define INCLUSION_GUARD_IDBGSRV_H

#include "idbg.h"
#include <stdio.h>
#include <stdarg.h>

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

    void setUserData (void *data_p);

private:
    int getOneCommand (char *cmdbuffer, int maxcmdbuffersize, FILE *fp);
    int runFromHandle(FILE *fp);

    char m_cmdline[CMDLINE_SIZE];
    idbg_t *m_idbg_p;
    int m_client_fd;
};

#endif // INCLUSION_GUARD_IDBGSRV_H