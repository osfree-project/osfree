/*! @file F_debug.cpp
    @brief Debug and fatal-error infrastructure, shared by all
        components. Lives in fpmdebug.dll.

    Output targets: log file, stderr (level-filtered), syslog (Unix
    only), wire sink (when installed). Thread-safe via a process-wide
    mutex. No dependency on the transport layer: the wire sink is
    installed by the client at connect time.

    The context stack is per-thread. On OS/2 the process-invariant
    pointer returned by DosAllocThreadLocalMemory (from <os2.h>)
    dereferences to per-thread memory. On Win32 the per-thread handle
    is a TLS index from TlsAlloc, and access goes through TlsGetValue
    / TlsSetValue. No Win32 emulation of the OS/2 API is attempted.
    Context enter/exit lines carry counter and timestamp like any
    other message and are emitted eagerly when they pass the filter.
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

/*! @brief Default configuration, applied from DllMain(ATTACH) before
    any component can log. Values come from F_config.h; see
    F_DBG_DEFAULT_* there. */
static const F_DbgConfig F_DBG_DEFAULT_CFG = {
    F_DBG_DEFAULT_LOGFILE,
    F_DBG_DEFAULT_ROTATENUMBER,
    F_DBG_DEFAULT_ROTATESIZE,
    F_DBG_DEFAULT_BUFFERED,
    F_DBG_DEFAULT_ANSICOLOR,
    F_DBG_DEFAULT_BEEPONFATAL,
    F_DBG_DEFAULT_USESYSLOG,
    F_DBG_DEFAULT_THRESHOLDLEVEL
};

/*! @brief Log-file handle, or NULL when no log file is configured. */
static FILE            *s_log       = NULL;

/*! @brief Bytes written to the active log file since the last open or
    rotation. Used to trigger size-based rotation. */
static ULONG            s_logBytes  = 0;

/*! @brief Registered wire-forward sink, or NULL when none is set. */
static F_DbgWireSink    s_wireSink  = NULL;

/*! @brief Monotonic counter prepended to every message. */
static int              s_counter   = 0;

/*! @brief Protects all mutable state above and every emit path below. */
static HMTX             s_mutex     = NULL;

/*! @brief Set once by F_DbgInit; subsequent calls are no-ops. */
static int              s_inited    = 0;

/*! @brief Thread-local handle for the per-thread context stack.

    OS/2: PULONG from DosAllocThreadLocalMemory — same virtual address
          in every thread, dereference yields per-thread storage.
    Win32: DWORD TLS index from TlsAlloc — invariant process-wide;
          per-thread storage accessed via TlsGetValue / TlsSetValue.
    Initialised once in F_DbgInit; per-thread DBG_CTX_STACK is
    allocated lazily by CtxGetOrCreate. */
#if defined(__OS2__) || defined(__OS2V2__)
static PULONG           s_ctxTls    = NULL;
#else
static DWORD            s_ctxTls    = TLS_OUT_OF_INDEXES;
#endif

/*! @struct _DBG_CTX_STACK
    @brief Per-thread context stack. Allocated lazily on first use by
    F_DbgCtxEnterf in a given thread; released only when the process
    exits (bounded by the number of threads, roughly 8 KiB per
    thread). */
typedef struct _DBG_CTX_STACK {
    int  depth;                                   /*!< -1 = outside any context */
    char descr[F_DBG_CTX_MAX][F_DBG_CTX_MAX_LEN]; /*!< level -> description     */
} DBG_CTX_STACK;

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

/*! @brief Return a pointer to @p n spaces, for indenting nested log
    lines. The returned pointer refers to a static, immutable array of
    spaces and is valid for the lifetime of the process; the function
    is thread-safe.
    @param [in] n  Number of spaces; clamped to 0..64.
    @return Pointer to a NUL-terminated string of @p n spaces. */
static const char *DbgIndent(int n)
{
    static const char s_spaces[] =
        "                                                                ";
    int max = (int)sizeof(s_spaces) - 1;  /* exclude NUL */
    if (n < 0)  n = 0;
    if (n > max) n = max;
    return s_spaces + max - n;
}

