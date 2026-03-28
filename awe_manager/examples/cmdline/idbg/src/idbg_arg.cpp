#include "idbg.h"
#include "idbg_util.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int  idbg_arg_error (idbg_t *pI)
{
	int last = pI->argPassError;
	pI->argPassError = FALSE;
	return last;
}

int idbg_arg_flag (idbg_t *pI, int argc, char **argv, char *key_p)
{
	int i;
	// NOT_USED (pI);

	for (i=1; i < argc; i++)
		if (strcmp(argv[i], key_p) == 0) 
			return 1;
	return 0;
}

char * idbg_arg_char (idbg_t *pI, int   argc, char **argv, 
					  char *key_p, char *default_p,
					  bool bNeeded)
{
	int i;
	for (i=1; i < argc; i++)
		if (strcmp(argv[i], key_p) == 0) {
			if ((i+1) == argc) {
				/* 
				IDBGLIB_C_(IDBG_ERR ("Syntax error: "
									"argument for %s is missing!!!\n", 
								   key_p);)
								   */
				if (bNeeded == TRUE)
					pI->argPassError = TRUE;
	    		return NULL;
			}
		return argv[i+1];
	}

	if (bNeeded == ARG_NEEDED) {
		pI->argPassError = TRUE;
		// IDBGLIB_C_(IDBG_ERR ("Syntax error: must have %s argument!!!\n",  key_p););
	}
	return default_p;
}

int    idbg_arg_int  (idbg_t *pI, int argc, char **argv, char *key_p, int defaultval, bool bNeeded)
{
	int i;

	for (i=1; i < argc; i++)
		if (strcmp(argv[i], key_p) == 0) {
			if (argv[i+1]) {
				char *endptr;
				return strtol(argv[i+1], &endptr, 0);
			}

			// todo: show syntax error : argument for %d is missing!!!\n",  like above!
			pI->argPassError = TRUE;
		}
        
	if (bNeeded == ARG_NEEDED) {
		pI->argPassError = TRUE;
		/*
		IDBGLIB_C_(IDBG_ERR ("Syntax error: must have %s argument. "
								"Try help for that command.\n", key_p););
								*/
	}

	//    _D (VERB_ERR, "Syntax error: must have %s argument. "
	//            "Try help for that command.\n", pchArg);

	return defaultval;
}

void idbg_arg_showhelp (idbg_t *pI, const char *headline_p, ...)
{
	va_list  ap;
	char *k, *v;

	idbg_print (pI, "Use: %s %s\n", pI->argv_ptrs[0], headline_p);

	va_start(ap, headline_p);
	do {

		k = va_arg(ap, char *);
		v = va_arg(ap, char *);
		if (k && v) {
			idbg_print (pI, "  %-20s - %s\n", k, v);
		} else if (v) {
			idbg_print (pI, "  %-20s   %s\n", "", v);
		} else if (k) {
			idbg_print (pI, "  %-20s\n", k);
		}
	} while ( k || v);
	va_end(ap);

	pI->argPassError = FALSE;  // clear possibly error flags
}
