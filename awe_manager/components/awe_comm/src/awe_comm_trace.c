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


#include "awe_comm_trace.h"

#include "awosal_string.h"
#include "awe_comm_logging.h"
// important: next include required for the AWEMGR_LOG macro used in the trace callback; we want to use the main manager logging system for comm trace output
// even though it is included by awe_comm_logging.h as well, we need to include it explicitly here to make sure the AWEMGR_LOG macro is available in this file for the trace callback implementation
#include "awemgr_logging.h"
#include "awe_cmd.h"


#define FAIL_ON_PTR(x) if (!x) { AWE_COMM_LOGE("Invalid argument: %s == NULL!", #x); return -1; }


/* ****************************************************************************
 * LOCAL TYPES
 * ***************************************************************************/

struct dumpctx
{
    int chn;
    char const *prefix;
};

/* ****************************************************************************
 * LOCAL FUNCTIONS
 * ***************************************************************************/
// helper function to log a line of trace output with the channel and direction prefix
static void trace_callback(char *line, int index, void *ctx)
{
    struct dumpctx *d = (struct dumpctx*) ctx;
    AWEMGR_LOG("[chn:%d] %s: %4d : %s", d->chn, d->prefix, index, line);
}

// callback method to be called when the trace flag config is changed;
// it will update the do_trace field in the trace config struct
static void trace_flag_changed(const char* key, const char* value, const char* description, void* context)
{
    struct awecomm_trace* this = (struct awecomm_trace*) context;
    aweconfig_get_as_bool(this->cfg_p, CFG_COMM_TRACE_STATE, &this->do_trace);
    AWE_COMM_LOGI("Trace flag changed: %s = %s, do_trace=%d", key, value, this->do_trace);
}

// callback method to be called when the trace file config is changed;
// it will open/close the trace file as needed and update the file handle in the trace config struct
static void trace_file_changed(const char* key, const char* value, const char* description, void* context)
{
    struct awecomm_trace* this = (struct awecomm_trace*) context;

    const char* file_name = aweconfig_get(this->cfg_p, CFG_COMM_TRACE_FILE, NULL);

    /* if the file name in the config is not "none" and the file handle is NULL,
       then open the file and update the file handle;
       if the file name is "none" now and the file handle is not NULL,
       then close the file and set the file handle to NULL.
     */
    if ((strcmp(file_name, CFG_COMM_TRACE_FILE_NONE) != 0) && this->comm_tx_fp == NULL)
    {
        AWE_COMM_LOGW("Open file %s for dumping TX control AWB data...", file_name);
        this->comm_tx_fp = fopen(file_name, "wb");
        if (this->comm_tx_fp == NULL)
        {
            AWE_COMM_LOGE("Error opening trace dump file %s for writing. File write disabled from now on!", file_name);
        }
    }
    else if ((strcmp(file_name, CFG_COMM_TRACE_FILE_NONE) == 0) && this->comm_tx_fp != NULL)
    {
        AWE_COMM_LOGW("Close dump file");
        fclose(this->comm_tx_fp);
        this->comm_tx_fp = NULL;
    }

    AWE_COMM_LOGI("Trace file config changed: %s = %s", key, value);

}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int awecomm_trace_register_configs(awe_config* cfg_p)
{
    aweconfig_init_tuple common_configs [] = {
        {CFG_COMM_TRACE_STATE, CFG_COMM_TRACE_STATE_VAL_DEFAULT, "Trace control data (on/off)", NULL, NULL},
        {CFG_COMM_TRACE_FILE, CFG_COMM_TRACE_FILE_VAL_DEFAULT, "Name of file to dump binary TX control data ('~' for no dump or file path)", NULL, NULL},
    };

    int rc = aweconfig_add_multiple(cfg_p, common_configs, sizeof(common_configs)/sizeof(common_configs[0]));

    return (rc != AWECFG_RC_OK) ? -1 : 0;
}