/* ======================================================================== */
/*  Context stack (per-thread)                                              */
/* ======================================================================== */

/*! @brief Read the DBG_CTX_STACK pointer from this thread's storage.
    OS/2: dereference the process-invariant pointer returned by
          DosAllocThreadLocalMemory; the kernel resolves it to
          per-thread memory.
    Win32: TlsGetValue on the TLS index.
    @return DBG_CTX_STACK pointer, or NULL.
    @retval NULL   TLS not initialised, or this thread has not
                   allocated a stack yet.
    @retval !NULL  DBG_CTX_STACK pointer for this thread. */
static PVOID CtxTlsGet(void)
{
#if defined(__OS2__) || defined(__OS2V2__)
    if (!s_ctxTls) return NULL;
    return (PVOID)s_ctxTls[0];
#else
    if (s_ctxTls == TLS_OUT_OF_INDEXES) return NULL;
    return TlsGetValue(s_ctxTls);
#endif
}

/*! @brief Write a DBG_CTX_STACK pointer into this thread's storage.
    OS/2: write through the process-invariant pointer.
    Win32: TlsSetValue on the TLS index.
    No-op if TLS is not initialised.
    @param [in] v  DBG_CTX_STACK pointer to store. May be NULL. */
static void CtxTlsSet(PVOID v)
{
#if defined(__OS2__) || defined(__OS2V2__)
    if (!s_ctxTls) return;
    s_ctxTls[0] = (ULONG)v;
#else
    if (s_ctxTls == TLS_OUT_OF_INDEXES) return;
    TlsSetValue(s_ctxTls, v);
#endif
}

/*! @brief Return this thread's DBG_CTX_STACK, or NULL if none has been
    allocated yet for this thread.
    @return Pointer to this thread's stack.
    @retval NULL  TLS not initialised or no stack allocated yet in
                  this thread. */
static DBG_CTX_STACK *CtxGet(void)
{
    return (DBG_CTX_STACK *)CtxTlsGet();
}

/*! @brief Return this thread's DBG_CTX_STACK, allocating it on first
    use.
    @return Pointer to this thread's stack.
    @retval NULL  TLS not initialised, or the per-thread stack could
                  not be allocated. */
static DBG_CTX_STACK *CtxGetOrCreate(void)
{
    DBG_CTX_STACK *p;
    PVOID          pv = NULL;

    p = (DBG_CTX_STACK *)CtxTlsGet();
    if (p) return p;

    if (DosAllocMem(&pv, sizeof(DBG_CTX_STACK),
                    PAG_COMMIT | PAG_READ | PAG_WRITE) != 0)
        return NULL;
    p = (DBG_CTX_STACK *)pv;
    p->depth = -1;
    CtxTlsSet(p);
    return p;
}

/* ======================================================================== */
/*  Emitters (called under lock)                                            */
/* ======================================================================== */

/*! @brief Rotate the log file. Renames N-1 -> N, …, 0 -> 1, then the
    active file -> 0, and reopens the log file. No-op when rotation is
    not configured. Caller must hold the debug mutex. */
static void DbgRotateLocked(void)
{
    int   i;
    char  from[260];
    char  to[260];

    if (!s_cfg.logfile || s_cfg.rotateNumber <= 0) return;

    for (i = s_cfg.rotateNumber; i > 1; ) {
        i--;
        snprintf(from, sizeof(from), "%s.%d", s_cfg.logfile, i - 1);
        snprintf(to,   sizeof(to),   "%s.%d", s_cfg.logfile, i);
        rename(from, to);
    }
    snprintf(to, sizeof(to), "%s.0", s_cfg.logfile);
    if (s_log) { fclose(s_log); s_log = NULL; }
    rename(s_cfg.logfile, to);
    s_log      = fopen(s_cfg.logfile, "a+");
    s_logBytes = 0;
}

