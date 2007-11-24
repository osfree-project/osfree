/*
 $Id: F_debug.cpp,v 1.4 2002/11/18 13:24:49 evgen2 Exp $
*/
/* F_debug.cpp */
/* ver 0.00 20.08.2002       */
/* DEBUG: section 0     Debug Routines */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <builtin.h>
#include <time.h>
#include <assert.h>

#include "FreePM.hpp"


#include "F_globals.hpp"
#include <pmclient.h>



void fatalf(const char *fmt,...);
const char * xstrerror_type(int type);

static char *debug_log_file = NULL;
static int Ctx_Lock = 0;
static const char *debugLogTime(time_t);
static void ctx_print(void);
#if HAVE_SYSLOG
static void _db_print_syslog(const char *format, va_list args);
#endif
static void _db_print_stderr(const char *format, va_list args);
static void _db_print_file(const char *format, va_list args);

#include <time.h>

int DebugCount=0;

void
_db_print(const char *format,...)
{
    char f[BUFSIZ];
    va_list args1;
    va_list args2;
    va_list args3;
static char strOldLogTime[32]="";
       char strLogTime[32];


    /* give a chance to context-based debugging to print current context */
    if (!Ctx_Lock)
       ctx_print();
    va_start(args1, format);
    va_start(args2, format);
    va_start(args3, format);
    strcpy(strLogTime,debugLogTime(_FreePM_curtime));
    if(strcmp(strLogTime,strOldLogTime) || DebugCount < 1)
    {  snprintf(f, BUFSIZ, "%i %s|%s",DebugCount ,strLogTime, format);
       strcpy(strOldLogTime,strLogTime);
    } else
       snprintf(f, BUFSIZ, "%i|%s",DebugCount ,format);
DebugCount++;

    if (_FreePM_debug_log != stderr || DebugCount >= 10)
    _db_print_file(f, args1);
    if (_FreePM_debug_log != stderr || DebugCount < 10)
       _db_print_stderr(f, args2);
#if HAVE_SYSLOG
    _db_print_syslog(format, args3);
#endif
    va_end(args1);
    va_end(args2);
    va_end(args3);
}



static void
_db_print_file(const char *format, va_list args)
{
    if (_FreePM_debug_log == NULL)
       return;
    /* give a chance to context-based debugging to print current context */
    if (!Ctx_Lock)
       ctx_print();
    vfprintf(_FreePM_debug_log, format, args);
    if (!_FreePMconfig.onoff.buffered_logs)
       fflush(_FreePM_debug_log);
}


static void
_db_print_stderr(const char *format, va_list args)
{   int useColor=0;
    if (_FreePM_opt_debug_stderr < _FreePM_db_level)
       return;
    if(_FreePMconfig.onoff.UseANSI_stdout)
    {  if(strstr(format,"WARNING:"))
       {  vfprintf(stderr, WARNING_COLOR,NULL);
          useColor=1;
       } else
             if(strstr(format,"IGNORING:")) {
          vfprintf(stderr, IGNORING_COLOR,NULL);
          useColor=1;
       }
    }

    vfprintf(stderr, format, args);
    if(useColor)
    {  vfprintf(stderr, NORMAL_COLOR,NULL);
       fflush(stderr);
       if(_FreePMconfig.onoff.UseBeepAtFatalError)
          DosBeep(2000,1);
    }
}

#if HAVE_SYSLOG
static void
_db_print_syslog(const char *format, va_list args)
{
    LOCAL_ARRAY(char, tmpbuf, BUFSIZ);
    /* level 0,1 go to syslog */
    if (_FreePM_db_level > 1)
       return;
    if (0 == opt_syslog_enable)
       return;
    tmpbuf[0] = '\0';
    vsnprintf(tmpbuf, BUFSIZ, format, args);
    tmpbuf[BUFSIZ - 1] = '\0';
    syslog(_FreePM_db_level == 0 ? LOG_WARNING : LOG_NOTICE, "%s", tmpbuf);
}
#endif /* HAVE_SYSLOG */

