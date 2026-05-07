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

#include "cmds_events.h"

#include "shell_ctx.h"     // own application related types and interactive cmdline
#include "awe_manager.h" // the AWE Manager include
#include "awe_comm.h"    // for setting configuration to AWECore
#include "awemgr_logging.h"  // for dumping buffer content

#include "cmds_showinfo.h"
#include "hlp_dump_response.h"  // dump response
#include "hlp_functions.h" // for helper functions like IDBG_PRINT_ERR

#include <stdlib.h>
#include <stdio.h>
#include <awosal_string.h>


/* callback invoked by AWE-Manager upon reception of event,
 * if a file name is given for dumping the event data, it will be dumped to that file; otherwise, it will be printed to console
 * Note: the event data is not owned by the caller and should not be freed; if the caller needs to keep the data, it should make a copy of it; also, the event data is only valid until the callback returns, so if the caller needs to keep it, it should make a copy of it before the callback returns
 * Note: the event data is only valid until the callback returns, so if the caller needs to keep it, it should make a copy of it before the callback returns.
 */
static void eventReporter(const awemgr_event* ev, void* userdata)
{
    idbg_t *p = (idbg_t*) userdata;
    struct app_ctx_ *appCtx_p = (struct app_ctx_ *) idbg_get_userdata(p);
    if (appCtx_p->ev_reader.ev_dump_file_name) {
        FILE *f = fopen(appCtx_p->ev_reader.ev_dump_file_name, "a+");
        if (f) {
            fprintf(f, "event:\n  category: %d\n  type: %d\n  size: %d\n  module_name: %s\n  object_id: %d\n",
                ev->eventCategory, ev->eventType, ev->sizeInBytes,
                ev->module.name ? ev->module.name : "",
                ev->module.objectId);
            fprintf(f, "  payload: [\n");
            dump_response(p, "    ", (void*) ev->payload, ev->sizeInBytes, AWEMGR_VARTYPE_INTEGER, false, f);
            fprintf(f, "  ]\n");
            fclose(f);
            idbg_print(p, "status: event dumped to file '%s'\n", appCtx_p->ev_reader.ev_dump_file_name);
        }
    } else {
        idbg_print(p, "event:\n  category: %d\n  type: %d\n  size: %d\n  module_name: %s\n  object_id: %d\n",
                ev->eventCategory, ev->eventType, ev->sizeInBytes,
                ev->module.name ? ev->module.name : "",
                ev->module.objectId);
        idbg_print(p, "  payload: [\n");
        dump_response(p, "    ", (void*) ev->payload, ev->sizeInBytes, AWEMGR_VARTYPE_INTEGER, false, NULL);
        idbg_print(p, "  ]\n");
    }
}


/*
 * grab one specific event
 */
