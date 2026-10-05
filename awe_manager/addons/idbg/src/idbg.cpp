
#include "idbg.h"
#include "idbg_types.h"
#include "idbg_util.h"
#include "idbg_internalcmds.h"

#include "awosal_string.h"

#include <stddef.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <stdio.h>


idbg_t   g_idbghdl;

/* Buffer used to format a single idbg_print() into the capture buffer; longer
 * output is formatted into an exactly fitting temporary instead. */
#define IDBG_CAPTURE_FMT_SIZE  1024

/* Portion of collected output handed to the output sink per call. Output sinks
 * format into fixed size buffers - si_writef_va() uses MAX_LINE_LENGTH (1024),
 * the Windows branch of the shell's socket sink uses 4096 - so the collected
 * block is emitted in chunks that fit into any of them. Splitting is safe: the
 * sinks write to a byte stream. */
#define IDBG_FLUSH_CHUNK_SIZE  512

/* ****************************************************************************
 * LOCAL FUNCTIONS
 * ***************************************************************************/

static void v_idbg_print (idbg_t *p, const char *fmt, va_list ap)
{
    (void)p; /* var not used; to avoid compiler warning */

    vprintf(fmt, ap);
    fflush(stdout);
}

/* Output sink installed while idbg_output_hold() is active: appends to the
 * handle's capture buffer instead of writing to stdout/socket. */
static void v_idbg_capture (idbg_t *p, const char *fmt, va_list ap)
{
    char    buf[IDBG_CAPTURE_FMT_SIZE];
    va_list ap_retry;

    va_copy (ap_retry, ap);

    int n = vsnprintf (buf, sizeof(buf), fmt, ap);
    if (n < 0)
    {
        /* encoding error - nothing sensible to collect */
    }
    else if ((size_t)n < sizeof(buf))
    {
        p->capture.append (buf, (size_t)n);
    }
    else
    {
        /* output longer than the stack buffer: format again into an exact fit */
        std::string big ((size_t)n + 1U, '\0');
        (void) vsnprintf (&big[0], big.size(), fmt, ap_retry);
        p->capture.append (big, 0, (size_t)n);
    }

    va_end (ap_retry);
}

/** Output sink installed functions like "repeat": drops every output. */
static void v_idbg_discard(idbg_t *p, const char *fmt, va_list ap)
{
    (void)p; (void)fmt; (void)ap;
}


static int free_dir(idbgtableentry_t *tbl_p)
{
    if (! tbl_p)
        return -1;
    // nothing to do here as no copies from AWC strings were done
    return 0;
}

static char ** get_dir_list  (idbg_t *this_p, bool bDirOnly)
{
    int i, cnt;
    idbgtableentry_t *tbl_p;

    if (!this_p)
        this_p = &g_idbghdl;

    tbl_p = this_p->currentTable_p;
    this_p->argv_ptrs[0] = NULL;

    if (! tbl_p)
        return this_p->argv_ptrs;

    cnt = 0;
    for (i = 0; tbl_p[i].pchName && (cnt < MAX_IDBG_ARGS-1) ; i++)
    {
        if (bDirOnly)
        {
            if (tbl_p[i].type == IDBG_TBL_TYPE_SUB_DIR)
                this_p->argv_ptrs[cnt++] = tbl_p[i].pchName;
        }
        else
            this_p->argv_ptrs[cnt++] = tbl_p[i].pchName;
    }

    if (!bDirOnly) {
        tbl_p = (idbgtableentry_t*) InternalCmds;
        for (i = 0; tbl_p[i].pchName && (cnt < MAX_IDBG_ARGS-1) ; i++)
        {
            this_p->argv_ptrs[cnt++] = tbl_p[i].pchName;
        }
    }

    this_p->argv_ptrs[cnt] = NULL;
    if (cnt == MAX_IDBG_ARGS) {
        // IDBGLIB_C_(IDBG_ERR ("ERROR: Too many dir entries for this table\n"););
    }
    return this_p->argv_ptrs;

}

/* ****************************************************************************
 * PUBLIC FUNCTIONS
 * ***************************************************************************/

