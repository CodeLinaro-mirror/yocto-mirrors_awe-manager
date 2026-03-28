
#include "idbg_srv.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cerrno>

#include "awosal_string.h"
#include "awosal_socket.h"

#ifdef IDBGSRV_USE_ISOCLINE
#include "isocline.h"
#define IDBG_PROMPT "awemgr-shell"
#else
#define IDBG_PROMPT "awemgr-shell > "
#endif
#if defined (AWOSAL_WINDOWS)
#define fdopen _fdopen
#endif


#ifdef IDBGSRV_USE_ISOCLINE
static void word_completer(ic_completion_env_t* cenv, const char* word )
{
    idbg_t* idbg_p = (idbg_t*) ic_completion_arg(cenv);

    if ((strcmp(word, "cd ") == 0) || (strcmp(word, "cd") == 0))
    {
        const char **dir_completions = (const char**) idbg_get_dirlist (idbg_p);
        const char **dir = dir_completions;
        while (*dir != NULL) {
            // ic_add_completion(cenv, *dir);

            char entry[256];
            snprintf(entry, sizeof(entry), "> %s/", *dir);
            ic_add_completion_ex(cenv, *dir, entry, "Subdirectory");

            dir++;
        }
    }
    else
    {
        const char **completions = (const char**) idbg_get_cmdlist (idbg_p); // todo: check conversion!
        ic_add_completions(cenv, word, completions);
    }
}

static void completer(ic_completion_env_t* cenv, const char* input )
{
    // try to complete file names from the roots "." and "/usr/local"
    // ic_complete_filename(cenv, input, 0, ".;/usr/local;c:\\Program Files" , NULL /* any extension */);

    // and also use our custom completer
    ic_complete_word( cenv, input, &word_completer, NULL /* from default word boundary; whitespace or separator */ );

    // ic_complete_word( cenv, input, &word_completer, &ic_char_is_idletter );
    // ic_complete_qword( cenv, input, &word_completer, &ic_char_is_idletter  );
}

#endif


CIdbgSrv::CIdbgSrv(idbgtableentry_t *startTbl) :
    m_client_fd(0)
{
    idbg_init (&m_idbg_p, (idbgtableentry_t *) startTbl );
    idbg_set_userdata(m_idbg_p, this);

#ifdef IDBGSRV_USE_ISOCLINE
    ic_set_history(".awemgrshell.history", -1 /* default entries (= 200) */);
    ic_enable_auto_tab(true);
    // ? todo: check this:  ic_enable_hint(false);
    ic_set_default_completer(&completer, m_idbg_p);
#endif
}

CIdbgSrv::~CIdbgSrv()
{
    idbg_exit (&m_idbg_p);
}

int CIdbgSrv::runConsole()
{
#ifdef IDBGSRV_USE_ISOCLINE
    return runFromHandle(NULL);
#else
    return runFromHandle(stdin);
#endif
}

int CIdbgSrv::runFile(char *filename_p)
{
    FILE *fp = fopen(filename_p, "r");
    int retval = -1;
    if (fp)
    {
        retval = runFromHandle(fp);
        fclose(fp);
    }
    return retval;
}

static void print_on_socket(idbg_t *p, const char *fmt, va_list ap)
{
    int *fd_p = (int*) idbg_get_printfct_backend(p);

#ifdef AWOSAL_WINDOWS
    printf("TODO: solve problem of missing vdprintf on Windows\n");
#else
    vdprintf(*fd_p, fmt, ap);
    // enable this to also dump to server terminal
    //vprintf(fmt, ap);
    //fflush(stdout);
#endif
}


int CIdbgSrv::runSocket(char* socket_host, char* socket_port)
{
    int server_fd;

    // todo: check socket_host arg.. needed/useful ?
    m_client_fd = si_create_server(atoi(socket_port), &server_fd, 10);

    FILE *fp = fdopen(m_client_fd, "r");
    if (fp)
    {
        idbg_set_printfct(m_idbg_p, print_on_socket, &m_client_fd);
        int r = runFromHandle(fp);
        idbg_reset_printfct(m_idbg_p);
    }

    si_close_server(server_fd);

    return 0;
}

void CIdbgSrv::setUserData (void *data_p)
{
    idbg_set_userdata(m_idbg_p, data_p);
}

int CIdbgSrv::runCommand(const char* fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vsnprintf(m_cmdline, sizeof(m_cmdline), fmt, ap);
    va_end   (ap);

    int scripterr = idbg_parse_cmd (m_idbg_p, (unsigned char*) m_cmdline, CMDLINE_SIZE);
    return scripterr;
}

// ***** PRIVATE ******

int CIdbgSrv::runFromHandle(FILE *fp)
{
    int scripterr = 0;

    // m_state = CIDBGSRV_RUNCONSOLE;
    while (! scripterr && ! getOneCommand (m_cmdline, sizeof(m_cmdline), fp))
    {
        scripterr = idbg_parse_cmd (m_idbg_p, (unsigned char*) m_cmdline, CMDLINE_SIZE);
    }
    // m_state = CIDBGSRV_IDLE;

    if (scripterr && (scripterr != -1))
        return -1;

    return 0;
}


int CIdbgSrv::getOneCommand (char *cmdbuffer, int maxcmdbuffersize, FILE *fp)
{
#ifdef IDBGSRV_USE_ISOCLINE
    if (fp)
    {
        int *fd_p = (int*) idbg_get_printfct_backend(m_idbg_p);
        if (fd_p)
            idbg_print(m_idbg_p, "%s > ", IDBG_PROMPT);

        char *context;
        char *l = fgets (cmdbuffer, maxcmdbuffersize, fp);
        if (!l)  {
            // avoid printing errors when EAGAIN was last error
            // (happens when e.g. script file was totally consumed and -i is used to remain in IDBG)
            int err = ferror(fp);
            if (err)
                // earlier we had a check for: if (errno && (errno != EAGAIN))
                // and only in this "if" case we printed an error;
                // keeping this comment just as a remineder in case EAGAIN situation comes again :)
                perror("Failed to get command line buffer");
            return -1;
        }
        strtok_r (cmdbuffer, "\r", &context);
        strtok_r (cmdbuffer, "\n", &context);
    }
    else
    {
        char *l = ic_readline(IDBG_PROMPT);
        if (!l)
            return -1;
        strlcpy (cmdbuffer, l, maxcmdbuffersize);
        free (l);
    }
    return 0;
#else
    printf (IDBG_PROMPT);
    char *context;
    char *l = fgets (cmdbuffer, maxcmdbuffersize, fp);
    if (!l)
        return -1;
    strtok_r (cmdbuffer, "\n", &context);
    return 0;
#endif

}
