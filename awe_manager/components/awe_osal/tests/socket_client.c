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
    printf("Hello Socket Client\n");

    int fd = si_open_connection("127.0.0.1", "9000", 10);

    char buffer[1024] = { 0 };
    char *received;

#ifdef AWOSAL_WINDOWS
    received = gets_s(buffer, sizeof(buffer));
#else
    received = fgets(buffer, sizeof(buffer), stdin);
#endif
    si_writef(fd, "CLIENT: %s", received);

    si_close_connection(fd);

    printf("Goodbye Socket Client\n");

    return 0;
}