int idbg_init (idbg_t **this_pp, idbgtableentry_t *starttbl_p)
{
    idbg_t *this_p = NULL;
    if ( this_pp )
    {
        // allocate new IDBG internal memory
	    // this_p     = (idbg_t*) calloc (1, sizeof(idbg_t));
        this_p     = new idbg_t();
	    *this_pp   = this_p;
    }
    else
    {
        // use internal global struct memory
        this_p = &g_idbghdl;
    }

    if (starttbl_p)
        this_p->currentTable_p = starttbl_p;
    else
        this_p->currentTable_p = (idbgtableentry_t *) NULL;

    this_p->table_level  = 0;
    this_p->print_fct_p   = v_idbg_print;
    this_p->held_fct_p    = NULL;
    this_p->hold_depth    = 0;
    this_p->capture.clear();
    this_p->saved_fct_p     = NULL;
    this_p->saved_backend_p = NULL;
    this_p->drop_depth      = 0;

    return 0;
}

int idbg_exit (idbg_t **this_pp)
{
    if ( this_pp )
    {
        if ((*this_pp)->nr_dyn_tables)
        {
            for (int i = 0; i < (*this_pp)->nr_dyn_tables; i++)
            {
                free_dir((*this_pp)->dyn_tables[i]);
                free((*this_pp)->dyn_tables[i]);
            }
        }
        delete *this_pp; *this_pp = NULL;
        // free (*this_pp); *this_pp = NULL;
    }
    return 0;
}

int idbg_parse_cmd (idbg_t *this_p, unsigned char *cmdbuf_p, int cmdbuf_sz)
{
    int               argc;
    int               row = -1;
    uint8             type;
    idbgtableentry_t *tbl_p = NULL;
    int               s = 0;
    char             *c_p, *last_start_p;
    int               level;

    if (!this_p)
        this_p = &g_idbghdl;

    if (!*cmdbuf_p)
        return 0;

    this_p->used_parameters.clear();
    this_p->argPassError = FALSE;

    s = util_split_cmdline ((uint8*) cmdbuf_p, cmdbuf_sz, &argc, this_p->argv_ptrs);
    if (s < 0)
	    return 0;  // return when an error occured; util fct will have fprintf to stderr

    c_p          = this_p->argv_ptrs[0];
    if(!c_p)
        return 0;  // already return when found no single argument, e.g. comment line

    last_start_p = c_p;
    level        = this_p->table_level;

    tbl_p = this_p->currentTable_p;
    if (*c_p && *c_p == '/')
    {
	    if (level)
	        tbl_p = this_p->TableLevel_p[0];
	    last_start_p++;
	    c_p++;
    }

    do
    {
        /* find next '/' in command */
        while ( *c_p && (*c_p != '/'))
            c_p++;

        if (*c_p == '/')
        {

            /* found a '/' in command: get directory name and change table */

            int   row = -1;
            uint8 type;

            *c_p = '\0';

            /* change into directory 'last_start_p' */
            if (strcmp (last_start_p, "..") == 0)
            {
                if (level)
                {
                    level -= 1;
                    tbl_p  = this_p->TableLevel_p[level];
                }
            } else if (util_find_command (tbl_p, last_start_p, &row, &type) == 0)
            {
                if (type == IDBG_TBL_TYPE_SUB_DIR)
                {
                    tbl_p = (idbgtableentry_t*) tbl_p[row].var_addr_p;
                }
            }

            *c_p++ = '/';

            last_start_p = c_p;
        } else
            break;

    }
    while (1);

    // try to find "command", i.e. argv[0], in table
    if (util_find_command (tbl_p, last_start_p, &row, &type) == 0)
    {
        //     IDBGLIB_C_(IDBG_DBG ("  -> User command. Do s.th !\n"););
        ;

    } else if (last_start_p == this_p->argv_ptrs[0])
    {

        /* check here only when there was no path specified */

        // the command could not be found in the actual table; it may be special:
        // search for internal command
        if (util_find_command ((idbgtableentry_t*) InternalCmds, this_p->argv_ptrs[0], &row, &type) == 0) {
            // IDBGLIB_C_( IDBG_DBG ("  -> Internal command. Do s.th !\n"); );
            tbl_p = (idbgtableentry_t*) InternalCmds;
        }
    }

    // if a table entry has been found the the variable row points to its
    // index (and not to -1 anymore); act, depending on the table entry type
    if (row >= 0) {

        switch (type) {

            // an ordinary command, i.e. locate the callback function and
            // execute it
            case IDBG_TBL_TYPE_CMD:
                // IDBGLIB_C_( IDBG_DBG ("  -> calling fct for: %s\n", tbl_p[row].pchName););
                s = tbl_p[row].pFunc (this_p, argc, this_p->argv_ptrs);
                break;

            // the selected entry is a sub-directory (i.e. another table),
            // adjust the current-Table pointer
            case IDBG_TBL_TYPE_SUB_DIR:
                if (last_start_p == this_p->argv_ptrs[0]) {
                    // IDBGLIB_C_( IDBG_DBG ("  -> enter sub-dir: %s\n", tbl_p[row].pchName); );
                    if (tbl_p[row].pFunc)
                    {
                        s = tbl_p[row].pFunc(this_p, argc, this_p->argv_ptrs);
                    }
                    this_p->TableLevel_p[this_p->table_level++] = tbl_p;
                    this_p->currentTable_p = (idbgtableentry_t*) tbl_p[row].var_addr_p;
                }
                break;

            // an unknown table entry type !
            // we should actually never get here
            default:
                IDBG_PRINT_ERR(this_p,"Unsupported command type (%d). Check implementation!\n", type);
                break;
        }
    } else {
        snprintf(this_p->argPassErrorStr, sizeof(this_p->argPassErrorStr),
            "Unknown command '%s'. Try 'help'", this_p->argv_ptrs[0]);
        IDBG_PRINT_ERR(this_p, "%s\n", this_p->argPassErrorStr);
    }

    // return what the callback function has returned
    return s;
}

