/*  DL.DLL functions (loading the l4env shared
 *  libs from OS/2 or another OS personality
 *  programs)
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/types.h>
#include <os3/kal.h>
#include <os3/dl.h>

/* libc includes */
#include <stdarg.h>
#include <dlfcn.h>

ULONG KalHandle;

typedef APIRET APIENTRY (*funcaddr_t)(va_list a);

APIRET CDECL KalDlOpen(PSZ name, PULONG handle)
{
  if ((*handle = (ULONG)dlopen(name, 2)))
    return NO_ERROR;

  return ERROR_FILE_NOT_FOUND;
}

APIRET CDECL KalDlSym(ULONG handle, PSZ sym, PPVOID addr)
{
  if ((*addr = dlsym((void *)handle, sym)))
    return NO_ERROR;

  return ERROR_INVALID_NAME;

}

APIRET CDECL KalDlRoute(ULONG handle, PSZ name, ...)
{
  funcaddr_t addr;
  va_list args;
  APIRET rc = 0;

  if (!handle)
    handle = KalHandle;

  va_start(args, name);

  if ((addr = (funcaddr_t)dlsym((void *)handle, name)))
    rc = addr(args);

  va_end(args);
  return rc;
}

void test(void)
{
}
