/*! @file F_debug.cpp
    @brief Debug and fatal-error infrastructure, shared by all
        components. Lives in fpmdebug.dll.

    Output targets: log file, stderr (level-filtered), syslog (Unix
    only), wire sink (when installed). Thread-safe via a process-wide
    mutex. No dependency on the transport layer: the wire sink is
    installed by the client at connect time.
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "F_platform.h"
#include "F_debug.h"

/*! @brief Per-section debug levels. -1 means the section is not
    configured; messages for it are silently dropped. */
static APIRET F_DbgLevels[F_DBG_MAX_SECTIONS];

/*! @brief Non-zero while a fatal error is being logged. Guards
    F_DbgFatal against re-entry (for example, an I/O error inside the
    log writer itself). */
static int    F_DbgFatalInProgress = 0;

/*! @brief Non-zero while the process is shutting down. Suppresses
    abort() inside F_DbgAssert. */
static int    F_DbgShuttingDown    = 0;

/*! @brief Runtime configuration, copied by F_DbgInit. */
static F_DbgConfig      s_cfg;

/*! @brief Log-file handle, or NULL when no log file is configured. */
static FILE            *s_log       = NULL;

/*! @brief Registered wire-forward sink, or NULL when none is set. */
static F_DbgWireSink    s_wireSink  = NULL;

/*! @brief Monotonic counter prepended to every message. */
static int              s_counter   = 0;

/*! @brief Protects all mutable state above and every emit path below. */
static HMTX             s_mutex     = NULL;

/*! @brief Set once by F_DbgInit; subsequent calls are no-ops. */
static int              s_inited    = 0;

/*! @def F_DBG_CTX_MAX
    @brief Maximum nesting depth for F_DbgCtxEnter. */
#define F_DBG_CTX_MAX  32

/*! @brief Context descriptions, indexed by nesting level. */
static const char *s_ctxDescr[F_DBG_CTX_MAX + 1];

/*! @brief Current context depth (-1 = outside any context). */
static int         s_ctxCur     = -1;

/*! @brief Highest context level already printed in the log. */
static int         s_ctxReported = -1;

/* ======================================================================== */
/*  Mutex helpers                                                           */
/* ======================================================================== */

/*! @brief Acquire the debug mutex. No-op when F_DbgInit could not
    create a mutex (single-threaded fallback). */
static void LockDbg(void)
{
    if (s_mutex) DosRequestMutexSem(s_mutex, SEM_INDEFINITE_WAIT);
}

/*! @brief Release the debug mutex. */
static void UnlockDbg(void)
{
    if (s_mutex) DosReleaseMutexSem(s_mutex);
}

/* ======================================================================== */
/*  Level parsing                                                           */
/* ======================================================================== */

/*! @brief Parse one level token of the form "N,M" or "ALL,M" and
    update F_DbgLevels. Levels are clamped to 0..10.
    @param [in] tok  Token to parse. NULL or empty is ignored. */
static void ParseLevelToken(const char *tok)
{
    int section = 0;
    int level   = 0;
    int all     = 0;
    const char *comma;

    if (!tok || !*tok) return;

    if (strncmp(tok, "ALL", 3) == 0) {
        all = 1;
        comma = strchr(tok, ',');
        if (!comma) return;
        level = atoi(comma + 1);
    } else {
        comma = strchr(tok, ',');
        if (!comma) return;
        section = atoi(tok);
        level   = atoi(comma + 1);
    }

    if (level < 0)  level = 0;
    if (level > 10) level = 10;

    if (all) {
        int i;
        for (i = 0; i < F_DBG_MAX_SECTIONS; i++)
            F_DbgLevels[i] = (APIRET)level;
    } else if (section >= 0 && section < F_DBG_MAX_SECTIONS) {
        F_DbgLevels[section] = (APIRET)level;
    }
}

/*! @brief Parse a whitespace-separated level string and update
    F_DbgLevels. Accepted forms: "ALL,7", "1,5", "1,5 2,3 4,7".
    @param [in] levels  Level string, or NULL to leave levels unchanged. */
