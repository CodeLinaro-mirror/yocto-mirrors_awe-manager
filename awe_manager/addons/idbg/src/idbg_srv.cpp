
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
#include <awosal_time.h>
#if defined (AWOSAL_WINDOWS)
#include <io.h>
#define fdopen _fdopen
#define STDIN_IS_TTY() (_isatty(_fileno(stdin)))
#else
#include <unistd.h>
#define STDIN_IS_TTY() (isatty(fileno(stdin)))
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
    m_server_fd(-1), m_client_fd(-1), m_bTimeCommands(false), m_script_mode(false)
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
    // Use isocline only when stdin is an interactive terminal.
    // When stdin is a pipe (e.g. test subprocess), isocline's prompt output
    // is unpredictable; fall through to plain fgets so the prompt is always
    // the well-known "awemgr-shell > " string.
    if (STDIN_IS_TTY())
        return runFromHandle(NULL);
#endif
    return runFromHandle(stdin);
}

int CIdbgSrv::runFile(char *filename_p)
{
    FILE *fp = fopen(filename_p, "r");
    int retval = -1;
    if (fp)
    {
        m_script_mode = true;
        retval = runFromHandle(fp);
        m_script_mode = false;
        fclose(fp);
    }
    return retval;
}

static void print_on_socket(idbg_t *p, const char *fmt, va_list ap)
{
    int *fd_p = (int*) idbg_get_printfct_backend(p);

#ifdef AWOSAL_WINDOWS
    char buf[4096];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    si_write(*fd_p, buf, (int)strlen(buf));
#else
    vdprintf(*fd_p, fmt, ap);
#endif
}


int CIdbgSrv::runSocket(char* socket_host, char* socket_port)
{
    int server_fd = -1;
    if (si_listen(atoi(socket_port), &server_fd) != 0)
        return -1;
    m_server_fd = server_fd;

    int client_fd = si_accept(server_fd);

    /* Close the listening socket immediately — we only serve one client
     * per call, and stop() may already have closed it. */
    si_close_server(server_fd);
    m_server_fd = -1;

    if (client_fd < 0)
        return 0; /* accept interrupted (stop() or signal) — not a fatal error */

    m_client_fd = client_fd;
    fprintf(stderr, "idbg_srv: shell client connected on port %s\n", socket_port);
    idbg_set_printfct(m_idbg_p, print_on_socket, &m_client_fd);
    /* Use si_readln-based reading on all platforms so that the prompt is sent
     * over the socket via idbg_print (the print_on_socket callback) before
     * each command read.  On Windows, fdopen of a SOCKET is unsupported; on
     * POSIX, using runFromSocket avoids isatty() incorrectly suppressing the
     * prompt (a socket fd is not a TTY). */
    runFromSocket(client_fd);
    idbg_reset_printfct(m_idbg_p);
    si_close_connection(client_fd);
    m_client_fd = -1;
    fprintf(stderr, "idbg_srv: shell client disconnected\n");

    return 0;
}

void CIdbgSrv::stop()
{
    int sfd = m_server_fd.exchange(-1);
    if (sfd >= 0)
        si_close_server(sfd);

    int cfd = m_client_fd.exchange(-1);
    if (cfd >= 0)
        si_close_connection(cfd);
}

int CIdbgSrv::runFromSocket(int fd)
{
    int scripterr = 0;

    while (!scripterr)
    {
        idbg_print(m_idbg_p, "%s > ", IDBG_PROMPT);
        int n = si_readln(fd, m_cmdline, sizeof(m_cmdline));
        if (n <= 0)
            break;

        /* Strip trailing CR/LF that telnet or netcat may send. */
        m_cmdline[strcspn(m_cmdline, "\r\n")] = '\0';

        if (m_cmdline[0] == '\0')
            continue;

        aweosal_clock_time start_time = aweosal_measure_start();
        scripterr = idbg_parse_cmd(m_idbg_p, (unsigned char*)m_cmdline, CMDLINE_SIZE);
        double elapsed_ms = aweosal_measure_elapsed(start_time);
        reportTimedCommand(elapsed_ms);
    }

    if (scripterr && scripterr != -1)
        return -1;

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

    aweosal_clock_time start_time = aweosal_measure_start();
    int scripterr = idbg_parse_cmd (m_idbg_p, (unsigned char*) m_cmdline, CMDLINE_SIZE);
    double elapsed_ms = aweosal_measure_elapsed(start_time);
    reportTimedCommand(elapsed_ms);
    return scripterr;
}

// ***** PRIVATE ******

int CIdbgSrv::runFromHandle(FILE *fp)
{
    int scripterr = 0;

    // m_state = CIDBGSRV_RUNCONSOLE;
    while (! scripterr && ! getOneCommand (m_cmdline, sizeof(m_cmdline), fp))
    {
        /* Skip empty lines and comment lines (# prefix, with optional leading whitespace). */
        const char *trimmed = m_cmdline;
        while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\r' || *trimmed == '\n') trimmed++;
        if (*trimmed == '\0' || *trimmed == '#')
            continue;

        aweosal_clock_time start_time = aweosal_measure_start();
        scripterr = idbg_parse_cmd (m_idbg_p, (unsigned char*) m_cmdline, CMDLINE_SIZE);
        double elapsed_ms = aweosal_measure_elapsed(start_time);
        reportTimedCommand(elapsed_ms);
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
        if (!m_script_mode)
        {
            idbg_print(m_idbg_p, "%s > ", IDBG_PROMPT);
            fflush(stdout);  // pipe stdout is block-buffered; flush so the prompt arrives before fgets blocks
        }

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
    if (!m_script_mode)
    {
        printf(IDBG_PROMPT);
        fflush(stdout);
    }
    char *context;
    char *l = fgets (cmdbuffer, maxcmdbuffersize, fp);
    if (!l)
        return -1;
    strtok_r (cmdbuffer, "\n", &context);
    return 0;
#endif

}

void CIdbgSrv::reportTimedCommand(double elapsed_ms)
{
    if (m_bTimeCommands)
    {
        idbg_print(m_idbg_p, "time_taken_ms: %f\n", elapsed_ms);
    }
}
