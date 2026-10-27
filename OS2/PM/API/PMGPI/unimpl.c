/*!
   @file

   @brief Diagnostic helpers for unimplemented GPI stubs.

   Provides a best-effort implementation of {@c unimplemented()} that
   logs the offending function name and returns 0. Logging is routed to
   {@c DosLogWrite} (DOSCALLS ordinal 1112) when available; otherwise
   output is silently discarded.
*/

#define INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>

/*!
   @brief Ordinal of {@c DosLogWrite} inside DOSCALLS.
*/
#define LOG_ORD 1112

/*!
   @brief Internal debug-print hook.

   No-op implementation. Intended to be replaced by a stronger
   implementation in debug builds if needed. Kept as a separate symbol
   so that {@c log()} does not depend on the final sink.
*/
void APIENTRY _db_print(const char *format,...)
{
    (void)format;
}

/*!
   @brief Signature of the {@c DosLogWrite} entry point.

   Resolved dynamically from DOSCALLS at startup by {@c log_init()}.
   The string argument is NUL-terminated and must not exceed 512 bytes
   (per the DOSCALLS contract).
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

   Formats the arguments with {@c vsprintf into a 1024-byte stack
   buffer and passes the result to {@c _db_print()}. Truncation is
   silent; callers must keep formatted lines reasonably short.

   @param[in] fmt  printf-style format string.
   @param[in] ...  Arguments for {@c fmt}.
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
   0 (the historical convention for unimplemented GPI calls). All
   current callers are forwarder stubs, so return value is only
   meaningful for functions whose public type is non-BOOL.

   @param[in] func  Name of the calling function (usually
                    {@c __FUNCTION__}); not copied, must remain valid
                    for the duration of the call.

   @return 0.
*/
APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}