static void
debugArg(const char *arg)
{
    int s = 0;
    int l = 0;
    int i;
    if (!strnicmp(arg, "ALL", 3)) {
       s = -1;
       arg += 4;
    } else {
       s = atoi(arg);
       while (*arg && *arg++ != ',');
    }

    l = atoi(arg);
//EK debug
//l=10;
    assert(s >= -1);
    assert(s < MAX_DEBUG_SECTIONS);
    if (l < 0)
       l = 0;
    if (l > 10)
       l = 10;
    if (s >= 0) {
       _FreePM_debugLevels[s] = l;
       return;
    }
    for (i = 0; i < MAX_DEBUG_SECTIONS; i++)
       _FreePM_debugLevels[i] = l;
}

static void
debugOpenLog(const char *logfile)
{
    if (logfile == NULL) {
       _FreePM_debug_log = stderr;
       return;
    }
    if (debug_log_file)
       free(debug_log_file);
    debug_log_file = strdup(logfile); /* keep a static copy */
    if (_FreePM_debug_log && _FreePM_debug_log != stderr)
       fclose(_FreePM_debug_log);
    _FreePM_debug_log = fopen(logfile, "a+");
    if (!_FreePM_debug_log) {
//         debug(0, 0) ("Cannot open log file: %s: fopen error: %s\n",
//               logfile, xstrerror_type(FD_FILE));
          fatalf("Cannot open log file:'%s', error: %s\n"
               "\tPlease check path and/or filename.\n", xstrerror_type(FD_FILE),logfile);

//       fprintf(stderr, "\aWARNING: Cannot write log file: %s\n\a", logfile);
//       perror(logfile);
//       fprintf(stderr, "         messages will be sent to 'stderr'.\n");
//       fflush(stderr);
       _FreePM_debug_log = stderr;
    }
}

void
_db_init(const char *logfile, const char *options)
{
    int i;
    char *p = NULL;
    char *s = NULL;

    for (i = 0; i < MAX_DEBUG_SECTIONS; i++)
       _FreePM_debugLevels[i] = -1;

    if (options) {
       p = strdup(options);
       for (s = strtok(p, w_space); s; s = strtok(NULL, w_space))
           debugArg(s);
       free(p);
    }
    debugOpenLog(logfile);

#if HAVE_SYSLOG && defined(LOG_LOCAL4)
    if (opt_syslog_enable)
       openlog(appname, LOG_PID | LOG_NDELAY | LOG_CONS, LOG_LOCAL4);
#endif /* HAVE_SYSLOG */

}

void
_db_rotate_log(void)
{
    int i;
    char from[_MAX_PATH];
    char to[_MAX_PATH];
#ifdef S_ISREG
    struct stat sb;
#endif

    if (debug_log_file == NULL)
       return;
#ifdef S_ISREG
    if (stat(debug_log_file, &sb) == 0)
       if (S_ISREG(sb.st_mode) == 0)
           return;
#endif

    /*
     * NOTE: we cannot use xrename here without having it in a
     * separate file -- tools.c has too many dependencies to be
     * used everywhere debug.c is used.
     */
    /* Rotate numbers 0 through N up one */
    for (i = _FreePMconfig.Log.rotateNumber; i > 1;) {
       i--;
       snprintf(from, _MAX_PATH, "%s.%d", debug_log_file, i - 1);
       snprintf(to, _MAX_PATH, "%s.%d", debug_log_file, i);
       rename(from, to);
    }
    /* Rotate the current log to .0 */
    if (_FreePMconfig.Log.rotateNumber > 0) {
       snprintf(to, _MAX_PATH, "%s.%d", debug_log_file, 0);
       if (_FreePM_debug_log && _FreePM_debug_log != stderr)
       {                 fclose(_FreePM_debug_log);
                         _FreePM_debug_log = NULL;
       }

       rename(debug_log_file, to);
    }
    /* Close and reopen the log.  It may have been renamed "manually"
     * before HUP'ing us. */
    if (_FreePM_debug_log != stderr)
       debugOpenLog(_FreePMconfig.Log.log);
}