int idbg_print (idbg_t *this_p, const char *fmt, ...)
{
    va_list ap;

    if (!this_p)
        this_p = &g_idbghdl;

    va_start (ap, fmt);

    if (this_p->print_fct_p)
        this_p->print_fct_p (this_p, fmt, ap);

    va_end (ap);

    return 0;
}

int idbg_print_direct (idbg_t *this_p, const char *fmt, ...)
{
    va_list ap;

    if (!this_p)
        this_p = &g_idbghdl;

    /* While output is held, print_fct_p is the capture sink and the real sink
     * is kept in held_fct_p. print_fd_p is never touched by the hold, so the
     * real sink still finds its backend (e.g. the socket fd). */
    idbglib_print_func *fct_p = (this_p->hold_depth > 0) ? this_p->held_fct_p
                                                         : this_p->print_fct_p;

    va_start (ap, fmt);

    if (fct_p)
        fct_p (this_p, fmt, ap);

    va_end (ap);

    return 0;
}

int idbg_output_hold (idbg_t *this_p)
{
    if (!this_p)
        this_p = &g_idbghdl;

    if (this_p->hold_depth == 0)
    {
        this_p->held_fct_p  = this_p->print_fct_p;
        this_p->print_fct_p = v_idbg_capture;
        this_p->capture.clear();
    }
    this_p->hold_depth++;

    return 0;
}

int idbg_output_flush (idbg_t *this_p)
{
    if (!this_p)
        this_p = &g_idbghdl;

    if (this_p->hold_depth == 0)
        return -1; /* flush without a matching hold */

    this_p->hold_depth--;
    if (this_p->hold_depth > 0)
        return 0; /* inner scope - keep collecting */

    this_p->print_fct_p = this_p->held_fct_p;
    this_p->held_fct_p  = NULL;

    if (!this_p->capture.empty())
    {
        /* hand the buffer over before printing: the sink may print again */
        std::string out;
        out.swap (this_p->capture);

        for (size_t pos = 0; pos < out.size(); pos += IDBG_FLUSH_CHUNK_SIZE)
        {
            size_t left = out.size() - pos;
            int    len  = (int) ((left < IDBG_FLUSH_CHUNK_SIZE) ? left
                                                                : (size_t)IDBG_FLUSH_CHUNK_SIZE);
            (void) idbg_print (this_p, "%.*s", len, out.c_str() + pos);
        }
    }

    return 0;
}

int idbg_output_disable (idbg_t *this_p)
{
    if (!this_p)
        this_p = &g_idbghdl;

    if (this_p->drop_depth == 0)
    {
        /* the sink currently receiving idbg_print(): while output is held that
         * is the capture sink, so a drop inside a hold discards what is
         * printed while it is active and leaves the collected block intact */
        this_p->saved_fct_p     = this_p->print_fct_p;
        this_p->saved_backend_p = this_p->print_fd_p;
        this_p->print_fct_p     = v_idbg_discard;
    }
    this_p->drop_depth++;

    return 0;
}

int idbg_output_enable (idbg_t *this_p)
{
    if (!this_p)
        this_p = &g_idbghdl;

    if (this_p->drop_depth == 0)
        return -1; /* enable without a matching disable */

    this_p->drop_depth--;
    if (this_p->drop_depth > 0)
        return 0; /* inner scope - keep dropping */

    this_p->print_fct_p     = this_p->saved_fct_p;
    this_p->print_fd_p      = this_p->saved_backend_p;
    this_p->saved_fct_p     = NULL;
    this_p->saved_backend_p = NULL;

    return 0;
}


