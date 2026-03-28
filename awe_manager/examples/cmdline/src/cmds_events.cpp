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

#include "cmds_events.h"

#include "app_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "awe_ctrl.h"    // for setting configuration to AWECore
#include "awemgr_logging.h"  // for dumping buffer content

#include "cmds_showinfo.h"

#include <stdlib.h>
#include <stdio.h>
#include <awosal_string.h>


struct dumpctx
{
    idbg_t *p;
    FILE *fp;
    char const *prefix;
};

/* method to dump a line of data to the IDBG console */
static void print_to_idbg(char *line, int index, void *ctx)
{
    struct dumpctx *d = (struct dumpctx*) ctx;
    idbg_print(d->p, "  => %s %4d : %s\n", d->prefix, index, line);
}

/* function to dump a line of data into a trace file */
static void print_to_file(char *line, int index, void *ctx)
{
    struct dumpctx *d = (struct dumpctx*) ctx;
    fprintf(d->fp, "  => %s %4d : %s\n", d->prefix, index, line);
}

/* callback invoked by AWE-Manager upon reception of event */
static void eventReporter(const awemgr_event* ev, void* userdata)
{
    idbg_t *p = (idbg_t*) userdata;
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(p);
    if (appCtx_p->ev_reader.ev_dump_file_name) {
        FILE *f = fopen(appCtx_p->ev_reader.ev_dump_file_name, "a+");
        if (f) {
            fprintf(f, "EVT: cat/typ: %d/%d, sz: %d, mod_name: %s, mod_objid: %d\n",
                ev->eventCategory, ev->eventType, ev->sizeInBytes,
                ev->module.name ? ev->module.name : "",
                ev->module.objectId);
            fprintf(f, "EVT: payload >>DSTART\n");
            struct dumpctx c = {NULL, f, "EVT"};
            awemgr_log_buffer((void*) ev->payload, ev->sizeInBytes, AWEMGR_LOG_VARTYPE_INT, print_to_file, &c);  // hardcoded to INT
            fprintf(f, "EVT: <<DEND\n");
            fclose(f);
        }
    } else {
        idbg_print(p, "EVT: cat/typ: %d/%d, sz: %d, mod_name: %s, mod_objid: %d\n",
                ev->eventCategory, ev->eventType, ev->sizeInBytes,
                ev->module.name ? ev->module.name : "",
                ev->module.objectId);
        idbg_print(p, "EVT: payload >>DSTART\n");
        struct dumpctx c = {p, NULL, "EVT"};
        awemgr_log_buffer((void*) ev->payload, ev->sizeInBytes, AWEMGR_LOG_VARTYPE_INT, print_to_idbg, &c);  // hardcoded to INT
        idbg_print(p, "EVT: <<DEND\n");
    }
}


/*
 * grab one specific event
 */
static int _read_one_event_with_timeout(idbg_t *p)
{
    DEF_VARS_MGR_AND_CTX(p);

    enum awemgr_rc rc = awemgr_events_process_next(awc_ctx_p, 1000);
    if (rc != awemgr_RC_OK) {
        idbg_print(p, "Failed to get next event data\n");
    }
    return IDBG_OK;
}

/* "local" thread to read from AWE-Manager's event handling component */
static void* _eventReaderThread(void* data)
{
    idbg_t *p = (idbg_t*) data;
    DEF_VARS_MGR_AND_CTX(p);
    (void)awc_ctx_p; // avoid compiler warning

    while (! appCtx_p->ev_reader.stop_flag) {
        _read_one_event_with_timeout(p);
    }
    return NULL;
}

/*
 * turn on continuous event reporting - to console or to file
 */
static int _start_listening_thread(idbg_t *p)
{
    DEF_VARS_MGR_AND_CTX(p);
    (void)awc_ctx_p; // avoid compiler warning

    if (!appCtx_p->ev_reader.event_reading_thread.joinable()) {
        appCtx_p->ev_reader.event_reading_thread = std::thread(_eventReaderThread, p);
        idbg_print(p, "Reading thread started\n");
    }
    return IDBG_OK;
}


/*
 * stop reporting event data; that does NOT mean that an event module does not "fire" anymore,
 * it is just not reported (for the AWC context)
 */
static int _stop_listening_thread(idbg_t *p)
{
    DEF_VARS_MGR_AND_CTX(p);
    (void)awc_ctx_p; // avoid compiler warning

    appCtx_p->ev_reader.stop_flag = 1;
    if (appCtx_p->ev_reader.event_reading_thread.joinable())
        appCtx_p->ev_reader.event_reading_thread.join();

    idbg_print(p, "Event reading thread stopped.\n");
    return IDBG_OK;
}