static const char *
debugLogTime(time_t t)
{
    struct tm *tm;
    static char buf[128];
    static time_t last_t = 0;
    if (t != last_t) {
       tm = localtime(&t);
       strftime(buf, 127, "%Y/%m/%d %H:%M:%S", tm);
       last_t = t;
    }
    return buf;
}

void
xassert(const char *msg, const char *file, int line)
{
    debug(0, 0) ("assertion failed: %s:%d: \"%s\"\n", file, line, msg);
    if (!_FreePM_shutting_down)
       abort();
}

/*
 * Context-based Debugging
 *
 * Rationale
 * ---------
 *
 * When you have a long nested processing sequence, it is often impossible
 * for low level routines to know in what larger context they operate. If a
 * routine coredumps, one can restore the context using debugger trace.
 * However, in many case you do not want to coredump, but just want to report
 * a potential problem. A report maybe useless out of problem context.
 *
 * To solve this potential problem, use the following approach:
 *
 * int
 * top_level_foo(const char *url)
 * {
 *      // define current context
 *      // note: we stack but do not dup ctx descriptions!
 *      Ctx ctx = ctx_enter(url);
 *      ...
 *      // go down; middle_level_bar will eventually call bottom_level_boo
 *      middle_level_bar(method, protocol);
 *      ...
 *      // exit, clean after yourself
 *      ctx_exit(ctx);
 * }
 *
 * void
 * bottom_level_boo(int status, void *data)
 * {
 *      // detect exceptional condition, and simply report it, the context
 *      // information will be available somewhere close in the log file
 *      if (status == STRANGE_STATUS)
 *      debug(13, 6) ("DOS attack detected, data: %p\n", data);
 *      ...
 * }
 *
 * Current implementation is extremely simple but still very handy. It has a
 * negligible overhead (descriptions are not duplicated).
 *
 * When the _first_ debug message for a given context is printed, it is
 * prepended with the current context description. Context is printed with
 * the same debugging level as the original message.
 *
 * Note that we do not print context every type you do ctx_enter(). This
 * approach would produce too many useless messages.  For the same reason, a
 * context description is printed at most _once_ even if you have 10
 * debugging messages within one context.
 *
 * Contexts can be nested, of course. You must use ctx_enter() to enter a
 * context (push it onto stack).  It is probably safe to exit several nested
 * contexts at _once_ by calling ctx_exit() at the top level (this will pop
 * all context till current one). However, as in any stack, you cannot start
 * in the middle.
 *
 * Analysis:
 * i)   locate debugging message,
 * ii)  locate current context by going _upstream_ in your log file,
 * iii) hack away.
 *
 *
 * To-Do:
 * -----
 *
 *       decide if we want to dup() descriptions (adds overhead) but allows to
 *       add printf()-style interface
 *
 * implementation:
 * ---------------
 *
 * descriptions for contexts over CTX_MAX_LEVEL limit are ignored, you probably
 * have a bug if your nesting goes that deep.
 */

#define CTX_MAX_LEVEL 255

/*
 * produce a warning when nesting reaches this level and then double
 * the level
 */
static int Ctx_Warn_Level = 32;
/* all descriptions has been printed up to this level */
static int Ctx_Reported_Level = -1;
/* descriptions are still valid or active up to this level */
static int Ctx_Valid_Level = -1;
/* current level, the number of nested ctx_enter() calls */
static int Ctx_Current_Level = -1;
/* saved descriptions (stack) */
static const char *Ctx_Descrs[CTX_MAX_LEVEL + 1];
/* "safe" get secription */
static const char *ctx_get_descr(int  ctx);


int
ctx_enter(const char *descr)
{
    Ctx_Current_Level++;

    if (Ctx_Current_Level <= CTX_MAX_LEVEL)
       Ctx_Descrs[Ctx_Current_Level] = descr;

    if (Ctx_Current_Level == Ctx_Warn_Level) {
       debug(0, 0) ("# ctx: suspiciously deep (%d) nesting:\n", Ctx_Warn_Level);
       Ctx_Warn_Level *= 2;
    }
    return Ctx_Current_Level;
}

