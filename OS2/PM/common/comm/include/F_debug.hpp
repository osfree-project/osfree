/*! @file F_debug.hpp
    @brief Debug and fatal-error infrastructure, shared by all layers
        (client, server, transports).

    The wire sink is optional: when set (client side), _db_print and
    _fatal_common forward messages to the server via F_CMD_DB_PRINT /
    F_CMD_FATAL_COMMON. When NULL (server side), only local logging.
*/
#ifndef F_DEBUG_HPP
#define F_DEBUG_HPP

#include "F_platform.h"

/*! @brief Kinds of wire-routed debug messages. */
typedef enum {
    F_DBG_PRINT,
    F_DBG_FATAL
} F_DBG_KIND;

/*! @brief Wire sink callback. Installed by the wire layer on connect. */
typedef void (*F_DbgWireSink)(F_DBG_KIND kind, const char *msg);

void F_DbgSetWireSink(F_DbgWireSink sink);

/*! @brief Local log (stdout + OutputDebugString on Win32). */
void F_Dbg(const char *fmt, ...);

/*! @brief Local log with an error code. */
void F_DbgError(const char *func, int err);

#ifdef __cplusplus
extern "C" {
#endif

void APIENTRY _db_print(const char *format, ...);
void _fatal_common(const char *message);
void APIENTRY _fatal(const char *message);
void fatalf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

void xassert(const char *msg, const char *file, int line);
void _db_init(const char *logfile, const char *options);
void _db_rotate_log(void);

#endif /* F_DEBUG_HPP */
