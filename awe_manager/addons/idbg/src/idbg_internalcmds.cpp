#include "idbg_internalcmds.h"

#include "idbg_util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#define usleep(x)       Sleep(x/1000)
#define sleep(x)        Sleep(x*1000)
#else
#include <unistd.h>
#endif

static char * construct_dir_line (const char *name_p, uint8 type, const char *desc_p)
{
	static char line[256];

	if (desc_p)
		snprintf(line, sizeof(line), " %-30s <%s>   - %s\n", name_p, util_type_2_string(type) , desc_p);
	else
		snprintf(line, sizeof(line), " %-30s <%s>\n", name_p, util_type_2_string(type));
	return line;
}

int intcmd_quit (IDBG_PARAMS)
{
    printf("quitting\n");
    return -1;
}

int intcmd_ls (IDBG_PARAMS)
{
	int i;
	idbgtableentry_t *tbl_p = p->currentTable_p;

	if (tbl_p)
	{
		for (i = 0; tbl_p[i].pchName; i++) {
			idbg_print (p, construct_dir_line (tbl_p[i].pchName, tbl_p[i].type, tbl_p[i].pchDesc) );
		}
	}
	return 0;
}

int intcmd_cd (IDBG_PARAMS)
{
	int   row = -1;
	uint8 type;

	if (argc < 2)
    {
		// this behaves like "cd ."
		return 0;
	}

	if (strcmp (argv[1], "/") == 0)
    {
		// 'cd /': go up to top-level dir
        if (p->table_level) {
            p->table_level = 0;
            p->currentTable_p = p->TableLevel_p[0];
        }
	}
	if (strcmp (argv[1], "..") == 0) {
		// 'cd /': go up one level to parent
        if (p->table_level) {
            p->table_level -= 1;
            p->currentTable_p = p->TableLevel_p[p->table_level];
        }
	}
	if (util_find_command (p->currentTable_p, argv[1], &row, &type) == 0) {
  		if (type == IDBG_TBL_TYPE_SUB_DIR) {
			if (p->currentTable_p[row].pFunc)
			{
				int s = p->currentTable_p[row].pFunc(p, argc, p->argv_ptrs);
			}
	  		p->TableLevel_p[p->table_level++] = p->currentTable_p;
	  		p->currentTable_p = (idbgtableentry_t*) p->currentTable_p[row].var_addr_p;
  		}
	}

	return 0;
}

int intcmd_cd_up (IDBG_PARAMS)
{
	// 'cd /': go up one level to parent
	if (p->table_level) {
		p->table_level -= 1;
		p->currentTable_p = p->TableLevel_p[p->table_level];
	}
	return 0;
}

int intcmd_help (IDBG_PARAMS)
{
	idbg_print (p, "Shell Commands are organized in a directory-structure.\n");
	idbg_print (p, "Client code can register callbacks for commands.\n");
	idbg_print (p, "Some basic internal commands exist:\n");
	idbg_print (p, "  * ls / dir  - show available commands on current input level\n");
	idbg_print (p, "  * pwd       - show current directory level\n");
	idbg_print (p, "  * cd <dir>  - change into (sub)directory\n");
	idbg_print (p, "  * ..        - alias for 'cd ..'\n");
	idbg_print (p, "  * sleep     - insert delay of certain number of msecs\n");
	idbg_print (p, "  * quit/exit - quit shell\n");
	idbg_print (p, "For command specific help: try <cmd> -h or <cmd> -help\n");

	return 0;
}

int intcmd_sleep(IDBG_PARAMS)
{
	if (argc < 2)
    {
		idbg_print (p, "sleep <milliseconds>\n");
		return 0;
	}

	int ms_to_sleep = atoi(argv[1]);
	if (ms_to_sleep > 0)
	{
		usleep(ms_to_sleep*1000L);
	}
	else
	{
		idbg_print(p, "Please enter positive millisecond numbers!\n");
	}

	return 0;
}

int intcmd_pwd (IDBG_PARAMS)
{
	idbg_print (p, "> %s\n", idbg_get_dirpath(p));
	return 0;
}

IDBG_TBL_START(InternalCmds)
    IDBG_TBL_CMD(intcmd_ls, "ls", NULL)
    IDBG_TBL_CMD(intcmd_ls, "dir", NULL)
    IDBG_TBL_CMD(intcmd_cd, "cd", NULL)
    IDBG_TBL_CMD(intcmd_cd_up, "..", NULL)
	IDBG_TBL_CMD(intcmd_pwd, "pwd", NULL)
    IDBG_TBL_CMD(intcmd_quit, "quit", NULL)
    IDBG_TBL_CMD(intcmd_quit, "exit", NULL)
    IDBG_TBL_CMD(intcmd_help, "help", NULL)
	IDBG_TBL_CMD(intcmd_sleep, "sleep", NULL)
IDBG_TBL_END