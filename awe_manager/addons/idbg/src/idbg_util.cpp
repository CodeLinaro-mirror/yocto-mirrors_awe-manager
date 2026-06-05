
#include "idbg_util.h"
#include <string.h>
#include <stdio.h>

#define BUFSZ_CHK(x, max, len) \
	if ((x - max) == len) { \
		fprintf(stderr, "error: internal command line buffer overflow (%d)\n", len); \
		return -1; \
	}

#define MAXARG_CHK(idx) \
	if (idx == MAX_IDBG_ARGS) { \
		fprintf(stderr, "error: maximum number of arguments (%d) reached! Parsing error!\n", MAX_IDBG_ARGS); \
		return -1; \
	}


int util_split_cmdline (uint8 *rawCmdLine_p, int lineLength, int *argc_ptr, char **argv_ptrs)
{
    uint8 *str_p = rawCmdLine_p;
    const uint8 *beginstr_p;
    Bool  foundDblQuote  = FALSE;
    Bool  foundSpace     = FALSE;
    Bool  foundBackSlash = FALSE;
    int   index;

	if (! rawCmdLine_p)
		return -1;

	// skip for next word
	while( *str_p != 0 && ( *str_p == ' ' || *str_p == '\t') ) {
	    str_p++;
		if ((str_p - rawCmdLine_p) == (lineLength-1)) {
			fprintf(stderr, "warn: empty line with size (%d)\n", lineLength); // warning here only!
			return 0;
		}
	}

	// strip everything behind starting comment marker; don't rely on underlying parsing lib
	// makes parsing here a bit faster
	if (*str_p == '#') {
		*str_p = '\0';
		return 0;
	}

	index = 0;
	beginstr_p = str_p;
	while( *str_p != 0 ) {

		if( *str_p == '"' || *str_p == '\'' )
	    {
	    	// also remove DblQuote from argument
	    	if (!foundDblQuote)
	    		beginstr_p = str_p + 1;
	    	else
	    		*str_p = 0;

			foundDblQuote = !foundDblQuote;
	    } else {
			foundSpace = (*str_p == ' ');
			foundBackSlash = (*str_p == '\\');
			if( !foundSpace && !foundBackSlash )
			{
				str_p++;
				BUFSZ_CHK(str_p, rawCmdLine_p, (lineLength-1));
				continue; // ## CONTINUE ## while
			}
	    }

		if( foundSpace )
		{
			foundSpace = FALSE;   // Clear the flag
			if( !foundDblQuote )
			{
				// Skip duplicate SPACEs, not in a string !
				while( *str_p != 0 && *str_p == ' ' ) {
					*str_p++ = 0;
					BUFSZ_CHK(str_p, rawCmdLine_p, (lineLength-1));
				}

				// Found new argument or end of string
				if( *str_p != 0 )
				{
					// New argument found
					MAXARG_CHK(index);
					argv_ptrs[index++] = (char*) beginstr_p;
					beginstr_p = str_p;
				}
			}
			else
			{
				// Found 'something' within "" don't touch it
				str_p++;
				BUFSZ_CHK(str_p, rawCmdLine_p, (lineLength-1));
			}
			continue; // ## CONTINUE ## while
		}

		str_p++;
		BUFSZ_CHK(str_p, rawCmdLine_p, (lineLength-1));
	}
	if (str_p != beginstr_p) {
		MAXARG_CHK(index);
		argv_ptrs[index++] = (char*) beginstr_p;
	}

	*argc_ptr = index;

#if 0
	IDBGLIB_C_(
		IDBG_PRINT ("splitcmdline: argc = %d\n", *argc_ptr);
		for (index = 0; index < *argc_ptr; index++) {
			IDBG_PRINT("splitcmdline: argv[%d] = >%s<\n",
						index, argv_ptrs[index]);
		}
	);
#endif
	return 0;
}

int util_find_command(idbgtableentry_t *tbl_p, const char *cmd_p, int *row_p, uint8 *type_p)
{
	int i;

	if (!tbl_p)
		return -1;

	for (i = 0; tbl_p[i].pchName; i++)
    {
		if (strcmp (tbl_p[i].pchName, cmd_p) == 0) {
			// found command
			*row_p = i;
			*type_p = tbl_p[i].type;
			return 0;
		}
	}
	return -1;
}

struct typedesc_st {
	uint8 type;
	const char *name_p;
};

struct typedesc_st  typedesc_table[] =
{
	{ IDBG_TBL_TYPE_SUB_DIR,  "dir" },
	{ IDBG_TBL_TYPE_CMD,      "cmd" },
	{ IDBG_TBL_TYPE_VAR_HEX,  "hex" },
	{ IDBG_TBL_TYPE_VAR_UDEC, "udec" },
	{ IDBG_TBL_TYPE_VAR_SDEC, "sdec" },
	{ IDBG_TBL_TYPE_VAR_CHAR, "char" },
	{ IDBG_TBL_TYPE_VAR_BIN,  "bin" },
	{ IDBG_TBL_TYPE_VAR_STR,  "str" },
	{ IDBG_TBL_TYPE_VAR_DATAPTR, "ptr" },
	{ IDBG_TBL_TYPE_END, NULL },
};
// *****************************************************************************
char *util_type_2_string (uint8 type)
{
	int i;
	for (i=0; typedesc_table[i].name_p; i++)
		if ( typedesc_table[i].type == type)
		  return (char*) typedesc_table[i].name_p;
	return NULL;
}