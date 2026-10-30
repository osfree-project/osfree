/*!
   @file

   @brief Diagnostic helpers for unimplemented GPI stubs.

   Provides a best-effort implementation of {@c unimplemented()} that
   logs the offending function name and returns GPI_ERROR (-1) so that
   callers can distinguish "not implemented" from a successful (0)
   result. Logging is routed to {@c DosLogWrite} (DOSCALLS ordinal
   1112) when available; otherwise output is written to stderr.

   @note Returning GPI_ERROR is the correct convention for LONG- and
         HBITMAP-returning stubs. BOOL-returning stubs will see the
         value as TRUE (non-zero); a full fix requires separate
         unimplemented_bool() / unimplemented_long() helpers. This is
         a known limitation of the single-function design.
*/

#include <stdio.h>
#include <stdarg.h>

#define INCL_DOSMODULEMGR
#include <gpi_platform.h>


/*!
   @def LOG_ORD
   @brief Ordinal of {@c DosLogWrite} inside DOSCALLS.
*/
#define LOG_ORD 1112

/*!
   @def GPI_ERROR
   @brief Fallback definition of the OS/2 GPI error code.

   Normally provided by <pmgpi.h> (via INCL_GPI / INCL_PM). This module
   is compiled separately with only INCL_DOSMODULEMGR, so provide a
   local fallback to avoid depending on the toolkit include order.
*/
#ifndef GPI_ERROR
#define GPI_ERROR (-1)
#endif

/*!
   @brief Internal debug-print hook.

   Writes the formatted string directly to stderr. Kept as a separate
   symbol so that {@c log()} does not depend on the final sink.

   @param [in] format  printf-style format string.
   @param [in] ...     Additional arguments for the format string.
*/
void APIENTRY _db_print(const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vfprintf(stderr, format, arg_ptr);
    va_end(arg_ptr);
    fflush(stderr);
}

/*!
   @brief Signature of the {@c DosLogWrite} entry point.

   Resolved dynamically from DOSCALLS at startup by {@c log_init()}.

   @param [in] s   NUL-terminated string to log (max 512 bytes).
   @return         APIRET return code from the underlying kernel call.
*/
typedef APIRET APIENTRY (*logwrt_t)(PSZ s);

/*!
   @brief Resolved pointer to {@c DosLogWrite}, or 0 if unavailable.

   Non-zero when {@c log_init()} successfully located ordinal 1112 in
   DOSCALLS. {@c log()} checks this before calling through.
*/
logwrt_t DosLogWrite = 0;

/*!
   @brief Resolve {@c DosLogWrite} from the DOSCALLS module.

   Called once at library load. Silently succeeds with {@c DosLogWrite
   left at 0 when DOSCALLS is absent or the ordinal does not exist
   (e.g. on some non-IBM kernels), in which case diagnostic output is
   discarded.
*/
void log_init(void)
{
  long __syscall (*pfn)(void);
  APIRET  rc;
  HMODULE handle;

  /* now check if DosLogWrite is available */
  rc = DosQueryModuleHandle("DOSCALLS", &handle);

  if (rc) return;

  rc = DosQueryProcAddr(handle, LOG_ORD, 0, (PFN *)&pfn);

  if (rc != 182) // ERROR_INVALID_ORDINAL
    DosLogWrite = (logwrt_t)pfn;
}

/*!
   @brief Format and emit a diagnostic message.

   Formats the arguments with {@c vsprintf} into a 1024-byte stack
   buffer and passes the result to {@c _db_print()}. Truncation is
   silent; callers must keep formatted lines reasonably short.

   @param [in] fmt  printf-style format string.
   @param [in] ...  Arguments for {@c fmt}.
*/
void log(const char *fmt, ...)
{
  va_list arg_ptr;
  char buf[1024];

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);

  _db_print(buf);
}


/*!
   @brief Report that a GPI entry point has no implementation yet.

   Logs a human-readable message identifying the function, then returns
   GPI_ERROR (-1), the OS/2 convention for a failing GPI call. This
   lets callers detect the missing implementation instead of treating
   the return value 0 as a valid result.

   @param [in] func  Name of the calling function (usually
                     {@c __FUNCTION__}); not copied, must remain valid
                     for the duration of the call.

   @return GPI_ERROR (-1) always.

   @note For BOOL-returning stubs this yields TRUE (non-zero). A full
         fix would require separate helpers per return type.
*/
APIRET unimplemented(char *func)
{
  /* Explicit debug output added for stderr visibility */
  fprintf(stderr, "[unimpl] unimplemented(%s)\n", func);
  fflush(stderr);

  log("%s is not yet implemented!\n", func);
  return (APIRET)GPI_ERROR;
}
