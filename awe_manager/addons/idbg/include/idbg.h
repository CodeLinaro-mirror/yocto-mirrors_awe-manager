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


#ifndef INCLUSION_GUARD_IDBG_H
#define INCLUSION_GUARD_IDBG_H

#include <stdarg.h>

typedef struct idbg_st           idbg_t;
typedef struct idbgtableentry_st idbgtableentry_t;

typedef int (cb_func) (idbg_t*,int,char**);
typedef void (idbglib_print_func) (idbg_t*,const char *, va_list);

struct idbgtableentry_st {
  int      type;       /**< type of this entry: SUB_DIR, CMD, VAR, ... */
  char    *pchName;    /**< name of entry */
  void    *var_addr_p; /**< address of a variable */
  char    *pchDesc;    /**< short description of that command */
  cb_func *pFunc;      /**< ptr to execution function */
};

int idbg_init (idbg_t **this_pp, idbgtableentry_t *starttbl_p);
int idbg_exit (idbg_t **this_pp);

int idbg_parse_cmd (idbg_t *this_p, unsigned char *cmdbuf_p, int cmdbuf_sz);

// arg and output related methods

int idbg_print     (idbg_t *this_p, const char *fmt, ...);

/**
 * Print bypassing an active idbg_output_hold(), i.e. always straight to the
 * real output sink (stdout, socket, ...).
 *
 * Use this for asynchronous output that is produced *while* a command is
 * running and that must not be folded into the command's own output block -
 * for example the comm-trace tap of the AWEMgr-Shell, which is called from
 * inside the AWE Manager API calls a command performs.
 *
 * Without a hold in place this behaves exactly like idbg_print().
 */
int idbg_print_direct (idbg_t *this_p, const char *fmt, ...);

/**
 * Start collecting idbg_print() output in an internal buffer instead of
 * writing it to the output sink.
 *
 * A command uses this to keep its output contiguous even when unrelated
 * output (comm traces) is emitted from within the API calls it makes.
 * Every call must be paired with idbg_output_flush(); calls nest, only the
 * outermost flush emits the collected output.
 *
 * Note: the buffer is per idbg handle and not protected against concurrent
 * access. Only the thread executing the command may hold/flush output; other
 * threads shall use idbg_print_direct().
 *
 * @return 0 on success
 */
int idbg_output_hold  (idbg_t *this_p);

/**
 * Counterpart of idbg_output_hold(). The outermost flush restores the output
 * sink and writes the collected output to it, without anything else being
 * able to appear in between. Output sinks format into fixed size buffers, so
 * the collected block is handed over in chunks that fit into them.
 *
 * @return 0 on success, -1 when no hold is active
 */
int idbg_output_flush (idbg_t *this_p);

/**
 * Drop all idbg_print() output instead of writing it to the output sink.
 *
 * A command uses this while it executes something whose output would flood
 * the console or socket, e.g. the command repeated by "repeat".
 * Every call must be paired with idbg_output_enable(); calls nest, only the
 * outermost enable puts the output sink back in place.
 *
 * @return 0 on success
 */
int idbg_output_disable(idbg_t *this_p);

/**
 * Counterpart of idbg_output_disable(). The outermost enable re-installs the
 * output sink - stdout or socket - that was in use when the output was
 * dropped, together with its backend.
 *
 * @return 0 on success, -1 when the output is not dropped
 */
int idbg_output_enable (idbg_t *this_p);

char ** idbg_get_cmdlist  (idbg_t *this_p);
char ** idbg_get_dirlist  (idbg_t *this_p);
char * idbg_get_dirpath (idbg_t *this_p);

int    idbg_arg_int  (idbg_t *pI, int argc, char **argv, char *key_p, int defaultval, bool needed);
char * idbg_arg_char (idbg_t *pI, int argc, char **argv, char *key_p, char *default_p, bool needed);
int    idbg_arg_flag (idbg_t *pI, int argc, char **argv, char *key_p);
void   idbg_arg_showhelp (idbg_t *pI, const char *headline_p, ...);
int    idbg_arg_error (idbg_t *pI, int argc, char **argv);

// user data handling
void  idbg_set_userdata (idbg_t *this_p, void *data_p);
void *idbg_get_userdata (idbg_t *this_p);

// print fct settings
void  idbg_set_printfct (idbg_t *this_p, idbglib_print_func *fct_p, void* backend_p);
void  idbg_reset_printfct (idbg_t *this_p);
void* idbg_get_printfct_backend(idbg_t *this_p);