/*! @brief Write a pre-formatted line to the configured log file.
    No-op when no log file is configured. Tracks the number of bytes
    written and triggers rotation by size when s_cfg.rotateSize is
    reached.
    @param [in] text  Pre-formatted line, including trailing newline. */
static void EmitFile(const char *text)
{
    size_t n;

    if (!s_log || !text) return;
    n = strlen(text);
    if (n == 0) return;
    fwrite(text, 1, n, s_log);
    if (!s_cfg.buffered) fflush(s_log);
    s_logBytes += (ULONG)n;
    if (s_cfg.rotateSize > 0 && s_logBytes >= (ULONG)s_cfg.rotateSize)
        DbgRotateLocked();
}

/*! @brief Write a pre-formatted line to stderr.
    @param [in] text  Pre-formatted line, including trailing newline. */
static void EmitStderr(const char *text)
{
    if (!text) return;
    fputs(text, stderr);
}

/*! @brief Mirror a pre-formatted line to syslog. No-op when
    HAVE_SYSLOG is 0 or s_cfg.useSyslog is 0.
    @param [in] text  Pre-formatted line. */
static void EmitSyslog(const char *text)
{
#if HAVE_SYSLOG
    if (!text || !s_cfg.useSyslog) return;
    syslog(LOG_DEBUG, "%s", text);
#else
    (void)text;
#endif
}

/*! @brief Emit a pre-formatted line to all live channels (log file,
    syslog, stderr). Caller must hold the debug mutex.
    @param [in] text  Pre-formatted line, including trailing newline. */
static void EmitLineLocked(const char *text)
{
    EmitFile(text);
    EmitSyslog(text);
    EmitStderr(text);
}

/* ======================================================================== */
/*  Rotation                                                                */
/* ======================================================================== */

/*! @brief Rotate the log file if rotation is enabled. Renames
    N-1 -> N, … , 0 -> 1, then the active file -> 0. Reopens the log
    file. No-op when rotation is not configured. */
void F_DbgRotate(void)
{
    LockDbg();
    DbgRotateLocked();
    UnlockDbg();
}

/* ======================================================================== */
/*  Init                                                                    */
/* ======================================================================== */

/*! @brief Open the configured log file. Leaves s_log NULL when no
    log file is configured or it could not be opened. Resets
    s_logBytes to the current file size for size-based rotation. */
static void DbgOpenLog(void)
{
    if (!s_cfg.logfile || s_cfg.logfile[0] == 0) {
        s_log      = NULL;
        s_logBytes = 0;
        return;
    }
    s_log = fopen(s_cfg.logfile, "a+");
    if (!s_log) {
        fprintf(stderr, "FDBG: cannot open '%s', continuing without file log\n",
                s_cfg.logfile);
        s_logBytes = 0;
        return;
    }
    fseek(s_log, 0, SEEK_END);
    s_logBytes = (ULONG)ftell(s_log);
}

/*! @brief Initialise the debug subsystem. Idempotent.

    Creates the process-wide mutex, initialises the per-thread handle
    used for the context stack (DosAllocThreadLocalMemory on OS/2,
    TlsAlloc on Win32), copies the configuration, resets all section
    levels to -1 (unconfigured), parses @p levels, and opens the log
    file.

    @param [in] cfg     Configuration, or NULL for defaults.
    @param [in] levels  Level string, or NULL to leave levels unconfigured. */