void
ctx_exit(int ctx)
{
    assert(ctx >= 0);
    Ctx_Current_Level = (ctx >= 0) ? ctx - 1 : -1;
    if (Ctx_Valid_Level > Ctx_Current_Level)
       Ctx_Valid_Level = Ctx_Current_Level;
}

/*
 * the idea id to print each context description at most once but provide enough
 * info for deducing the current execution stack
 */
static void
ctx_print(void)
{
    /* lock so _db_print will not call us recursively */
    Ctx_Lock++;
    /* ok, user saw [0,Ctx_Reported_Level] descriptions */
    /* first inform about entries popped since user saw them */
    if (Ctx_Valid_Level < Ctx_Reported_Level) {
       if (Ctx_Reported_Level != Ctx_Valid_Level + 1)
           _db_print("ctx: exit levels from %2d down to %2d\n",
               Ctx_Reported_Level, Ctx_Valid_Level + 1);
       else
           _db_print("ctx: exit level %2d\n", Ctx_Reported_Level);
       Ctx_Reported_Level = Ctx_Valid_Level;
    }
    /* report new contexts that were pushed since last report */
    while (Ctx_Reported_Level < Ctx_Current_Level) {
       Ctx_Reported_Level++;
       Ctx_Valid_Level++;
       _db_print("ctx: enter level %2d: '%s'\n", Ctx_Reported_Level,
           ctx_get_descr(Ctx_Reported_Level));
    }
    /* unlock */
    Ctx_Lock--;
}

/* checks for nulls and overflows */
static const char *
ctx_get_descr(int ctx)
{
    if (ctx < 0 || ctx > CTX_MAX_LEVEL)
       return "<lost>";
    return Ctx_Descrs[ctx] ? Ctx_Descrs[ctx] : "<null>";
}

/************************************/
/*   fatal* functions family        */
/************************************/
static void fatalvf(const char *fmt, va_list args);
static void fatal_common(const char *message);

void fatalf(const char *fmt,...);

static void
fatal_common(const char *message)
{
 if(_FreePM_FatalInProgress) return;
 _FreePM_FatalInProgress = 1;

#if HAVE_SYSLOG
    syslog(LOG_ALERT, "%s", message);
#endif
    if(_FreePMconfig.onoff.UseANSI_stdout && _FreePM_debug_log == stderr)
    {  fprintf(_FreePM_debug_log, FATAL_COLOR  "FATAL: %s\n" NORMAL_COLOR, message);
    } else {
       fprintf(_FreePM_debug_log, "FATAL: %s\n", message);
    }
    if (_FreePM_opt_debug_stderr > 0 && _FreePM_debug_log != stderr)
    {  if(_FreePMconfig.onoff.UseANSI_stdout)
       {  fprintf(stderr, FATAL_COLOR "FATAL: %s\n" NORMAL_COLOR, message);
       } else {
           fprintf(stderr, "FATAL: %s\n", message);
       }
    }
    fprintf(_FreePM_debug_log, "%s (Version %s): Terminated abnormally.\n",
    _FreePM_Application_Name,_FreePM_Application_Vers);

    fflush(_FreePM_debug_log);

    if(_FreePMconfig.onoff.UseBeepAtFatalError)
    {  DosBeep(1000,1);  DosBeep(1200,1);  DosBeep(1300,2);
       DosBeep(5000,1);  DosBeep(5400,1);  DosBeep(5500,2);
    }
//todo ?
//    PrintRusage();
}

/* fatal */
void APIENTRY fatal(const char *message)
{
//todo
// for future usage with sockets
//    releaseServerSockets();
//todo
//    KillAllChilds();

    fatal_common(message);
    if (_FreePM_shutting_down)
       exit(0);
    else
       abort();
}

/* printf-style interface for fatal */
void
fatalf(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);

    fatalvf(fmt, args);
    va_end(args);
}


/* used by fatalf */
static void
fatalvf(const char *fmt, va_list args)
{
    static char fatal_str[BUFSIZ];
    vsnprintf(fatal_str, sizeof(fatal_str), fmt, args);
    fatal(fatal_str);
}