/*
 * main entry point for command; analyze parameters and call sub-routines to handle specific tasks
 */
int handle_event_cmd (IDBG_PARAMS)
{
    enum awemgr_rc rc;

    DEF_VARS_MGR_AND_CTX(IDBG_HDL_VAR);
    (void)awc_ctx_p; // avoid compiler warning

    bool show_list = IDBG_CHK_FLAG("-list");
    bool switch_off = IDBG_CHK_FLAG("-report_off");
    bool switch_on = IDBG_CHK_FLAG("-report_on");
    bool get_one = IDBG_CHK_FLAG("-get");
    char *outfile_name = IDBG_GET_STRING("-file", NULL, ARG_OPTIONAL);
    char *enable = IDBG_GET_STRING("-enable", NULL, ARG_OPTIONAL);
    char *disable = IDBG_GET_STRING("-disable", NULL, ARG_OPTIONAL);

    bool no_arg_given = !show_list && !switch_off && !switch_on && !outfile_name && !enable && !disable && !get_one;

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR || no_arg_given)
    {
        IDBG_CMDUSAGE ((p, "[OPTIONS]",
                        "-list", "show all available event modules defined",
                        "-report_off", "stop reporting of events (default startup behavior; events may still be subscribed to, but they are not propagated to console or file)",
                        "-report_on", "start reporting (depending on events in signal flow this might involve lots of debug prints into the console window!)",
                        "-get", "gets one single event and displays it",
                        "-file <filename>", "Event data will be dumped into file instead onto console",
                        "-enable <modname>|<ev_type>", "start reporting events from this module or for this event type (ev_type := Integer, modname := String), this sets the runtime state of a module to ACTIVE",
                        "-disable <modname>|<ev_type>", "stop reporting, by setting runtime state of module to INACTIVE",
        				NULL, NULL));
        return IDBG_OK;
    }

    // if simply only show modules, do that and return; no further logic required
    if (show_list)
    {
        show_available_event_modules(IDBG_HDL_VAR, awc_ctx_p, "");
        return IDBG_OK;
    }

    // handle event module subscription first; this only depends on the module
    // name, not on the event communication backend
    if (enable) {
        rc = awemgr_enable_event(awc_ctx_p, enable);
        if (rc != awemgr_RC_OK) {
            idbg_print(p, "ERROR: could not subscribe to event module: %s\n", enable);
        }
    }

    if (disable) {
        rc = awemgr_disable_event(awc_ctx_p, disable);
        if (rc != awemgr_RC_OK) {
            idbg_print(p, "ERROR: could not disable event module: %s\n", disable);
        }
    }

    // if the continuous listening for an event shall be stopped, tell AWE Manager to shut
    // down the event handler; also reset our filename
    if (switch_off) {
        _stop_listening_thread(p);
        rc = awemgr_events_stop(awc_ctx_p);
        free(appCtx_p->ev_reader.ev_dump_file_name);
        appCtx_p->ev_reader.ev_dump_file_name = NULL;
        return IDBG_OK;
    }

    // if no event handler backend in AWE Manager has been started, do so...
    bool event_backend_started = awemgr_events_started(awc_ctx_p);
    if (!event_backend_started) {

        // do not continue to store data into an old file
        if (appCtx_p->ev_reader.ev_dump_file_name) {
            free(appCtx_p->ev_reader.ev_dump_file_name);
        }
        appCtx_p->ev_reader.ev_dump_file_name = NULL;

        // if a new file name is given, then create this output file (0 length);
        // remember the name for later; it's used in the AWE Manager event callback then
        if (outfile_name) {
            appCtx_p->ev_reader.ev_dump_file_name = strdup(outfile_name);
            if (!appCtx_p->ev_reader.ev_dump_file_name) {
                idbg_print(p, "Fatal. Did not get memory for a string allocation?! Bailing out.\n");
                return IDBG_STOP;
            }
            idbg_print(p, "File %s will be used to trace event data\n", appCtx_p->ev_reader.ev_dump_file_name);
            FILE *f = fopen(appCtx_p->ev_reader.ev_dump_file_name, "w");
            fclose(f);
        }

        // powers up AWE Managers event handling; events will be reported to the callback given;
        // either we read them one by one (see "-get [-file xyz]" parameter, or continuously "-on [-file xyz]")
        rc = awemgr_events_start(awc_ctx_p, eventReporter, p);
        if (rc != awemgr_RC_OK) {
            idbg_print(p, "Failed to bring up AWE-Manager event handler\n");
            return IDBG_OK;
        }
    }

    if (get_one)
        return _read_one_event_with_timeout(p);

    if (switch_on)
        return _start_listening_thread(p);

    return IDBG_OK;
}