static int _read_one_event_with_timeout(idbg_t *p)
{
    DEF_VARS_MGR_AND_CTX(p);

    enum awemgr_rc rc = awemgr_events_process_next(awc_ctx_p, 1000);
    if ((rc != awemgr_RC_OK) && (rc != awemgr_RC_COMM_TIMEOUT)) {
        IDBG_PRINT_ERR(p, "failed to get next event data\n");
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
        idbg_print(p, "status: event reading thread started\n");
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

    return IDBG_OK;
}

// helper fct to make sure the event system is started and the shell inspector is registered.
// returns -1 if failed to start, 0 if already started, 1 if started by this call
static int _ensure_event_system_started(idbg_t *p)
{
    DEF_VARS_MGR_AND_CTX(p);
    (void)awc_ctx_p; // avoid compiler warning

    int result = 0;
    if (!awemgr_events_started(awc_ctx_p)) {
        enum awemgr_rc rc = awemgr_events_start(awc_ctx_p, NULL, NULL);
        if (rc != awemgr_RC_OK) {
            return -1;
        }
        result = 1;
    }
    awemgr_events_set_listener(awc_ctx_p, eventReporter, p);
    return result;
}


/*
 * main entry point for command; analyze parameters and call sub-routines to handle specific tasks
 *
 * TODO: put proper documentation here; but in short:
 *
 * event  - enables subsystem and reads one event
 * event -show  - shows all available event modules
 * event -sub <cat>  - makes actually only sense for cat!=0
 * event -file <filename> - only sets a file name to be used for output when an event is received
 * event -stop - shuts down the event subsystem
 */
int handle_event_cmd (IDBG_PARAMS)
{
    enum awemgr_rc rc;

    DEF_VARS_MGR_AND_CTX(IDBG_HDL_VAR);
    (void)awc_ctx_p; // avoid compiler warning

    bool show_list = IDBG_CHK_FLAG("-show");
    int unsubscribe_cat = IDBG_GET_INT("-unsub", -1, ARG_OPTIONAL);
    int subscribe_cat = IDBG_GET_INT("-sub", -1, ARG_OPTIONAL);
    char *outfile_name = IDBG_GET_STRING("-file", NULL, ARG_OPTIONAL);
    char *enable_module = IDBG_GET_STRING("-enable", NULL, ARG_OPTIONAL);
    char *disable_module = IDBG_GET_STRING("-disable", NULL, ARG_OPTIONAL);
    bool listen_always = IDBG_CHK_FLAG("-listen");
    bool stop_evt_system = IDBG_CHK_FLAG("-stop");

    bool no_arg_given = !show_list && !subscribe_cat && !unsubscribe_cat && !outfile_name && !enable_module && !disable_module && !stop_evt_system;

    if (IDBG_CHK_HELP || IDBG_ARG_ERROR || no_arg_given)
    {
        IDBG_CMDUSAGE ((p, "[OPTIONS] - with no option given it will enable event subsystem and read one event"
                         "OPTIONS:\n",
                        "-show", "show all available event modules defined",
                        "-listen", "starts continuous event reporting; events will be printed to console every 1 sec or dumped into file if '-file' option is given",
                        "-stop", "stops the event subsystem; no events will be reported anymore until it's started again",
                        "-sub <cat>", "subscribe to a specific event category (0-31)",
                        "-unsub <cat>", "unsubscribe from a specific event category (0-31)",
                        "-enable <modname>", "start reporting events from this module, this sets the runtime state of a module to ACTIVE",
                        "-disable <modname>", "stop reporting, by setting runtime state of module to INACTIVE",
                        "-file <filename>", "Event data will be dumped into file instead onto console",
        				NULL, NULL));
        return IDBG_OK;
    }

    // if simply only show modules, do that and return; no further logic required
    if (show_list)
    {
        show_available_event_modules(IDBG_HDL_VAR, awc_ctx_p);
        return IDBG_OK;
    }

    // handle event module subscription first; this only depends on the module name
    // it does not change callbacks or file names; just sets module to ACTIVE (or INACTIVE) state
    if (enable_module) {
        rc = awemgr_enable_event(awc_ctx_p, enable_module);
        idbg_print(p, (rc != awemgr_RC_OK) ? "error: could not enable event module: %s\n" : "status: event module '%s' enabled\n", enable_module);
        return IDBG_OK;
    }

    if (disable_module) {
        rc = awemgr_disable_event(awc_ctx_p, disable_module);
        idbg_print(p, (rc != awemgr_RC_OK) ? "error: could not disable event module: %s\n" : "status: event module '%s' disabled\n", disable_module);
        return IDBG_OK;
    }

    // if a (new) file name is given, then create this output file (0 length);
    // remember the name for later; it's used in the event callback then
    if (outfile_name && (appCtx_p->ev_reader.ev_dump_file_name == NULL)) {
        appCtx_p->ev_reader.ev_dump_file_name = strdup(outfile_name);
        if (!appCtx_p->ev_reader.ev_dump_file_name) {
            IDBG_PRINT_ERR(p, "Did not get memory for a string allocation! Bailing out.\n");
            return IDBG_STOP;
        }
        idbg_print(p, "status: file %s will be used to trace event data\n", appCtx_p->ev_reader.ev_dump_file_name);
        FILE *f = fopen(appCtx_p->ev_reader.ev_dump_file_name, "w");
        fclose(f);
        return IDBG_OK;
    }

    if (stop_evt_system) {
        awemgr_events_clear_inspector(awc_ctx_p, eventReporter);
        rc = awemgr_events_stop(awc_ctx_p);
        if (rc != awemgr_RC_OK) {
            IDBG_PRINT_ERR(p, "Could not stop event system\n");
        }
        _stop_listening_thread(p);
        // clean the dump file name, as no further data should be stored
        if (appCtx_p->ev_reader.ev_dump_file_name) {
            free(appCtx_p->ev_reader.ev_dump_file_name);
        }
        appCtx_p->ev_reader.ev_dump_file_name = NULL;
        return IDBG_OK;
    }

    if (subscribe_cat >= 0) {

        int ev_sys_just_started = _ensure_event_system_started(p);
        if (ev_sys_just_started < 0) {
            IDBG_PRINT_ERR(p, "Could not start event system\n");
            return IDBG_OK;
        }

        rc = awemgr_events_add_category_listener(awc_ctx_p, (uint32_t)subscribe_cat, eventReporter, p);
        if (rc != awemgr_RC_OK) {
            IDBG_PRINT_ERR(p, "Could not subscribe to event category: %d\n", subscribe_cat);
            return IDBG_OK;
        }

        idbg_print(p, "status: subscribed to event category %d. %s\n",
            subscribe_cat, ev_sys_just_started == 1 ? "Event system was just started by this call." : "");
        return IDBG_OK;
    }

    if (unsubscribe_cat >= 0) {

        rc = awemgr_events_remove_category_listener(awc_ctx_p, (uint32_t)unsubscribe_cat);
        if (rc != awemgr_RC_OK) {
            IDBG_PRINT_ERR(p, "Could not unsubscribe from event category: %d\n", unsubscribe_cat);
            return IDBG_OK;
        }
        // clean the dump file name, as no further data should be stored
        if (appCtx_p->ev_reader.ev_dump_file_name) {
            free(appCtx_p->ev_reader.ev_dump_file_name);
        }
        appCtx_p->ev_reader.ev_dump_file_name = NULL;
        return IDBG_OK;
    }

    // read out one single event and print it to console; this is the default if no option is given;
    // if file name was given before, the event will be dumped into the file instead
    int ev_sys_just_started = _ensure_event_system_started(p);
    if (ev_sys_just_started < 0) {
        IDBG_PRINT_ERR(p, "Could not start event system\n");
        return IDBG_OK;
    }

    if (listen_always) {
        _start_listening_thread(p);
        return IDBG_OK;
    }

    rc = awemgr_events_process_next(awc_ctx_p, 250);
    if (rc == awemgr_RC_COMM_TIMEOUT) {
        idbg_print(p, "status: No event received within 250ms. %s\n",
            ev_sys_just_started == 1 ? "Event system was just started. Prior events not reported." : "");
    } else if (rc != awemgr_RC_OK) {
        IDBG_PRINT_ERR(p, "Failed to get next event data\n");
    }

    return IDBG_OK;
}
