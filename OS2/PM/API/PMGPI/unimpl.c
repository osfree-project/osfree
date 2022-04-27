

#define INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>

// DosLogWrite ordinal
#define LOG_ORD 1112

void APIENTRY _db_print(const char *format,...);
void APIENTRY _db_print(const char *format,...)
{
}

typedef APIRET APIENTRY (*logwrt_t)(PSZ s);

logwrt_t DosLogWrite = 0;

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

void log(const char *fmt, ...)
{
  va_list arg_ptr;
  char buf[1024];

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);

  _db_print(buf);
}


APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}