int awecomm_trace_init(struct awecomm_trace *trace_cfg_p, awe_config* cfg_p)
{
    FAIL_ON_PTR(trace_cfg_p);
    trace_cfg_p->cfg_p = cfg_p;
    trace_cfg_p->comm_tx_fp = NULL;

    trace_cfg_p->observer_cb_mutex = awosal_create_mutex();
    if (trace_cfg_p->observer_cb_mutex == NULL)
    {
        AWE_COMM_LOGE("Failed to create callback protection mutex");
        return -1;
    }

    // we can ignore the return values, add_listener can only return error if the key is not found,
    // but we know the keys are there since we just added them in register_configs
    (void) aweconfig_add_listener(cfg_p, CFG_COMM_TRACE_STATE, trace_flag_changed, trace_cfg_p);
    (void) aweconfig_add_listener(cfg_p, CFG_COMM_TRACE_FILE, trace_file_changed, trace_cfg_p);

    return 0;
}

int awecomm_trace_exit(struct awecomm_trace *trace_cfg_p)
{
    FAIL_ON_PTR(trace_cfg_p);

    awecomm_trace_finalize(trace_cfg_p);

    (void) aweconfig_remove_listener(trace_cfg_p->cfg_p, CFG_COMM_TRACE_STATE);
    (void) aweconfig_remove_listener(trace_cfg_p->cfg_p, CFG_COMM_TRACE_FILE);

    return 0;
}

int awecomm_trace_dump(struct awecomm_trace *trace_cfg_p, int chn, const char *direction, FILE *fp, void* data, int data_sz_words)
{
    FAIL_ON_PTR(trace_cfg_p);

    // check if tracing is enabled for console output
    if (trace_cfg_p->do_trace)
    {
        struct dumpctx c = { .chn = chn, .prefix = direction};
        awemgr_log_buffer(data, data_sz_words * sizeof(unsigned int), AWEMGR_LOG_VARTYPE_INT, trace_callback, &c);

        // call the application comm tap if installed
        int rc = trace_cfg_p->observer_cb_mutex->lock(trace_cfg_p->observer_cb_mutex, -1);
        if (rc == AWEOSAL_RC_FAIL)
        {
            AWE_COMM_LOGE("Failed to acquire observer callback mutex. Observer callback not called.");
            return 0;
        }
        if (trace_cfg_p->tap_cb)
            trace_cfg_p->tap_cb(direction, data, data_sz_words, trace_cfg_p->tap_ctx);
        trace_cfg_p->observer_cb_mutex->unlock(trace_cfg_p->observer_cb_mutex);

    }
    // check if dumping to file is enabled
    if (trace_cfg_p->comm_tx_fp)
    {
        int nr_words_to_write = AWECMD_MSG_NO_CRC_LENGTH(data);
        fwrite(data, nr_words_to_write, sizeof(unsigned int), trace_cfg_p->comm_tx_fp);
    }
    return 0;
}

void awecomm_trace_set_observer(struct awecomm_trace *trace_cfg_p, awecomm_buf_observer_cb cb, void *ctx)
{
    if (!trace_cfg_p)
        return;
    int rc = trace_cfg_p->observer_cb_mutex->lock(trace_cfg_p->observer_cb_mutex, -1);
    if (rc == AWEOSAL_RC_FAIL)
    {
        AWE_COMM_LOGE("Failed to acquire observer callback mutex. Callback not set.");
        return;
    }

    trace_cfg_p->tap_ctx = ctx;
    trace_cfg_p->tap_cb  = cb;

    trace_cfg_p->observer_cb_mutex->unlock(trace_cfg_p->observer_cb_mutex);
}

int awecomm_trace_finalize(struct awecomm_trace *trace_cfg_p)
{
    FAIL_ON_PTR(trace_cfg_p);

    if (trace_cfg_p->comm_tx_fp) {
        fclose(trace_cfg_p->comm_tx_fp);
        trace_cfg_p->comm_tx_fp = NULL;
    }
    return 0;
}