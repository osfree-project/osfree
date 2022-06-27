/*  PMWP.DLL init/term: 
 */

// libc includes
#include <string.h>
#include <stdio.h>
// OS/2 includes
#define  INCL_DOSMISC
#define  INCL_DOSMODULEMGR
#include <os2.h>

extern "C" APIRET APIENTRY __DLLstart_ (HMODULE hmod, ULONG flag);

extern "C" APIRET APIENTRY dll_initterm (HMODULE hmod, ULONG flag)
{
  APIRET    rc;

  // call C startup init first
  rc = __DLLstart_(hmod, flag);

  return rc;
}
