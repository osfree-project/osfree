/*  DL.DLL functions (loading the l4env shared
 *  libs from OS/2 or another OS personality
 *  programs)
 */

#include <l4/log/l4log.h>

#include <gcc_os2def.h>
#include <apistub.h>
#include <dl.h>

#include <stdarg.h>
#include <dlfcn.h>

extern ULONG kalHandle;

typedef APIRET APIENTRY (*funcaddr_t)(va_list a);

APIRET CDECL DlOpen(PSZ name, PULONG handle)
{
  if ((*handle = (ULONG)dlopen(name, 2)))
    return NO_ERROR;
    
  return ERROR_FILE_NOT_FOUND;
}

APIRET CDECL DlSym(ULONG handle, PSZ sym, PPVOID addr)
{
  if ((*addr = dlsym((void *)handle, sym)))
    return NO_ERROR;
    
  return ERROR_INVALID_NAME;
  
}

APIRET CDECL DlRoute(ULONG handle, PSZ name, ...)
{
  funcaddr_t addr;
  va_list args;
  APIRET rc = 0;
  //int i;

  if (!handle)
    handle = kalHandle;

  va_start(args, name);

  if ((addr = (funcaddr_t)dlsym((void *)handle, name)))
    rc = addr(args);

  va_end(args);    
  return rc;
}
