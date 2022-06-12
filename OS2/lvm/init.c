#define  INCL_BASE
#include <os2.h>

APIRET APIENTRY __DLLstart_ (HMODULE hmod, ULONG flag);

APIRET APIENTRY dll_initterm (HMODULE hmod, ULONG flag)
{
  APIRET rc;

  // call C startup first
  rc = __DLLstart_(hmod, flag);

  if (!rc)
    return 0;

  return 1;
}
