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
#include <stdio.h>

int main(int argc, char const* argv[])
{
    printf("Hello Socket Server\n");

    int serverFd;
    int fd = si_create_server(9000, &serverFd, 10);

    if (fd > 0)
    {
        char buffer[1024] = { 0 };
        char* hello = "Hello from server\n";


        int valread = 1;
        while (valread != 0)
        {
            valread = si_readln(fd, buffer, 1024 - 1);
            printf("RX: %s (%d)\n", buffer, valread);
            si_writef(fd, hello, strlen(hello), 0);
            if (strncmp(buffer, "QUIT", strlen("QUIT")) == 0)
                valread = 0;

        }
        si_close_connection(fd);
    }
    printf("Goodbye Socket Server\n");
    si_close_server(serverFd);
    return 0;
}
