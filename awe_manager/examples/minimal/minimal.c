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


#include "awe_manager.h" // the AWE Manager include
#include "awe_ctrl.h"    // for setting configuration to AWECore comm, before AWEMgr starts

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ******************************************************************************************************

int main(int argc, char* argv[])
{
    struct awemgr_data *mgr_p = NULL;   // NULL is important

    enum awemgr_rc rc;

    rc = awemgr_init(NULL, &mgr_p);
    if(rc != awemgr_RC_OK)
    {
        printf("awemgr_init returned %d", rc);
    }

    int endpoint = 0;
    char *awc_file = TEST_DATA_DIR "/designs/set_get/target_files/awc_index.txt";
    if (argc == 2)
    {
        awc_file = argv[1];
    }
    rc = awemgr_load_awc(mgr_p, awc_file, endpoint);

    struct awemgr_ctx *ctx_p = awemgr_get_awc_context(mgr_p, endpoint);

    awemgr_targetinfo info_buffer;
    rc = awemgr_get_target_info(mgr_p, &info_buffer);

    int nr_designs = awemgr_get_design_count(ctx_p);
    for (int x=0; x<nr_designs; x++)
    {
        struct awemgr_design_info info;
        enum awemgr_rc rc = awemgr_get_design_info(ctx_p, x, &info);
        printf("AWC has design: %s\n", info.name);
    }

    // disable to attach to running AWB via AWE-Manager -
    // just like in Designer "attach to running target" vs "play"
    rc = awemgr_load_design(ctx_p, "Main");

    struct awemgr_ctl_elem_info info;
    rc = awemgr_get_control_info_by_name(ctx_p, "SourceInt_1.value", &info);

    int value = 123;
    rc = awemgr_control_write(ctx_p, info.id.name, 0, (void*) &value, 1, info.id.type);

    // enable the 2 lines below for reading from a "downstream" module,
    // instead of simply getting back the value written into the SourceInt module
    //
    // however, this requires that AWE core needs time to pump the signal flow "a bit"
    // a short nap (sleep) is to ensure the awemgr_control_read() call will return
    // the value that was "downstreamed" to the SinkInt module.
    //
    // rc = awemgr_get_control_info_by_name(ctx_p, "SinkInt_1.value", &info);
    // usleep(1000);

    unsigned int nr_words_in_buf;
    enum awemgr_vartype typ;
    int value_rx;
    rc = awemgr_control_read(ctx_p, info.id.name, &value_rx, 1, &nr_words_in_buf, &typ);
    printf("%s has value %d now\n", info.id.name, value_rx);

    rc = awemgr_exit(&mgr_p);

    return 0;
}
