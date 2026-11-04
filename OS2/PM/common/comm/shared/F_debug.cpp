/*! @file F_debug.cpp
    @brief Debug and fatal-error infrastructure, shared by all layers.

    No transport dependency: fatals and prints are routed through an
    optional F_DbgWireSink installed by the wire layer. When no sink
    is set, messages go to local log only.
*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "F_platform.h"
#include "F_debug.hpp"

#define F_DBG_BUFSIZE 512

/* Fatal re-entry guard (defined in F_globals.cpp). */
extern int _FreePM_FatalInProgress;

static F_DbgWireSink g_wireSink = NULL;

void F_DbgSetWireSink(F_DbgWireSink sink)
{
    g_wireSink = sink;
}

void F_Dbg(const char *fmt, ...)
{
    char buf[F_DBG_BUFSIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

#if defined(__WIN32__) || defined(_WIN32)
    OutputDebugStringA(buf);
#endif
    printf("%s", buf);
    fflush(stdout);
}

void F_DbgError(const char *func, int err)
{
    F_Dbg("%s failed: 0x%x\n", func, err);
}

extern "C" void APIENTRY _db_print(const char *format, ...)
{
    char buf[256];
    va_list args;

    memset(buf, 0, sizeof(buf));
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    if (g_wireSink) g_wireSink(F_DBG_PRINT, buf);
}

extern "C" void _fatal_common(const char *message)
{
    if (_FreePM_FatalInProgress) return;
    _FreePM_FatalInProgress = 1;

    if (g_wireSink) g_wireSink(F_DBG_FATAL, message);
}

extern "C" void APIENTRY _fatal(const char *message)
{
    _fatal_common(message);
    abort();
}

void fatalf(const char *fmt, ...)
{
    char buf[F_DBG_BUFSIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    _fatal(buf);
}

void xassert(const char *msg, const char *file, int line)
{
    _db_print("assertion failed: %s:%d: \"%s\"\n", file, line, msg);
    abort();
}

void _db_init(const char *logfile, const char *options)
{
    (void)logfile;
    (void)options;
}

void _db_rotate_log(void) {}