void F_DbgInit(const F_DbgConfig *cfg, const char *levels)
{
    int i;

    if (s_inited) return;
    s_inited = 1;

    if (DosCreateMutexSem(NULL, &s_mutex, 0, FALSE) != 0)
        s_mutex = NULL;

#if defined(__OS2__) || defined(__OS2V2__)
    if (DosAllocThreadLocalMemory(1, &s_ctxTls) != 0)
        s_ctxTls = NULL;
#else
    s_ctxTls = TlsAlloc();
    /* s_ctxTls == TLS_OUT_OF_INDEXES signals failure; CtxTlsGet /
       CtxTlsSet check for it and act as no-ops. */
#endif

    if (cfg) s_cfg = *cfg;
    else {
        s_cfg.logfile        = NULL;
        s_cfg.rotateNumber   = 0;
        s_cfg.rotateSize     = 0;
        s_cfg.buffered       = 0;
        s_cfg.ansiColor      = 0;
        s_cfg.beepOnFatal    = 0;
        s_cfg.useSyslog      = 0;
        s_cfg.thresholdLevel = 5;
    }

    for (i = 0; i < F_DBG_MAX_SECTIONS; i++)
        F_DbgLevels[i] = (APIRET)-1;

    if (levels) F_DbgSetLevels(levels);

    DbgOpenLog();
}

/*! @brief Re-apply configuration at runtime (e.g. after reading ini).
    Reopens the log file if @c logfile changed. Does not reset levels;
    use F_DbgSetLevels for that. Thread-safe.
    @param [in] cfg  New configuration. NULL is ignored. */
void APIENTRY F_DbgSetConfig(const F_DbgConfig *cfg)
{
    if (!cfg) return;

    LockDbg();
    s_cfg = *cfg;
    if (s_log) { fclose(s_log); s_log = NULL; s_logBytes = 0; }
    DbgOpenLog();
    UnlockDbg();
}

/*! @brief Flush any buffered log output. Thread-safe. */
void APIENTRY F_DbgFlush(void)
{
    LockDbg();
    if (s_log) fflush(s_log);
    fflush(stderr);
    UnlockDbg();
}

/* ======================================================================== */
/*  Context                                                                 */
/* ======================================================================== */

/*! @brief Enter a debug context with formatted description and
    explicit section/level. Pushes the description onto this thread's
    context stack and emits an "ctx: enter level N" line to all live
    channels. If the level does not pass the section filter or the
    process-wide threshold, no context is pushed.

    Thread-safe.

    @param [in] section  Debug section index (0..F_DBG_MAX_SECTIONS-1).
    @param [in] level    Message level (0..10).
    @param [in] fmt      printf-style description format.
    @param [in] ...      Format arguments.
    @return The new context level on success.
    @retval 0..F_DBG_CTX_MAX-1  New context level.
    @retval -1                  Filtered out, TLS unavailable, or the
                                stack is full. */
int APIENTRY F_DbgCtxEnterf(int section, int level, const char *fmt, ...)
{
    va_list        args;
    char           descr[F_DBG_CTX_MAX_LEN];
    char           line[F_DBG_BUFSIZE * 2];
    DBG_CTX_STACK *s;
    int            newDepth;

    if (section < 0 || section >= F_DBG_MAX_SECTIONS) return -1;
    if ((int)F_DbgLevels[section] < 0) return -1;
    if (level > (int)F_DbgLevels[section]) return -1;
    if (level > s_cfg.thresholdLevel) return -1;

    s = CtxGetOrCreate();
    if (!s) return -1;
    if (s->depth >= F_DBG_CTX_MAX - 1) return -1;

    va_start(args, fmt);
    vsnprintf(descr, sizeof(descr), fmt, args);
    va_end(args);

    newDepth = s->depth + 1;
    s->depth = newDepth;
    strncpy(s->descr[newDepth], descr, F_DBG_CTX_MAX_LEN - 1);
    s->descr[newDepth][F_DBG_CTX_MAX_LEN - 1] = 0;

    LockDbg();
//    snprintf(line, sizeof(line), "%d %s|%sctx: enter level %d: '%s'\n",
//             s_counter, DbgTimeNow(), DbgIndent(newDepth),
//             newDepth, descr);
    snprintf(line, sizeof(line), "%d %s|%s%s\n",
             s_counter, DbgTimeNow(), DbgIndent(newDepth),
             descr);
    s_counter++;
    EmitLineLocked(line);
    if (s_wireSink) s_wireSink(F_DBG_KIND_CTX_OPEN, section, level, descr);
    UnlockDbg();

    return newDepth;
}

