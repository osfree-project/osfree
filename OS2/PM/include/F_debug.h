/*! @file F_debug.hpp
    @brief Debug and fatal-error infrastructure, shared by all
        components (client, server, transports).

    Lives in fpmdebug.dll, linked by pmclient.dll, pmserver.exe,
    fpmtcp.dll, fpmpipe.dll. One F_DbgInit() per process, called by
    the top-level component; the rest just call F_DbgPrint / F_DbgFatal.

    The wire sink is optional: on the client side, F_DbgPrint and
    F_DbgFatal also forward the message to the server via the
    transport. On the server side, no wire sink is set, so messages
    stay local.
*/
#ifndef F_DEBUG_H
#define F_DEBUG_H

#include "F_config.h"
#include "F_platform.h"

/*! @brief Size of the formatted-message buffer. */
#define F_DBG_BUFSIZE       512

/*! @brief Kind of a wire-forwarded debug message. */
typedef enum {
    F_DBG_KIND_PRINT,   /*!< Regular diagnostic line. */
    F_DBG_KIND_FATAL,   /*!< Fatal error message.     */
    F_DBG_KIND_CTX_OPEN,/*!< Internal: context enter. */
    F_DBG_KIND_CTX_CLOSE/*!< Internal: context exit.  */
} F_DbgKind;

/*! @brief Calling convention of the wire-forward callback.

    On OS/2 the DLL exports use APIENTRY (_System), and a callback
    invoked from a DLL into an executable built with the default
    _Optlink must use the same convention; otherwise arguments are
    passed differently and arrive garbled.

    On Win32, APIENTRY expands to __declspec(__pragma("__estdcall")),
    which is valid only on function declarations/definitions and is
    rejected in a pointer-to-function typedef. __stdcall is used there
    instead: it is ABI-compatible with the surrounding F_DBG exports
    and is well supported by Watcom C/C++. */
#if defined(__OS2__) || defined(__OS2V2__)
#define F_DBG_WIRE_CALLBACK APIENTRY
#else
#define F_DBG_WIRE_CALLBACK __stdcall
#endif

/*! @brief Wire-forward callback. Invoked from F_DbgPrint / F_DbgFatal /
    F_DbgCtxEnterf / F_DbgCtxExitf when set. Must not re-enter the
    debug subsystem.

    @param [in] kind     Message kind.
    @param [in] section  Debug section (F_DBG_SEC_*).
    @param [in] level    Message level (0..10).
    @param [in] msg      Message text (no counter, no timestamp, no \n). */
typedef void (F_DBG_WIRE_CALLBACK *F_DbgWireSink)(
    F_DbgKind kind, int section, int level, const char *msg);

struct F_DbgConfig {
    const char *logfile;      /*!< Log file path, or NULL for stderr-only.   */
    int         rotateNumber; /*!< Rotation depth; 0 disables rotation.      */
    int         rotateSize;   /*!< Rotate when file reaches N bytes; 0 disables. */
    int         buffered;     /*!< 1: do not fflush after each message.      */
    int         ansiColor;    /*!< 1: emit ANSI color escapes.               */
    int         beepOnFatal;  /*!< 1: beep before abort on fatal.            */
    int         useSyslog;    /*!< 1: mirror to syslog (Unix only, no-op elsewhere). */
    int         thresholdLevel;/*!< Max level routed to any channel (0..10). */
};

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Initialise the debug subsystem. Idempotent: subsequent
    calls return without effect.
    @param [in] cfg     Configuration, or NULL for defaults.
    @param [in] levels  Level string, e.g. "ALL,7" or "1,5 2,3". May be NULL. */
void APIENTRY F_DbgInit(const F_DbgConfig *cfg, const char *levels);

/*! @brief Re-parse the level string at runtime. */
void APIENTRY F_DbgSetLevels(const char *levels);

/*! @brief Re-apply configuration at runtime (e.g. after reading ini).
    Reopens the log file if @c logfile changed. Does not reset levels;
    use F_DbgSetLevels for that. Thread-safe. */
void APIENTRY F_DbgSetConfig(const F_DbgConfig *cfg);

/*! @brief Flush any buffered log output. Thread-safe. */
void APIENTRY F_DbgFlush(void);

/*! @brief Rotate the log file (if rotation is enabled). */
void APIENTRY F_DbgRotate(void);

/*! @brief Install (or remove, with NULL) the wire-forward sink. */
void APIENTRY F_DbgSetWireSink(F_DbgWireSink sink);

/*! @brief Emit a diagnostic message if its level passes the section
    filter. Thread-safe.
    @param [in] section  Debug section index.
    @param [in] level    Message level, 0 (critical) .. 10 (verbose).
    @param [in] fmt      printf-style format string.
    @param [in] ...      Format arguments. */
void APIENTRY F_DbgPrint(int section, int level, const char *fmt, ...);

/*! @brief Alias for F_DbgPrint. The name documents intent
    (transport-level tracing). Same signature. */
#define F_DbgTrace F_DbgPrint

/*! @brief Log and abort with a raw string. Guarded. */
void APIENTRY F_DbgFatal(const char *msg);

/*! @brief Log and abort with a printf-style string. */
void APIENTRY F_DbgFatalf(const char *fmt, ...);

/*! @brief Assertion-failure handler. Calls F_DbgFatal unless the
    process is shutting down. */
void APIENTRY F_DbgAssert(const char *msg, const char *file, int line);

/*! @brief Enter a debug context. Returns the new level. Thread-safe
    (writes guarded by mutex; the stack itself is process-global). */
int  APIENTRY F_DbgCtxEnter(const char *descr);

/*! @brief Leave a debug context (pop to the given level). */
void APIENTRY F_DbgCtxExit(int ctx);

/*! @brief Enter a debug context with formatted description and
    explicit section/level. If level does not pass the section filter,
    no context is pushed and -1 is returned.
    @return Context level (0..F_DBG_CTX_MAX-1), or -1 if filtered. */
int  APIENTRY F_DbgCtxEnterf(int section, int level,
                             const char *fmt, ...);

/*! @brief Leave a debug context with formatted result.
    @param [in] ctx  Level returned by F_DbgCtxEnterf, or -1 (no-op). */
void APIENTRY F_DbgCtxExitf(int ctx, const char *fmt, ...);

/*! @brief Set the process-wide shutdown flag. Suppresses abort()
    inside F_DbgAssert while the flag is non-zero.
    @param [in] flag  Non-zero to suppress asserts during shutdown. */
void APIENTRY F_DbgSetShuttingDown(int flag);

#ifdef __cplusplus
}
#endif

#endif /* F_DEBUG_H */