void F_DbgSetLevels(const char *levels)
{
    const char *p;
    char        tok[32];
    int         n;

    if (!levels) return;

    p = levels;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        n = 0;
        while (*p && *p != ' ' && *p != '\t' && n < (int)sizeof(tok) - 1)
            tok[n++] = *p++;
        tok[n] = 0;
        if (n) ParseLevelToken(tok);
    }
}

/* ======================================================================== */
/*  Timestamp                                                               */
/* ======================================================================== */

/*! @brief Format the current local time as "YYYY/MM/DD HH:MM:SS".
    The returned pointer refers to a static buffer and is valid until
    the next call.
    @return Pointer to the formatted timestamp, or to a fixed
            zero-filled string if localtime() fails. */
static const char *DbgTimeNow(void)
{
    static char  buf[32];
    time_t       t = time(NULL);
    struct tm   *tm = localtime(&t);

    if (!tm) { strcpy(buf, "0000/00/00 00:00:00"); return buf; }
    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", tm);
    return buf;
}

/* ======================================================================== */
/*  Context printing                                                        */
/* ======================================================================== */

/*! @brief Emit any pending context-change lines to @p emit. Called
    under the debug mutex, before each regular message.
    @param [in] emit  Output function; must accept a printf-style
                      format string followed by varargs. */
static void DbgCtxDump(void (*emit)(const char *fmt, ...))
{
    if (s_ctxReported < s_ctxCur) {
        while (s_ctxReported < s_ctxCur) {
            s_ctxReported++;
            emit("ctx: enter level %d: '%s'\n",
                 s_ctxReported,
                 s_ctxDescr[s_ctxReported] ? s_ctxDescr[s_ctxReported] : "<null>");
        }
    } else if (s_ctxReported > s_ctxCur) {
        emit("ctx: exit level %d\n", s_ctxReported);
        s_ctxReported = s_ctxCur;
    }
}

/*! @brief Enter a debug context. Pushes @p descr onto the
    process-global context stack. Thread-safe.
    @param [in] descr  Context description; must remain valid for the
                       lifetime of the context.
    @return The new context level (passed to F_DbgCtxExit). */
int F_DbgCtxEnter(const char *descr)
{
    LockDbg();
    s_ctxCur++;
    if (s_ctxCur <= F_DBG_CTX_MAX) s_ctxDescr[s_ctxCur] = descr;
    UnlockDbg();
    return s_ctxCur;
}

/*! @brief Leave a debug context, popping all levels above @p ctx.
    Thread-safe.
    @param [in] ctx  Level returned by the matching F_DbgCtxEnter, or a
                     lower value to unwind several nested contexts at once. */
void F_DbgCtxExit(int ctx)
{
    LockDbg();
    s_ctxCur = (ctx > 0) ? ctx - 1 : -1;
    if (s_ctxReported > s_ctxCur) s_ctxReported = s_ctxCur;
    UnlockDbg();
}

/* ======================================================================== */
/*  Emitters (called under lock)                                            */
/* ======================================================================== */

/*! @brief Write a printf-style line to the configured log file. No-op
    when no log file is configured.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Format arguments. */
static void EmitFile(const char *fmt, ...)
{
    va_list args;
    if (!s_log) return;
    va_start(args, fmt);
    vfprintf(s_log, fmt, args);
    va_end(args);
    if (!s_cfg.buffered) fflush(s_log);
}