/**
 * Get the output sink currently installed on the handle.
 *
 * Counterpart of idbg_set_printfct(): while output is held, the real sink is
 * returned, i.e. the one a matching idbg_set_printfct() call would replace.
 * Together with idbg_get_printfct_backend() this allows a command to install
 * a temporary sink - e.g. to suppress the output of the commands it invokes,
 * see the "repeat" command of the AWEMgr-Shell - and to restore the previous
 * one afterwards, no matter whether output goes to stdout or to a socket.
 */
idbglib_print_func *idbg_get_printfct (idbg_t *this_p);

// creating entries dynamically
int idbg_allocate_dir(idbg_t *this_p, const char *dirname_p, int nr_entries, idbgtableentry_t **newtbl_pp);


// callback arg parse related defines

#define IDBG_PARAMS            idbg_t *p, int argc, char **argv
#define IDBG_HDL_VAR           p

/** check for -help on the command line */
#define IDBG_CHK_HELP          idbg_arg_flag (p, argc, argv, (char*)"-help") || \
  idbg_arg_flag (p, argc, argv, (char*)"-h")

/** check for any flag on the command line */
#define IDBG_CHK_FLAG(x)       idbg_arg_flag (p, argc, argv, (char*)x)

/** return a chr-pointer to the value of a certain parameter */
#define IDBG_GET_STRING(k,d,n) idbg_arg_char (p, argc, argv, (char*)k, (char*)d, n)

/** return an integer value of a certain parameter */
#define IDBG_GET_INT(k,d,n)    idbg_arg_int (p, argc, argv, (char*)k, d, n)

/** check if an error has occured during parsing the command line */
#define IDBG_ARG_ERROR         idbg_arg_error(p, argc, argv)

/** convenient macro to display help messages for a command */
#define IDBG_CMDUSAGE(x)       idbg_arg_showhelp x

#define ARG_NEEDED   1
#define ARG_OPTIONAL 0

#define IDBG_OK    0
#define IDBG_STOP   -1

/**
 * Print an error message to the idbg handle, automatically prefixed with
 * "error: " so that shell clients (e.g. awemgr_client.py) can detect and
 * render it in red.  Use this instead of bare idbg_print() for all failure
 * paths.  The format string must end with "\n".
 *
 * Example:
 *   IDBG_PRINT_ERR(IDBG_HDL_VAR, "could not load design '%s'\n", name);
 *   -> prints:  error: could not load design 'Main'
 */
#define IDBG_PRINT_ERR(p, fmt, ...) idbg_print((p), "error: " fmt, ##__VA_ARGS__)

/**
 * Same as IDBG_PRINT_ERR but with "warn: " prefix.
 * Use this for non-fatal issues that should be highlighted to the user,
 * but do not prevent the command from executing successfully.
 * The format string must end with "\n".
 */
#define IDBG_PRINT_WARN(p, fmt, ...) idbg_print((p), "warn: " fmt, ##__VA_ARGS__)

// table related defines

#define IDBG_TBL_TYPE_END         0   /**< */
#define IDBG_TBL_TYPE_SUB_DIR     '!' /**< */
#define IDBG_TBL_TYPE_CMD         'F' /**< */
#define IDBG_TBL_TYPE_VAR_HEX     'x' /**< */
#define IDBG_TBL_TYPE_VAR_UDEC    'd' /**< */
#define IDBG_TBL_TYPE_VAR_SDEC    'n' /**< */
#define IDBG_TBL_TYPE_VAR_CHAR    'c' /**< */
#define IDBG_TBL_TYPE_VAR_BIN     'b' /**< */
#define IDBG_TBL_TYPE_VAR_STR     's' /**< */
#define IDBG_TBL_TYPE_VAR_NUMARR  'A' /**< */
#define IDBG_TBL_TYPE_VAR_TXTARR  'a' /**< */
#define IDBG_TBL_TYPE_VAR_ENUM    'e' /**< */
#define IDBG_TBL_TYPE_VAR_DATAPTR 'p' /**< */

#define IDBG_NO_FNC     (cb_func*) NULL

#define IDBG_TBL_EXTERN(__tab)  extern  idbgtableentry_t __tab[];
#define IDBG_TBL_START(__tab)           idbgtableentry_t __tab[] = {

#define IDBG_TBL_END  {							\
	IDBG_TBL_TYPE_END,							\
	  NULL, NULL, NULL, IDBG_NO_FNC} };

#define IDBG_TBL_SUB_DIR(__dir,__str,__desc)  {		\
	IDBG_TBL_TYPE_SUB_DIR,							\
	  (char*)__str, (void *)(__dir), (char*)__desc,	\
	  IDBG_NO_FNC},

#define IDBG_TBL_CMD(__fnc,__str,__desc)  {			\
	IDBG_TBL_TYPE_CMD,								\
	  (char*)__str, NULL, (char*)__desc,  __fnc  },


#endif // INCLUSION_GUARD_IDBG_H