/*! @brief Leave a debug context, popping all levels above @p ctx.
    Emits an "ctx: exit level N" line to all live channels; if @p fmt
    is non-NULL and produces a non-empty string, it is appended after
    the description ("... -> result"). Thread-safe.

    @param [in] ctx  Level returned by the matching F_DbgCtxEnterf, or
                     a lower value to unwind several nested contexts
                     at once. -1 is a no-op.
    @param [in] fmt  printf-style result format, or NULL.
    @param [in] ...  Format arguments. */
void APIENTRY F_DbgCtxExitf(int ctx, const char *fmt, ...)
{
    va_list        args;
    char           result[F_DBG_CTX_MAX_LEN];
    char           line[F_DBG_BUFSIZE * 2];
    DBG_CTX_STACK *s;

    if (ctx < 0) return;

    s = CtxGet();
    if (!s || s->depth < ctx) return;

    result[0] = 0;
    if (fmt) {
        va_start(args, fmt);
        vsnprintf(result, sizeof(result), fmt, args);
        va_end(args);
    }

    LockDbg();
    if (result[0]) {
//        snprintf(line, sizeof(line),
//                 "%d %s|%sctx: exit level %d: '%s -> %s'\n",
//                 s_counter, DbgTimeNow(), DbgIndent(ctx),
//                 ctx, s->descr[ctx], result);
        snprintf(line, sizeof(line),
                 "%d %s|%s%s -> %s\n",
                 s_counter, DbgTimeNow(), DbgIndent(ctx),
                 s->descr[ctx], result);
    } else {
//        snprintf(line, sizeof(line),
//                 "%d %s|%sctx: exit level %d: '%s'\n",
//                 s_counter, DbgTimeNow(), DbgIndent(ctx),
//                 ctx, s->descr[ctx]);
        snprintf(line, sizeof(line),
                 "%d %s|%s%s\n",
                 s_counter, DbgTimeNow(), DbgIndent(ctx),
                 s->descr[ctx]);
    }
    s_counter++;
    EmitLineLocked(line);
    if (s_wireSink) s_wireSink(F_DBG_KIND_CTX_CLOSE, 0, 0, result);
    UnlockDbg();

    s->depth = ctx - 1;
}

/*! @brief Enter a debug context without explicit section/level.
    Uses F_DBG_SEC_DEBUG and F_DBG_LVL_VERBOSE. See F_DbgCtxEnterf.
    @param [in] descr  Context description.
    @return The new context level, or -1 if filtered.
    Thread-safe. */
int F_DbgCtxEnter(const char *descr)
{
    return F_DbgCtxEnterf(F_DBG_SEC_DEBUG, F_DBG_LVL_VERBOSE,
                          "%s", descr ? descr : "");
}

/*! @brief Leave a debug context without a result line. See
    F_DbgCtxExitf.
    @param [in] ctx  Level returned by the matching F_DbgCtxEnter, or a
                     lower value to unwind several nested contexts at once. */
void F_DbgCtxExit(int ctx)
{
    F_DbgCtxExitf(ctx, NULL);
}

/* ======================================================================== */
/*  Main emission                                                           */
/* ======================================================================== */

/*! @brief Install or remove the wire-forward sink. Pass NULL to
    uninstall. The sink is invoked for every accepted F_DbgPrint,
    F_DbgFatal, F_DbgCtxEnterf and F_DbgCtxExitf call.
    @param [in] sink  Sink callback, or NULL. */
void APIENTRY F_DbgSetWireSink(F_DbgWireSink sink)
{
    s_wireSink = sink;
}

