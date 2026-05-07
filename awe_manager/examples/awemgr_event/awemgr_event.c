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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <stdatomic.h>
#endif
#include <signal.h>

#include "awe_manager.h" // the AWE Manager include
#include "awe_comm.h"    // for setting configuration to AWECore comm, before AWEMgr starts
#include "awe_types.h"
#include "awemgr_logging.h"

#define MAX_EV_MODULE_CNT                      (64)
#define EV_SUB_STR                             "-sub:"
#define EV_UNSUB_STR                           "-unsub:"
#define AWE_LOG(arg,...) printf("%s: %d "  arg, __func__, __LINE__, ##__VA_ARGS__)
#define FAIL_ON_RC(rc)   if(rc != awemgr_RC_OK) {AWE_LOG("FAILED! Exiting\n"); return -1;}
#define FAIL_ON_NULL(p)  if(p == NULL) {AWE_LOG("FAILED! Exiting\n"); return -1;}
#define TIMEOUT_MS                             (60*1000)

// ******************************************************************************************************
void eventAlert(const awemgr_event* ev, void* userdata)
{
    unsigned int i;
    uint32_t *pay;

    printf("Event name:%s instanceId:(%d) objectId:(%d) classId:0x%x\n",
        (ev->module.name ? ev->module.name:"NULL"), ev->module.instanceId,
        ev->module.objectId, ev->module.classId);
    printf("eventType:%d eventCategory:%d timeStamp:%lld sizeInBytes:0x%x\n",
        ev->eventType, ev->eventCategory, (long long unsigned int)ev->timeStamp,
        ev->sizeInBytes);

    pay = (uint32_t *)ev->payload;
    for (i = 0; i < ev->sizeInBytes / sizeof(uint32_t); i++)
    {
        printf("%08x ", pay[i]);
        if ((i&7) == 7)
        {
            printf("\n");
        }
    }

    printf("\n");
}

#ifdef WIN32
bool is_exit_flag = false;
#else
atomic_bool is_exit_flag = ATOMIC_VAR_INIT(false);
#endif
void sigint_handler(int sig_num)
{
    if(sig_num == SIGINT)
    {
#ifdef WIN32
        is_exit_flag = true;
#else
        atomic_store(&is_exit_flag, true);
#endif

    }
}

// ******************************************************************************************************

int main(int argc, char* argv[])
{
    struct awemgr_data *mgr_p = NULL;   // NULL is important
    enum awemgr_rc rc;
    char *awc_file = NULL;
    int i;
    int sub_ev_cnt = 0;
    char *sub_ev_name[MAX_EV_MODULE_CNT];
    int unsub_ev_cnt = 0;
    char *unsub_ev_name[MAX_EV_MODULE_CNT];

    if (argc < 2)
    {
        printf("awemgr_event -awc:awe_filename [-ev:event] [-unev:event]\n");
        printf("  -awc:filename awc file name\n");
        printf("  %sevent_name subscribe_event_name \n", EV_SUB_STR);
        printf("  %sevent_name unsubscribe_event name\n", EV_UNSUB_STR);
        printf("For example:\n");
        printf("  awemgr_event -awc:/data/volker/target_files/test_event.txt "
            "%sevent1 %sevent2\n", EV_SUB_STR, EV_UNSUB_STR);
        exit(-1);
        return -1; // Return an error code
    }

    // Parse command line
    for (int n = 1; n < argc; n++)
    {
        printf("argc=%d, argv[%d]=%s\n", argc, n, argv[n]);
        char *sValue = argv[n];

        if (strncmp(sValue, "-awc:", 5) == 0) {
            awc_file = sValue + 5;
            AWE_LOG("AWC_FILE %s\n", awc_file);
            continue;
        }

        if ((strncmp(sValue, EV_SUB_STR, strlen(EV_SUB_STR)) == 0))
        {
            if (sub_ev_cnt < MAX_EV_MODULE_CNT)
            {
                sub_ev_name[sub_ev_cnt++] = sValue + strlen(EV_SUB_STR);
            }
            continue;
        }

        if ((strncmp(sValue, EV_UNSUB_STR, strlen(EV_UNSUB_STR)) == 0))
        {
            if (unsub_ev_cnt < MAX_EV_MODULE_CNT)
            {
                unsub_ev_name[unsub_ev_cnt++] = sValue + strlen(EV_UNSUB_STR);
            }
            continue;
        }

        AWE_LOG("Unknown command option\n");
        exit(1);
    }

    signal(SIGINT, sigint_handler);

    rc = awemgr_init(NULL, &mgr_p);

    int endpoint = 0;
    rc = awemgr_load_awc(mgr_p, awc_file, endpoint);
    FAIL_ON_RC(rc);

    struct awemgr_ctx *ctx_p = awemgr_get_awc_context(mgr_p, endpoint);
    FAIL_ON_NULL(ctx_p);

    for (i = 0; i < sub_ev_cnt; i++)
    {
        AWE_LOG("subscribe:%s\n", sub_ev_name[i]);
        rc = awemgr_enable_event(ctx_p, sub_ev_name[i]);
        FAIL_ON_RC(rc);
    }
    for (i = 0; i < unsub_ev_cnt; i++)
    {
        AWE_LOG("unsubscribe:%s\n", unsub_ev_name[i]);
        rc = awemgr_disable_event(ctx_p, unsub_ev_name[i]);
        FAIL_ON_RC(rc);
    }

    rc = awemgr_events_start(ctx_p, eventAlert, NULL);
    FAIL_ON_RC(rc);
    AWE_LOG("awemgr_events_start\n");

    while (!is_exit_flag)
    {
        awemgr_events_process_next(ctx_p, TIMEOUT_MS);
    }

    awemgr_events_stop(ctx_p);
    awemgr_exit(&mgr_p);

    return 0;
}