/*! @brief Write a printf-style line to stderr.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Format arguments. */
static void EmitStderr(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

/*! @brief Mirror a printf-style line to syslog. No-op when HAVE_SYSLOG
    is 0 or s_cfg.useSyslog is 0.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Format arguments. */
static void EmitSyslog(const char *fmt, ...)
{
#if HAVE_SYSLOG
    va_list args;
    char    buf[F_DBG_BUFSIZE];
    if (!s_cfg.useSyslog) return;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    syslog(LOG_DEBUG, "%s", buf);
#else
    (void)fmt;
#endif
}

/* ======================================================================== */
/*  Rotation                                                                */
/* ======================================================================== */

/*! @brief Rotate the log file if rotation is enabled. Renames
    N-1 -> N, … , 0 -> 1, then the active file -> 0. Reopens the log
    file. No-op when rotation is not configured. */
void F_DbgRotate(void)
{
    int   i;
    char  from[260];
    char  to[260];

    LockDbg();
    if (!s_cfg.logfile || s_cfg.rotateNumber <= 0) { UnlockDbg(); return; }

    for (i = s_cfg.rotateNumber; i > 1; ) {
        i--;
        snprintf(from, sizeof(from), "%s.%d", s_cfg.logfile, i - 1);
        snprintf(to,   sizeof(to),   "%s.%d", s_cfg.logfile, i);
        rename(from, to);
    }
    snprintf(to, sizeof(to), "%s.0", s_cfg.logfile);
    if (s_log) { fclose(s_log); s_log = NULL; }
    rename(s_cfg.logfile, to);
    s_log = fopen(s_cfg.logfile, "a+");
    UnlockDbg();
}

/* ======================================================================== */
/*  Init                                                                    */
/* ======================================================================== */

/*! @brief Open the configured log file. Leaves s_log NULL when no
    log file is configured or it could not be opened. */
static void DbgOpenLog(void)
{
    if (!s_cfg.logfile || s_cfg.logfile[0] == 0) {
        s_log = NULL;
        return;
    }
    s_log = fopen(s_cfg.logfile, "a+");
    if (!s_log) {
        fprintf(stderr, "FDBG: cannot open '%s', continuing without file log\n",
                s_cfg.logfile);
    }
}

/*! @brief Initialise the debug subsystem. Idempotent.

    Creates the process-wide mutex, copies the configuration, resets
    all section levels to -1 (unconfigured), parses @p levels, and
    opens the log file.

    @param [in] cfg     Configuration, or NULL for defaults.
    @param [in] levels  Level string, or NULL to leave levels unconfigured. */
void F_DbgInit(const F_DbgConfig *cfg, const char *levels)
{
    int i;

    if (s_inited) return;
    s_inited = 1;

    if (DosCreateMutexSem(NULL, &s_mutex, 0, FALSE) != 0)
        s_mutex = NULL;

    if (cfg) s_cfg = *cfg;
    else {
        s_cfg.logfile      = NULL;
        s_cfg.rotateNumber = 0;
        s_cfg.buffered     = 0;
        s_cfg.ansiColor    = 0;
        s_cfg.beepOnFatal  = 0;
        s_cfg.useSyslog    = 0;
        s_cfg.stderrLevel  = 5;
    }

    for (i = 0; i < F_DBG_MAX_SECTIONS; i++)
        F_DbgLevels[i] = (APIRET)-1;

    if (levels) F_DbgSetLevels(levels);

    DbgOpenLog();
}

/* ======================================================================== */
/*  Main emission                                                           */
/* ======================================================================== */

/*! @brief Install or remove the wire-forward sink. Pass NULL to
    uninstall. The sink is invoked for every accepted F_DbgPrint /
    F_DbgFatal call.
    @param [in] sink  Sink callback, or NULL. */
void APIENTRY F_DbgSetWireSink(F_DbgWireSink sink)
{
    s_wireSink = sink;
}

/*! @brief Emit a diagnostic message if its level passes the section
    filter. Thread-safe.

    Messages whose level is greater than s_cfg.stderrLevel are routed
    to the log file and the wire sink only; they do not appear on
    stderr.

    A trailing newline is appended to the composed line if it is not
    already present; otherwise consecutive messages would be glued
    together in the log.

    @param [in] section  Debug section index (0..F_DBG_MAX_SECTIONS-1).
    @param [in] level    Message level (0 = critical, 10 = verbose).
    @param [in] fmt      printf-style format string.
    @param [in] ...      Format arguments. */
void APIENTRY F_DbgPrint(int section, int level, const char *fmt, ...)
{
    va_list args;
    char    argsbuf[F_DBG_BUFSIZE];
    char    fullbuf[F_DBG_BUFSIZE * 2];
    int     toStderr;

    if (section < 0 || section >= F_DBG_MAX_SECTIONS) return;
    if ((int)F_DbgLevels[section] < 0) return;
    if (level > (int)F_DbgLevels[section]) return;

    va_start(args, fmt);
    vsnprintf(argsbuf, sizeof(argsbuf), fmt, args);
    va_end(args);

    LockDbg();

    snprintf(fullbuf, sizeof(fullbuf), "%d %s|%s",
             s_counter, DbgTimeNow(), argsbuf);
    s_counter++;

    {
        size_t len = strlen(fullbuf);
        if (len > 0 && len < sizeof(fullbuf) - 1 && fullbuf[len - 1] != '\n') {
            fullbuf[len]     = '\n';
            fullbuf[len + 1] = 0;
        }
    }

    toStderr = (level <= s_cfg.stderrLevel);

    if (s_log) {
        DbgCtxDump(EmitFile);
        EmitFile("%s", fullbuf);
    }
    EmitSyslog("%s", fullbuf);
    if (toStderr) {
        DbgCtxDump(EmitStderr);
        EmitStderr("%s", fullbuf);
    }

    if (s_wireSink) s_wireSink(F_DBG_KIND_PRINT, fullbuf);

    UnlockDbg();
}

/*! @brief Emit an unconditional local-only trace (no section filter,
    no wire sink). Intended for transport-level debugging. Thread-safe.
    A trailing newline is appended if not already present.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Format arguments. */
void APIENTRY F_DbgTrace(const char *fmt, ...)
{
    va_list args;
    char    buf[F_DBG_BUFSIZE];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    LockDbg();
    if (s_log) {
        fprintf(s_log, "%s%s", buf,
                (buf[0] && buf[strlen(buf) - 1] != '\n') ? "\n" : "");
        fflush(s_log);
    }
    fprintf(stderr, "%s%s", buf,
            (buf[0] && buf[strlen(buf) - 1] != '\n') ? "\n" : "");
    fflush(stderr);
    UnlockDbg();
}

/* ======================================================================== */
/*  Fatal                                                                   */
/* ======================================================================== */

/*! @brief Play the configured fatal-error beep sequence. */
static void DbgBeep(void)
{
    DosBeep(3000, 1); DosBeep(3200, 1); DosBeep(3300, 2);
    DosBeep(5000, 2); DosBeep(5200, 2); DosBeep(5300, 5);
}

/*! @brief Log and abort with a raw string. Guarded against re-entry:
    a second call while the first is logging is a no-op.

    @param [in] msg  Fatal error message. Must not be NULL. */
void APIENTRY F_DbgFatal(const char *msg)
{
    char line[F_DBG_BUFSIZE * 2];

    if (F_DbgFatalInProgress) return;
    F_DbgFatalInProgress = 1;

    LockDbg();

    snprintf(line, sizeof(line), "%d %s|FATAL: %s\n",
             s_counter, DbgTimeNow(), msg);
    s_counter++;

    if (s_log) {
        DbgCtxDump(EmitFile);
        EmitFile("%s", line);
    }
    EmitSyslog("%s", line);
    if (s_cfg.stderrLevel > 0) {
        DbgCtxDump(EmitStderr);
        EmitStderr("%s", line);
    }

    if (s_wireSink) s_wireSink(F_DBG_KIND_FATAL, msg);

    if (s_cfg.beepOnFatal) DbgBeep();

    if (s_log) fflush(s_log);

    UnlockDbg();

    abort();
}

/*! @brief Log and abort with a printf-style string.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Format arguments. */
void APIENTRY F_DbgFatalf(const char *fmt, ...)
{
    char    buf[F_DBG_BUFSIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    F_DbgFatal(buf);
}

/*! @brief Assertion-failure handler. Calls F_DbgFatal unless the
    process is already shutting down, in which case the failure is
    swallowed.
    @param [in] msg   Assertion text.
    @param [in] file  Source file name.
    @param [in] line  Source line number. */
void APIENTRY F_DbgAssert(const char *msg, const char *file, int line)
{
    char buf[F_DBG_BUFSIZE];

    snprintf(buf, sizeof(buf), "assertion failed: %s:%d: \"%s\"",
             file, line, msg);

    if (F_DbgShuttingDown) return;
    F_DbgFatal(buf);
}

/*! @brief Set the process-wide shutdown flag. Suppresses abort()
    inside F_DbgAssert while the flag is non-zero.
    @param [in] flag  Non-zero to suppress asserts during shutdown. */
void APIENTRY F_DbgSetShuttingDown(int flag)
{
    F_DbgShuttingDown = flag;
}
