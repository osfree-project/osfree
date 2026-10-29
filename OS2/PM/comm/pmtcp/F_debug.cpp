/*! @file F_debug.cpp
    @brief PMTCP-local debug infrastructure.

    Sends diagnostics over the TCP connection when it is open.
    When the connection is not established, messages are silently
    discarded.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "F_tcp.hpp"

/*! @def BUFSIZE
    @brief Size of the diagnostic message buffer. */
#define BUFSIZE 256

/*! @brief Backend connection object, defined in Ft_main.cpp. */
extern class TCPipe *pF_pipe;

/*! @brief printf-style fatal error entry point.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Arguments for the format string. */
void fatalf(const char *fmt,...);

/*! @brief Error string for the given error class.
    @param [in] type  Error class code.
    @return Human-readable error string. */
const char * xstrerror_type(int type);

/*! @brief Current debug verbosity level. */
extern "C" APIRET _FreePM_db_level;

/*! @brief Per-section debug levels. */
extern "C" APIRET _FreePM_debugLevels[64];

/*! @brief Fatal error state flag. */
extern int _FreePM_FatalInProgress;

/*! @brief Send a command with a 32-bit argument (defined in Ft_main.cpp).
    @param [in] recvobj  Connection object.
    @param [in] ncmd     Command code.
    @param [in] data     32-bit argument.
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data);

/*! @brief Send a raw data buffer (defined in Ft_main.cpp).
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, non-zero on transport error. */
extern "C" APIRET EXPENTRY _F_SendDataToServer(void *recvobj, void *data, int len);

/*! @brief Emit a diagnostic message.

    Formats the arguments into a fixed-size buffer and, if the TCP
    connection is open, sends the result to the server via
    F_CMD_DB_PRINT. Otherwise the message is discarded.

    @param [in] format  printf-style format string.
    @param [in] ...     Arguments for the format string. */
extern "C" void APIENTRY
_db_print(const char *format,...)
{
  char f[BUFSIZE];
  va_list args;

  memset(&f, 0, BUFSIZE);
  va_start(args, format);
  vsprintf(f, format, args);

  if (pF_pipe)
  {
    if (!_F_SendCmdToServer(pF_pipe, 0x200 /* F_CMD_DB_PRINT */, 0))
      _F_SendDataToServer(pF_pipe, (void *)f, BUFSIZE);
  }

  va_end(args);
}

/*! @brief Debug initialisation stub.
    @param [in] logfile  Log file name (unused).
    @param [in] options  Debug options (unused). */
extern "C" void
_db_init(const char *logfile, const char *options)
{
    (void)logfile;
    (void)options;
}

/*! @brief Log rotation stub. */
void
_db_rotate_log(void) {}

/*! @brief Assertion failure handler.
    @param [in] msg   Assertion text.
    @param [in] file  Source file name.
    @param [in] line  Line number. */
void
xassert(const char *msg, const char *file, int line)
{
    _db_print("assertion failed: %s:%d: \"%s\"\n", file, line, msg);
    abort();
}

/*! @brief printf-style fatal helper with va_list.
    @param [in] fmt   printf-style format string.
    @param [in] args  Argument list. */
void fatalvf(const char *fmt, va_list args);

/*! @brief Report a fatal error.
    @param [in] message  Fatal error message. */
extern "C" void _fatal_common(const char *message);

/*! @brief printf-style fatal error entry point.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Arguments for the format string. */
void fatalf(const char *fmt,...);

/*! @brief Report a fatal error to the server.

    Guards against re-entry via _FreePM_FatalInProgress. If the TCP
    connection is open, sends the message via F_CMD_FATAL_COMMON.

    @param [in] message  Fatal error message. */
extern "C" void
_fatal_common(const char *message)
{
  if(_FreePM_FatalInProgress) return;
  _FreePM_FatalInProgress = 1;

  if (pF_pipe)
  {
    if (!_F_SendCmdToServer(pF_pipe, 0x201 /* F_CMD_FATAL_COMMON */, 0))
      _F_SendDataToServer(pF_pipe, (void *)message, strlen(message));
  }
}

/*! @brief Fatal error: report and abort.
    @param [in] message  Fatal error message. */
void APIENTRY _fatal(const char *message)
{
    _fatal_common(message);
    abort();
}

/*! @brief printf-style fatal error entry point.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Arguments for the format string. */
void
fatalf(const char *fmt,...)
{
    va_list args;
    va_start(args, fmt);

    fatalvf(fmt, args);
    va_end(args);
}

/*! @brief printf-style fatal helper with va_list.
    @param [in] fmt   printf-style format string.
    @param [in] args  Argument list. */
void
fatalvf(const char *fmt, va_list args)
{
    static char fatal_str[BUFSIZE];
    vsnprintf(fatal_str, sizeof(fatal_str), fmt, args);
    _fatal(fatal_str);
}
