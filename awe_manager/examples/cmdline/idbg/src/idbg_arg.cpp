#include "idbg.h"
#include "idbg_util.h"
#include "awosal_string.h"

#include <string.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * routine to check if arguments passed to a command are valid,
 * i.e. if all parameters are known
 * \param pI      IN: idbg handle
 * \param argc    IN: number of arguments passed to command
 * \param argv    IN: array of argument strings passed to command
 * \return        0: all parameters are known
 * 			 -1: unknown parameter found; error message is printed
 *
 */
int  idbg_arg_error (idbg_t *pI, int argc, char **argv)
{
	// check if every argument is known
	bool is_known = pI->used_parameters.parse(argc, argv);
	if (!is_known) {
		pI->argPassError = TRUE;
		snprintf(pI->argPassErrorStr, sizeof(pI->argPassErrorStr),
			"error: unknown parameter found.");
	}

	int last = pI->argPassError;
	if (last && pI->argPassErrorStr[0]) {
		idbg_print (pI, "%s\n", pI->argPassErrorStr);
	}
	pI->argPassError = FALSE;
	return last;
}

int idbg_arg_flag (idbg_t *pI, int argc, char **argv, char *key_p)
{
	int i;
	// NOT_USED (pI);

	pI->used_parameters.add(key_p);

	for (i=1; i < argc; i++)
		if (strcmp(argv[i], key_p) == 0)
			return 1;
	return 0;
}

char * idbg_arg_char (idbg_t *pI, int   argc, char **argv,
					  char *key_p, char *default_p,
					  bool bNeeded)
{
	pI->used_parameters.add(key_p);

	int i;
	for (i=1; i < argc; i++)
		if (strcmp(argv[i], key_p) == 0) {
			if ((i+1) == argc) {
				if (bNeeded == TRUE) {
					pI->argPassError = TRUE;
					snprintf(pI->argPassErrorStr, sizeof(pI->argPassErrorStr),
						"error: value for parameter '%s' is missing", key_p);
				}

	    		return NULL;
			}
		pI->used_parameters.addValue(argv[i+1]);
		return argv[i+1];
	}

	if (bNeeded == ARG_NEEDED) {
		pI->argPassError = TRUE;
		snprintf(pI->argPassErrorStr, sizeof(pI->argPassErrorStr),
			"error: argument '%s' needed", key_p);
	}
	return default_p;
}

int    idbg_arg_int  (idbg_t *pI, int argc, char **argv, char *key_p, int defaultval, bool bNeeded)
{
	pI->used_parameters.add(key_p);

	int i;

	for (i=1; i < argc; i++)
		if (strcmp(argv[i], key_p) == 0) {
			if (argv[i+1]) {
				char *endptr;
				pI->used_parameters.addValue(argv[i+1]);
				return strtol(argv[i+1], &endptr, 0);
			}

			pI->argPassError = TRUE;
			snprintf(pI->argPassErrorStr, sizeof(pI->argPassErrorStr),
				"error: value for '%s' is missing", argv[i]);
		}

	if (bNeeded == ARG_NEEDED) {
		pI->argPassError = TRUE;
		snprintf(pI->argPassErrorStr, sizeof(pI->argPassErrorStr),
			"error: argument '%s' is missing", key_p);
	}

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
