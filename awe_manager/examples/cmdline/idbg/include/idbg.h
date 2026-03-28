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