/*! @brief Emit a diagnostic message if its level passes the section
    filter and the process-wide threshold. Thread-safe.

    Messages whose level is greater than s_cfg.thresholdLevel are
    dropped entirely (no file, stderr, syslog or wire). The
    F_DbgLevels[] section filter is applied first.

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

    if (section < 0 || section >= F_DBG_MAX_SECTIONS) return;
    if ((int)F_DbgLevels[section] < 0) return;
    if (level > (int)F_DbgLevels[section]) return;
    if (level > s_cfg.thresholdLevel) return;

    va_start(args, fmt);
    vsnprintf(argsbuf, sizeof(argsbuf), fmt, args);
    va_end(args);

    LockDbg();

    {
        DBG_CTX_STACK *cs    = CtxGet();
        int            depth = (cs && cs->depth >= 0) ? (cs->depth + 1) : 0;

        snprintf(fullbuf, sizeof(fullbuf), "%d %s|%s%s",
                 s_counter, DbgTimeNow(), DbgIndent(depth), argsbuf);
    }
    s_counter++;

    {
        size_t len = strlen(fullbuf);
        if (len > 0 && len < sizeof(fullbuf) - 1 && fullbuf[len - 1] != '\n') {
            fullbuf[len]     = '\n';
            fullbuf[len + 1] = 0;
        }
    }

    EmitLineLocked(fullbuf);
    if (s_wireSink) s_wireSink(F_DBG_KIND_PRINT, section, level, argsbuf);

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

    If this thread has an active context, the whole stack is emitted
    first as "ctx: enter level N" lines, then the FATAL line.

    @param [in] msg  Fatal error message. Must not be NULL. */
void APIENTRY F_DbgFatal(const char *msg)
{
    char           line[F_DBG_BUFSIZE * 2];
    DBG_CTX_STACK *s;
    int            i;

    if (F_DbgFatalInProgress) return;
    F_DbgFatalInProgress = 1;

    LockDbg();

    s = CtxGet();
    if (s && s->depth >= 0) {
        for (i = 0; i <= s->depth; i++) {
//            snprintf(line, sizeof(line),
//                     "%d %s|%sctx: enter level %d: '%s'\n",
//                     s_counter, DbgTimeNow(), DbgIndent(i),
//                     i, s->descr[i]);
            snprintf(line, sizeof(line),
                     "%d %s|%s%s\n",
                     s_counter, DbgTimeNow(), DbgIndent(i),
                     s->descr[i]);
            s_counter++;
            EmitLineLocked(line);
        }
    }

    {
        int depth = (s && s->depth >= 0) ? (s->depth + 1) : 0;
        snprintf(line, sizeof(line), "%d %s|%sFATAL: %s\n",
                 s_counter, DbgTimeNow(), DbgIndent(depth), msg);
    }
    s_counter++;
    EmitLineLocked(line);

    if (s_wireSink)
        s_wireSink(F_DBG_KIND_FATAL, F_DBG_SEC_DEBUG, F_DBG_LVL_FATAL, msg);

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

/* ======================================================================== */
/*  Platform entry points                                                   */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)

/*! @brief OS/2 DLL entry point. On load (flag == 0), initialise the
    debug subsystem with the default configuration so that every
    component loaded afterwards (PMCLIENT, PMTCP, PMSERVER, PMGPI)
    finds a fully working F_DbgPrint.

    @param [in] hModule  DLL module handle (unused).
    @param [in] flag     0 = load, non-zero = unload.
    @return 1 on success.
    @retval 1  Always. */
unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long flag)
{
    (void)hModule;
    if (flag == 0) {
        F_DbgInit(&F_DBG_DEFAULT_CFG, F_DBG_DEFAULT_LEVELS);
    }
    return 1;
}

#else /* Win32 */

/*! @brief Win32 DLL entry point. On process attach, initialise the
    debug subsystem with the default configuration so that every
    component loaded afterwards (PMCLIENT, PMTCP, PMSERVER, PMGPI)
    finds a fully working F_DbgPrint.

    @param [in] hinstDLL    DLL instance handle (unused).
    @param [in] fdwReason   Reason for calling.
    @param [in] lpReserved  Reserved (unused).
    @return TRUE on success.
    @retval TRUE  Always. */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    (void)hinstDLL; (void)lpReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        F_DbgInit(&F_DBG_DEFAULT_CFG, F_DBG_DEFAULT_LEVELS);
    }
    return TRUE;
}

#endif