void  idbg_set_userdata (idbg_t *this_p, void *data_p)
{
	if (!this_p)
		this_p = &g_idbghdl;

	this_p->userdata_p = data_p;
}

void *idbg_get_userdata (idbg_t *this_p)
{
	if (!this_p)
		this_p = &g_idbghdl;

	return this_p->userdata_p;
}

char ** idbg_get_dirlist  (idbg_t *this_p)
{
    return get_dir_list(this_p, true);
}

char ** idbg_get_cmdlist  (idbg_t *this_p)
{
    return get_dir_list(this_p, false);
}

char * idbg_get_dirpath(idbg_t *this_p)
{
	char *dir_stack[MAX_IDBG_DIR_LEVEL] = { 0 };
	char **dir_stack_p = &(dir_stack[0]);

	idbgtableentry_t *current_tbl_p = this_p->currentTable_p;
	int lvl = this_p->table_level;

    // loop while we are in a "sub-directory"
 	while (lvl--)
	{
        // lvl--;

		idbgtableentry_t *tbl_2_p = this_p->TableLevel_p[lvl];
        // check every entry in the "level above" if its var_add_p points to the
        // current (start) table entry
		for (int i = 0; tbl_2_p[i].pchName; i++) {
			if (tbl_2_p[i].var_addr_p == current_tbl_p)
			{
				*dir_stack_p++ = tbl_2_p[i].pchName;
				current_tbl_p = tbl_2_p;
				break;
			}
		}
	}
    // the directory stack has been filled,
    // revert back to start of this array from current position, and concatenate
    // the directory names found on the way
	lvl = this_p->table_level;
    std::string path = "/";
	while (dir_stack_p-- != &(dir_stack[0]))
	{
		// dir_stack_p--;
		lvl--;
        path += *dir_stack_p;
        if (lvl != 0) {
            path += "/";
        }
	}
    strlcpy(this_p->dir_path, path.c_str(), sizeof(this_p->dir_path));
    this_p->dir_path[sizeof(this_p->dir_path) - 1] = '\0';
	return this_p->dir_path;
}

int idbg_allocate_dir(idbg_t *this_p, const char *dirname_p, int nr_entries, idbgtableentry_t **newtbl_pp)
{
    idbgtableentry_t *tbl_p = this_p->currentTable_p;
    int               row = -1;
    uint8             type;

    if (! newtbl_pp)
        return -1;

    if (util_find_command (tbl_p, dirname_p, &row, &type) != 0)
    {
        return -1;
    }

#if 0
    if (tbl_p[row].var_addr_p)
    {
        idbgtableentry_t *p = (idbgtableentry_t*) tbl_p[row].var_addr_p;
        if (p && p->pchName)
        {
            free(tbl_p[row].var_addr_p);
            tbl_p[row].var_addr_p = NULL;
        }
    }
#endif

    idbgtableentry_t *p = (idbgtableentry_t *)calloc(nr_entries + 1, sizeof(idbgtableentry_t));
    tbl_p[row].var_addr_p = p;
    // todo: size of array check!
    // todo: check mem alloc fail!

    this_p->dyn_tables[this_p->nr_dyn_tables++] = p;

    *newtbl_pp = p;
    return 0;
}

void  idbg_set_printfct (idbg_t *this_p, idbglib_print_func *fct_p, void* backend_p)
{
	if (!this_p)
		this_p = &g_idbghdl;

    /* while output is held the capture sink must stay installed; exchange the
     * real sink behind it instead */
	if (this_p->hold_depth > 0)
		this_p->held_fct_p = fct_p;
	else
		this_p->print_fct_p = fct_p;

    this_p->print_fd_p = backend_p;
}

void idbg_reset_printfct (idbg_t *this_p)
{
	if (!this_p)
		this_p = &g_idbghdl;

	if (this_p->hold_depth > 0)
		this_p->held_fct_p = v_idbg_print;
	else
		this_p->print_fct_p = v_idbg_print;

    this_p->print_fd_p = NULL;
}

void* idbg_get_printfct_backend(idbg_t *this_p)
{
	if (!this_p)
		this_p = &g_idbghdl;

    return this_p->print_fd_p;
}

idbglib_print_func *idbg_get_printfct (idbg_t *this_p)
{
	if (!this_p)
		this_p = &g_idbghdl;

    /* mirror idbg_set_printfct(): while output is held the capture sink is
     * installed and the real sink is kept in held_fct_p */
	if (this_p->hold_depth > 0)
		return this_p->held_fct_p;

	return this_p->print_fct_p;
}